/*
 * Interop test: File transfer operations.
 *
 * Tests the opendnp3 master/outstation file transfer API using a
 * TestFileHandler (in-memory filesystem) on the opendnp3 outstation side.
 * Also tests stepfunc master reading from opendnp3 outstation and
 * error paths when no file handler is present.
 */

#include "InteropFixture.h"

#include <opendnp3/outstation/IFileHandler.h>

#include <ser4cpp/serialization/LittleEndian.h>

#include <catch.hpp>

#include <algorithm>
#include <map>
#include <numeric>

#define SUITE(name) "InteropFileTransfer - " name

// ---------------------------------------------------------------------------
// In-memory file handler for the opendnp3 outstation
// ---------------------------------------------------------------------------

class TestFileHandler : public opendnp3::IFileHandler
{
public:
    struct FileEntry
    {
        std::vector<uint8_t> data;
        opendnp3::FilePermissions permissions;
        uint64_t timeOfCreation = 0;
    };

    struct DirEntry
    {
        std::vector<opendnp3::FileInfo> entries;
    };

    struct OpenHandle
    {
        std::string filename;
        opendnp3::FileMode mode;
        uint16_t maxBlockSize;
        std::vector<uint8_t> writeBuffer; // accumulates written blocks
    };

    // Credentials for authentication testing
    std::string validUsername = "admin";
    std::string validPassword = "secret";

    std::mutex mutex;
    std::map<std::string, FileEntry> files;
    std::map<std::string, DirEntry> directories;
    std::map<uint32_t, OpenHandle> openHandles;
    uint32_t nextHandle = 1;
    bool abortCalled = false;
    uint32_t lastAbortedHandle = 0;
    uint32_t lastReceivedAuthKey = 0;

    void AddFile(const std::string& name,
                 const std::vector<uint8_t>& data,
                 opendnp3::FilePermissions perms = opendnp3::FilePermissions())
    {
        std::lock_guard<std::mutex> lock(mutex);
        files[name] = {data, perms, 0};
    }

    void AddDirectory(const std::string& path, const std::vector<opendnp3::FileInfo>& entries)
    {
        std::lock_guard<std::mutex> lock(mutex);
        directories[path] = {entries};
    }

    /// Serialize directory entries into raw Group70Var7 format (without free-format wrapper)
    static std::vector<uint8_t> SerializeDirectoryEntries(const std::vector<opendnp3::FileInfo>& entries)
    {
        std::vector<uint8_t> result;
        for (const auto& info : entries)
        {
            // Fixed part: 18 bytes (2+2+2+4+6+2), then filename
            const uint16_t fixedLen = 18;
            const uint16_t fileNameLen = static_cast<uint16_t>(info.fileName.size());

            uint8_t buf[18];
            auto wseq = ser4cpp::wseq_t(buf, 18);
            ser4cpp::UInt16::write_to(wseq, fixedLen);                         // file_name_offset
            ser4cpp::UInt16::write_to(wseq, fileNameLen);                      // file_name_size
            ser4cpp::UInt16::write_to(wseq, static_cast<uint16_t>(info.type)); // file_type
            ser4cpp::UInt32::write_to(wseq, info.size);                        // file_size

            // time_of_creation: 6 bytes LE
            for (int j = 0; j < 6; ++j)
            {
                ser4cpp::UInt8::write_to(wseq, static_cast<uint8_t>((info.timeOfCreation >> (j * 8)) & 0xFF));
            }

            ser4cpp::UInt16::write_to(wseq, info.permissions.ToRaw()); // permissions

            result.insert(result.end(), buf, buf + 18);
            result.insert(result.end(), info.fileName.begin(), info.fileName.end());
        }
        return result;
    }

    std::vector<uint8_t> GetFileData(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = files.find(name);
        if (it != files.end())
            return it->second.data;
        return {};
    }

    bool HasFile(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(mutex);
        return files.count(name) > 0;
    }

    opendnp3::FileCommandResult GetFileInfo(const std::string& filename) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        opendnp3::FileCommandResult result;

        // Check directories first
        auto dit = directories.find(filename);
        if (dit != directories.end())
        {
            result.status = opendnp3::FileStatus::SUCCESS;
            result.info.fileName = filename;
            result.info.type = opendnp3::FileType::DIRECTORY;
            result.info.size = 0;
            result.info.timeOfCreation = 0;
            return result;
        }

        auto it = files.find(filename);
        if (it == files.end())
        {
            result.status = opendnp3::FileStatus::NOT_EXIST;
            return result;
        }
        result.status = opendnp3::FileStatus::SUCCESS;
        result.info.fileName = filename;
        result.info.type = opendnp3::FileType::SIMPLE_FILE;
        result.info.size = static_cast<uint32_t>(it->second.data.size());
        result.info.timeOfCreation = it->second.timeOfCreation;
        result.info.permissions = it->second.permissions;
        return result;
    }

    opendnp3::FileOpenResult OpenFile(const std::string& filename,
                                      uint32_t authKey,
                                      opendnp3::FilePermissions permissions,
                                      opendnp3::FileMode mode,
                                      uint16_t maxBlockSize,
                                      uint16_t /*requestId*/) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        opendnp3::FileOpenResult result;
        lastReceivedAuthKey = authKey;

        if (mode == opendnp3::FileMode::READ)
        {
            // Check directories first
            auto dit = directories.find(filename);
            if (dit != directories.end())
            {
                // Serialize directory entries into raw data
                auto dirData = SerializeDirectoryEntries(dit->second.entries);
                // Store as a temporary file entry for block reading
                std::string tempKey = filename + ".__dir_data__";
                files[tempKey] = {dirData, opendnp3::FilePermissions(), 0};

                result.status = opendnp3::FileStatus::SUCCESS;
                result.fileHandle = nextHandle++;
                result.fileSize = static_cast<uint32_t>(dirData.size());
                result.maxBlockSize = maxBlockSize;
                openHandles[result.fileHandle] = {tempKey, mode, maxBlockSize, {}};
                return result;
            }

            auto it = files.find(filename);
            if (it == files.end())
            {
                result.status = opendnp3::FileStatus::NOT_EXIST;
                return result;
            }
            result.status = opendnp3::FileStatus::SUCCESS;
            result.fileHandle = nextHandle++;
            result.fileSize = static_cast<uint32_t>(it->second.data.size());
            result.maxBlockSize = maxBlockSize;
            openHandles[result.fileHandle] = {filename, mode, maxBlockSize, {}};
        }
        else if (mode == opendnp3::FileMode::WRITE)
        {
            result.status = opendnp3::FileStatus::SUCCESS;
            result.fileHandle = nextHandle++;
            result.fileSize = 0;
            result.maxBlockSize = maxBlockSize;
            // Create or overwrite the file entry (empty for now)
            files[filename] = {std::vector<uint8_t>(), permissions, 0};
            openHandles[result.fileHandle] = {filename, mode, maxBlockSize, {}};
        }
        else if (mode == opendnp3::FileMode::APPEND)
        {
            result.status = opendnp3::FileStatus::SUCCESS;
            result.fileHandle = nextHandle++;
            result.maxBlockSize = maxBlockSize;
            // Create file if it doesn't exist, preserve existing data for appending
            auto it = files.find(filename);
            if (it == files.end())
            {
                files[filename] = {std::vector<uint8_t>(), permissions, 0};
            }
            result.fileSize = static_cast<uint32_t>(files[filename].data.size());
            openHandles[result.fileHandle] = {filename, mode, maxBlockSize, {}};
        }
        else
        {
            result.status = opendnp3::FileStatus::INVALID_MODE;
        }
        return result;
    }

    opendnp3::FileBlockResult ReadBlock(uint32_t fileHandle, uint32_t blockNum) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        opendnp3::FileBlockResult result;

        auto hit = openHandles.find(fileHandle);
        if (hit == openHandles.end())
        {
            result.status = opendnp3::FileStatus::NOT_OPENED;
            return result;
        }

        auto fit = files.find(hit->second.filename);
        if (fit == files.end())
        {
            result.status = opendnp3::FileStatus::NOT_EXIST;
            return result;
        }

        const auto& fileData = fit->second.data;
        size_t blockSize = hit->second.maxBlockSize;
        size_t offset = static_cast<size_t>(blockNum) * blockSize;

        if (offset > fileData.size())
        {
            result.status = opendnp3::FileStatus::BLOCK_SEQ;
            return result;
        }

        size_t remaining = fileData.size() - offset;
        size_t chunkSize = std::min(remaining, blockSize);

        result.status = opendnp3::FileStatus::SUCCESS;
        result.data.assign(fileData.begin() + offset, fileData.begin() + offset + chunkSize);
        result.lastBlock = (offset + chunkSize >= fileData.size());
        return result;
    }

    opendnp3::FileStatus WriteBlock(
        uint32_t fileHandle, uint32_t /*blockNum*/, bool lastBlock, const uint8_t* data, size_t size) override
    {
        std::lock_guard<std::mutex> lock(mutex);

        auto hit = openHandles.find(fileHandle);
        if (hit == openHandles.end())
        {
            return opendnp3::FileStatus::NOT_OPENED;
        }

        hit->second.writeBuffer.insert(hit->second.writeBuffer.end(), data, data + size);

        if (lastBlock)
        {
            // Flush to file
            auto fit = files.find(hit->second.filename);
            if (fit != files.end())
            {
                if (hit->second.mode == opendnp3::FileMode::APPEND)
                {
                    // Append to existing data
                    fit->second.data.insert(fit->second.data.end(), hit->second.writeBuffer.begin(),
                                            hit->second.writeBuffer.end());
                }
                else
                {
                    fit->second.data = hit->second.writeBuffer;
                }
            }
            hit->second.writeBuffer.clear();
        }

        return opendnp3::FileStatus::SUCCESS;
    }

    opendnp3::FileStatus CloseFile(uint32_t fileHandle, uint16_t /*requestId*/) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto hit = openHandles.find(fileHandle);
        if (hit == openHandles.end())
        {
            return opendnp3::FileStatus::NOT_OPENED;
        }

        // If there's unflushed write data, flush it now
        if (!hit->second.writeBuffer.empty())
        {
            auto fit = files.find(hit->second.filename);
            if (fit != files.end())
            {
                if (hit->second.mode == opendnp3::FileMode::APPEND)
                {
                    fit->second.data.insert(fit->second.data.end(), hit->second.writeBuffer.begin(),
                                            hit->second.writeBuffer.end());
                }
                else if (hit->second.mode == opendnp3::FileMode::WRITE)
                {
                    fit->second.data = hit->second.writeBuffer;
                }
            }
        }

        openHandles.erase(hit);
        return opendnp3::FileStatus::SUCCESS;
    }

    opendnp3::FileStatus DeleteFile(const std::string& filename) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = files.find(filename);
        if (it == files.end())
        {
            return opendnp3::FileStatus::NOT_EXIST;
        }
        files.erase(it);
        return opendnp3::FileStatus::SUCCESS;
    }

    void AbortFile(uint32_t fileHandle) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        abortCalled = true;
        lastAbortedHandle = fileHandle;
        openHandles.erase(fileHandle);
    }

    opendnp3::FileAuthResult AuthenticateFile(const std::string& username, const std::string& password) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        opendnp3::FileAuthResult result;
        if (username == validUsername && password == validPassword)
        {
            result.status = opendnp3::FileStatus::SUCCESS;
            result.authKey = 12345;
        }
        else
        {
            result.status = opendnp3::FileStatus::PERMISSION_DENIED;
            result.authKey = 0;
        }
        return result;
    }
};

// ---------------------------------------------------------------------------
// Synchronizing callback for opendnp3 file read operations
// ---------------------------------------------------------------------------
class SyncFileReadCallback
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    opendnp3::FileReadResult result;

    opendnp3::FileReadCallbackT Callback()
    {
        return [this](const opendnp3::FileReadResult& r) {
            std::lock_guard<std::mutex> lock(mutex);
            result = r;
            completed = true;
            cv.notify_all();
        };
    }

    bool WaitForCompletion(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return completed; });
    }
};

// ---------------------------------------------------------------------------
// Synchronizing callback for opendnp3 file info operations
// ---------------------------------------------------------------------------
class SyncFileInfoCallback
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    opendnp3::FileInfoResult result;

    opendnp3::FileInfoCallbackT Callback()
    {
        return [this](const opendnp3::FileInfoResult& r) {
            std::lock_guard<std::mutex> lock(mutex);
            result = r;
            completed = true;
            cv.notify_all();
        };
    }

    bool WaitForCompletion(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return completed; });
    }
};

// ---------------------------------------------------------------------------
// Synchronizing callback for opendnp3 file operation (delete)
// ---------------------------------------------------------------------------
class SyncFileOpCallback
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    opendnp3::FileOperationResult result;

    opendnp3::FileOperationCallbackT Callback()
    {
        return [this](const opendnp3::FileOperationResult& r) {
            std::lock_guard<std::mutex> lock(mutex);
            result = r;
            completed = true;
            cv.notify_all();
        };
    }

    bool WaitForCompletion(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return completed; });
    }
};

// ---------------------------------------------------------------------------
// Synchronizing callback for opendnp3 file write operations
// ---------------------------------------------------------------------------
class SyncFileWriteCallback
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    opendnp3::FileWriteResult result;

    opendnp3::FileWriteCallbackT Callback()
    {
        return [this](const opendnp3::FileWriteResult& r) {
            std::lock_guard<std::mutex> lock(mutex);
            result = r;
            completed = true;
            cv.notify_all();
        };
    }

    bool WaitForCompletion(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return completed; });
    }
};

// Shared state for stepfunc file read results (outlives the FileReader)
struct SyncFileReadState
{
    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;
    bool success = false;
    bool was_opened = false;
    std::vector<uint8_t> data;

    bool WaitForDone(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return done; });
    }
};

// Synchronizing callback for stepfunc file read operations.
// Uses shared state so results survive after the stepfunc library deletes the reader.
class SyncFileReader : public dnp3::FileReader
{
public:
    std::shared_ptr<SyncFileReadState> state;

    explicit SyncFileReader(std::shared_ptr<SyncFileReadState> s) : state(std::move(s)) {}

    bool opened(uint32_t /*size*/) override
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        state->was_opened = true;
        return true;
    }

    bool block_received(uint32_t /*block_num*/, dnp3::ByteIterator& bytes) override
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        while (bytes.next())
        {
            state->data.push_back(bytes.get());
        }
        return true;
    }

    void aborted(dnp3::FileError /*error*/) override
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        state->success = false;
        state->done = true;
        state->cv.notify_all();
    }

    void completed() override
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        state->success = true;
        state->done = true;
        state->cv.notify_all();
    }
};

// ---------------------------------------------------------------------------
// Helper to create a loopback test fixture with an opendnp3 outstation
// (with file handler) and an opendnp3 master on TCP.
// ---------------------------------------------------------------------------

struct LoopbackFixture
{
    std::shared_ptr<TestFileHandler> fileHandler;
    opendnp3::DNP3Manager manager;
    std::shared_ptr<opendnp3::IChannel> serverChannel;
    std::shared_ptr<opendnp3::IChannel> clientChannel;
    std::shared_ptr<opendnp3::IOutstation> outstation;
    std::shared_ptr<opendnp3::IMaster> master;
    uint16_t port;

    LoopbackFixture() : fileHandler(std::make_shared<TestFileHandler>()), manager(1, opendnp3::ConsoleLogger::Create())
    {
        port = FindEphemeralPort();

        // Outstation (server)
        serverChannel
            = manager.AddTCPServer("server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                                   opendnp3::IPEndpoint("127.0.0.1", port), nullptr);

        opendnp3::OutstationStackConfig outCfg{opendnp3::DatabaseConfig()};
        outCfg.outstation.params.allowUnsolicited = false;
        outCfg.link.LocalAddr = 1024;
        outCfg.link.RemoteAddr = 1;

        outstation
            = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                           opendnp3::DefaultOutstationApplication::Create(), outCfg, fileHandler);

        outstation->Enable();

        // Master (client)
        clientChannel = manager.AddTCPClient("client", opendnp3::levels::NOTHING, opendnp3::ChannelRetry::Default(),
                                             {opendnp3::IPEndpoint("127.0.0.1", port)}, "127.0.0.1", nullptr);

        opendnp3::MasterStackConfig masterCfg;
        masterCfg.master.responseTimeout = opendnp3::TimeDuration::Seconds(5);
        masterCfg.master.disableUnsolOnStartup = true;
        masterCfg.master.startupIntegrityClassMask = opendnp3::ClassField::None();
        masterCfg.link.LocalAddr = 1;
        masterCfg.link.RemoteAddr = 1024;

        auto soeHandler = std::make_shared<CollectingSOEHandler>();
        master
            = clientChannel->AddMaster("master", soeHandler, opendnp3::DefaultMasterApplication::Create(), masterCfg);

        master->Enable();

        // Wait for connection
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    ~LoopbackFixture()
    {
        master->Disable();
    }
};

// ===========================================================================
// Tests
// ===========================================================================

// ---------------------------------------------------------------------------
// opendnp3 master reads a small file from opendnp3 outstation
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackReadSmallFile"))
{
    LoopbackFixture fix;

    // Add a small test file
    std::vector<uint8_t> testData = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    fix.fileHandler->AddFile("/test/hello.txt", testData);

    SyncFileReadCallback cb;
    fix.master->ReadFile("/test/hello.txt", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    REQUIRE(cb.result.data.size() == testData.size());
    CHECK(cb.result.data == testData);
}

// ---------------------------------------------------------------------------
// opendnp3 master reads a multi-block file from opendnp3 outstation
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackReadMultiBlockFile"))
{
    LoopbackFixture fix;

    // Create a file larger than one block (default max block = 2048)
    std::vector<uint8_t> testData(5000);
    std::iota(testData.begin(), testData.end(), 0);

    fix.fileHandler->AddFile("/test/large.bin", testData);

    SyncFileReadCallback cb;
    fix.master->ReadFile("/test/large.bin", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    REQUIRE(cb.result.data.size() == testData.size());
    CHECK(cb.result.data == testData);
}

// ---------------------------------------------------------------------------
// opendnp3 master reads nonexistent file - error path
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackReadFileNotFound"))
{
    LoopbackFixture fix;

    SyncFileReadCallback cb;
    fix.master->ReadFile("/nonexistent/file.txt", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    // The task should complete (not hang), and indicate the file doesn't exist
    CHECK(cb.result.statusCode == opendnp3::FileStatus::NOT_EXIST);
}

// ---------------------------------------------------------------------------
// opendnp3 master gets file info from opendnp3 outstation
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackGetFileInfo"))
{
    LoopbackFixture fix;

    opendnp3::FilePermissions perms;
    perms.owner = opendnp3::FilePermissionSet(true, true, false);
    perms.group = opendnp3::FilePermissionSet(true, false, false);
    perms.world = opendnp3::FilePermissionSet(true, false, false);

    std::vector<uint8_t> testData(42);
    fix.fileHandler->AddFile("/config/settings.dat", testData, perms);

    SyncFileInfoCallback cb;
    fix.master->GetFileInfo("/config/settings.dat", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    CHECK(cb.result.info.size == 42);
    CHECK(cb.result.info.type == opendnp3::FileType::SIMPLE_FILE);
    CHECK(cb.result.info.permissions.owner.read == true);
    CHECK(cb.result.info.permissions.owner.write == true);
    CHECK(cb.result.info.permissions.owner.execute == false);
    CHECK(cb.result.info.permissions.group.read == true);
    CHECK(cb.result.info.permissions.world.read == true);
}

// ---------------------------------------------------------------------------
// opendnp3 master gets file info for nonexistent file
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackGetFileInfoNotFound"))
{
    LoopbackFixture fix;

    SyncFileInfoCallback cb;
    fix.master->GetFileInfo("/does/not/exist.txt", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.statusCode == opendnp3::FileStatus::NOT_EXIST);
}

// ---------------------------------------------------------------------------
// opendnp3 master deletes a file on opendnp3 outstation
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackDeleteFile"))
{
    LoopbackFixture fix;

    fix.fileHandler->AddFile("/tmp/deleteme.txt", {1, 2, 3});
    CHECK(fix.fileHandler->HasFile("/tmp/deleteme.txt"));

    SyncFileOpCallback cb;
    fix.master->DeleteFile("/tmp/deleteme.txt", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cb.result.statusCode == opendnp3::FileStatus::SUCCESS);

    // Verify the file was actually deleted
    CHECK_FALSE(fix.fileHandler->HasFile("/tmp/deleteme.txt"));
}

// ---------------------------------------------------------------------------
// opendnp3 master deletes nonexistent file - error path
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackDeleteFileNotFound"))
{
    LoopbackFixture fix;

    SyncFileOpCallback cb;
    fix.master->DeleteFile("/nonexistent.txt", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.statusCode == opendnp3::FileStatus::NOT_EXIST);
}

// ---------------------------------------------------------------------------
// opendnp3 master writes a small file to opendnp3 outstation
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackWriteSmallFile"))
{
    LoopbackFixture fix;

    std::vector<uint8_t> testData = {0xDE, 0xAD, 0xBE, 0xEF};
    opendnp3::FilePermissions perms;
    perms.owner = opendnp3::FilePermissionSet(true, true, false);

    SyncFileWriteCallback cb;
    fix.master->WriteFile("/output/test.bin", testData, perms, cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cb.result.statusCode == opendnp3::FileStatus::SUCCESS);

    // Verify the file was written with correct content
    auto stored = fix.fileHandler->GetFileData("/output/test.bin");
    REQUIRE(stored.size() == testData.size());
    CHECK(stored == testData);
}

// ---------------------------------------------------------------------------
// opendnp3 master writes a multi-block file to opendnp3 outstation
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackWriteMultiBlockFile"))
{
    LoopbackFixture fix;

    // Create data larger than one block
    std::vector<uint8_t> testData(5000);
    std::iota(testData.begin(), testData.end(), 0);

    opendnp3::FilePermissions perms;
    perms.owner = opendnp3::FilePermissionSet(true, true, true);

    SyncFileWriteCallback cb;
    fix.master->WriteFile("/output/big.bin", testData, perms, cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cb.result.statusCode == opendnp3::FileStatus::SUCCESS);

    // Verify content
    auto stored = fix.fileHandler->GetFileData("/output/big.bin");
    REQUIRE(stored.size() == testData.size());
    CHECK(stored == testData);
}

// ---------------------------------------------------------------------------
// opendnp3 master writes then reads back (round-trip)
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackWriteThenReadRoundTrip"))
{
    LoopbackFixture fix;

    std::vector<uint8_t> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    opendnp3::FilePermissions perms;

    // Write
    SyncFileWriteCallback wcb;
    fix.master->WriteFile("/roundtrip.dat", testData, perms, wcb.Callback());
    REQUIRE(wcb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(wcb.result.summary == opendnp3::TaskCompletion::SUCCESS);

    // Wait for outstation to process
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Read back
    SyncFileReadCallback rcb;
    fix.master->ReadFile("/roundtrip.dat", rcb.Callback());
    REQUIRE(rcb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(rcb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(rcb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    REQUIRE(rcb.result.data.size() == testData.size());
    CHECK(rcb.result.data == testData);
}

// ---------------------------------------------------------------------------
// stepfunc master reads file from opendnp3 outstation
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("StepfuncMasterReadsFromOpenDNP3Outstation"))
{
    auto fileHandler = std::make_shared<TestFileHandler>();
    std::vector<uint8_t> testData = {0x41, 0x42, 0x43, 0x44, 0x45}; // "ABCDE"
    fileHandler->AddFile("/test/abc.txt", testData);

    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- opendnp3 outstation (server) --
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto serverChannel
        = manager.AddTCPServer("server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                               opendnp3::IPEndpoint("127.0.0.1", port), nullptr);

    opendnp3::OutstationStackConfig outCfg{opendnp3::DatabaseConfig()};
    outCfg.outstation.params.allowUnsolicited = false;
    outCfg.link.LocalAddr = 1024;
    outCfg.link.RemoteAddr = 1;

    auto outstation
        = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                       opendnp3::DefaultOutstationApplication::Create(), outCfg, fileHandler);

    outstation->Enable();

    // -- stepfunc master (client) --
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    dnp3::MasterChannelConfig sfMasterCfg(1);
    dnp3::EndpointList endpoints(endpoint);

    auto sfMaster
        = dnp3::MasterChannel::create_tcp_channel(sfRuntime, dnp3::LinkErrorMode::close, sfMasterCfg, endpoints,
                                                  dnp3::ConnectStrategy(), std::make_unique<NullClientStateListener>());

    dnp3::AssociationConfig assocCfg(dnp3::EventClasses::none(), dnp3::EventClasses::none(),
                                     dnp3::Classes(false, false, false, false), dnp3::EventClasses::none());

    auto assocId = sfMaster.add_association(
        1024, assocCfg, std::make_unique<CollectingReadHandler>(),
        dnp3::functional::association_handler([]() -> dnp3::UtcTimestamp { return dnp3::UtcTimestamp::invalid(); }),
        std::make_unique<NullAssociationInfo>());

    sfMaster.enable();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Read the file
    auto readState = std::make_shared<SyncFileReadState>();
    sfMaster.read_file(assocId, "/test/abc.txt", dnp3::FileReadConfig::defaults(),
                       std::make_unique<SyncFileReader>(readState));

    REQUIRE(readState->WaitForDone(INTEROP_TIMEOUT));
    CHECK(readState->success);
    CHECK(readState->was_opened);
    REQUIRE(readState->data.size() == testData.size());
    CHECK(readState->data == testData);
}

// ---------------------------------------------------------------------------
// stepfunc master reads multi-block file from opendnp3 outstation
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("StepfuncMasterReadsLargeFileFromOpenDNP3Outstation"))
{
    auto fileHandler = std::make_shared<TestFileHandler>();
    std::vector<uint8_t> testData(5000);
    std::iota(testData.begin(), testData.end(), 0);
    fileHandler->AddFile("/test/large.bin", testData);

    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- opendnp3 outstation --
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto serverChannel
        = manager.AddTCPServer("server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                               opendnp3::IPEndpoint("127.0.0.1", port), nullptr);

    opendnp3::OutstationStackConfig outCfg{opendnp3::DatabaseConfig()};
    outCfg.outstation.params.allowUnsolicited = false;
    outCfg.link.LocalAddr = 1024;
    outCfg.link.RemoteAddr = 1;

    auto outstation
        = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                       opendnp3::DefaultOutstationApplication::Create(), outCfg, fileHandler);

    outstation->Enable();

    // -- stepfunc master --
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    dnp3::MasterChannelConfig sfMasterCfg(1);
    dnp3::EndpointList endpoints(endpoint);

    auto sfMaster
        = dnp3::MasterChannel::create_tcp_channel(sfRuntime, dnp3::LinkErrorMode::close, sfMasterCfg, endpoints,
                                                  dnp3::ConnectStrategy(), std::make_unique<NullClientStateListener>());

    dnp3::AssociationConfig assocCfg(dnp3::EventClasses::none(), dnp3::EventClasses::none(),
                                     dnp3::Classes(false, false, false, false), dnp3::EventClasses::none());

    auto assocId = sfMaster.add_association(
        1024, assocCfg, std::make_unique<CollectingReadHandler>(),
        dnp3::functional::association_handler([]() -> dnp3::UtcTimestamp { return dnp3::UtcTimestamp::invalid(); }),
        std::make_unique<NullAssociationInfo>());

    sfMaster.enable();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto readState = std::make_shared<SyncFileReadState>();
    sfMaster.read_file(assocId, "/test/large.bin", dnp3::FileReadConfig::defaults(),
                       std::make_unique<SyncFileReader>(readState));

    REQUIRE(readState->WaitForDone(INTEROP_TIMEOUT));
    CHECK(readState->success);
    REQUIRE(readState->data.size() == testData.size());
    CHECK(readState->data == testData);
}

// ---------------------------------------------------------------------------
// stepfunc master reads nonexistent file from opendnp3 outstation - error path
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("StepfuncMasterReadsNonexistentFile"))
{
    auto fileHandler = std::make_shared<TestFileHandler>();

    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto serverChannel
        = manager.AddTCPServer("server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                               opendnp3::IPEndpoint("127.0.0.1", port), nullptr);

    opendnp3::OutstationStackConfig outCfg{opendnp3::DatabaseConfig()};
    outCfg.outstation.params.allowUnsolicited = false;
    outCfg.link.LocalAddr = 1024;
    outCfg.link.RemoteAddr = 1;

    auto outstation
        = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                       opendnp3::DefaultOutstationApplication::Create(), outCfg, fileHandler);

    outstation->Enable();

    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));
    dnp3::MasterChannelConfig sfMasterCfg(1);
    dnp3::EndpointList endpoints(endpoint);

    auto sfMaster
        = dnp3::MasterChannel::create_tcp_channel(sfRuntime, dnp3::LinkErrorMode::close, sfMasterCfg, endpoints,
                                                  dnp3::ConnectStrategy(), std::make_unique<NullClientStateListener>());

    dnp3::AssociationConfig assocCfg(dnp3::EventClasses::none(), dnp3::EventClasses::none(),
                                     dnp3::Classes(false, false, false, false), dnp3::EventClasses::none());

    auto assocId = sfMaster.add_association(
        1024, assocCfg, std::make_unique<CollectingReadHandler>(),
        dnp3::functional::association_handler([]() -> dnp3::UtcTimestamp { return dnp3::UtcTimestamp::invalid(); }),
        std::make_unique<NullAssociationInfo>());

    sfMaster.enable();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto readState = std::make_shared<SyncFileReadState>();
    sfMaster.read_file(assocId, "/does/not/exist.txt", dnp3::FileReadConfig::defaults(),
                       std::make_unique<SyncFileReader>(readState));

    REQUIRE(readState->WaitForDone(INTEROP_TIMEOUT));
    CHECK_FALSE(readState->success);
}

// ---------------------------------------------------------------------------
// opendnp3 master reads from stepfunc outstation (no file handler - error path)
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("OpenDNP3MasterReadFileFromStepfuncOutstation"))
{
    const uint16_t port = FindEphemeralPort();
    const std::string endpoint = MakeEndpoint(port);

    // -- stepfunc outstation (no file handler) --
    dnp3::Runtime sfRuntime((dnp3::RuntimeConfig()));

    auto sfServer = dnp3::OutstationServer::create_tcp_server(sfRuntime, dnp3::LinkErrorMode::close, endpoint);

    dnp3::OutstationConfig sfOutCfg(1024, 1, dnp3::EventBufferConfig::no_events());
    sfOutCfg.features.unsolicited = false;

    auto sfFilter = dnp3::AddressFilter::any();

    sfServer.add_outstation(sfOutCfg, std::make_unique<NullOutstationApplication>(),
                            std::make_unique<NullOutstationInformation>(), std::make_unique<NullControlHandler>(),
                            std::make_unique<NullConnectionStateListener>(), sfFilter);

    sfServer.bind();

    // -- opendnp3 master --
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto channel = manager.AddTCPClient("client", opendnp3::levels::NOTHING, opendnp3::ChannelRetry::Default(),
                                        {opendnp3::IPEndpoint("127.0.0.1", port)}, "127.0.0.1", nullptr);

    opendnp3::MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = opendnp3::TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = opendnp3::ClassField::None();
    masterCfg.link.LocalAddr = 1;
    masterCfg.link.RemoteAddr = 1024;

    auto master = channel->AddMaster("master", std::make_shared<CollectingSOEHandler>(),
                                     opendnp3::DefaultMasterApplication::Create(), masterCfg);

    master->Enable();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    SyncFileReadCallback fileCallback;
    master->ReadFile("/test/nonexistent.txt", fileCallback.Callback());

    REQUIRE(fileCallback.WaitForCompletion(INTEROP_TIMEOUT));
    // The stepfunc outstation doesn't support file transfer,
    // so we just verify it completes without hanging
    CHECK(fileCallback.completed);

    master->Disable();
}

// ---------------------------------------------------------------------------
// opendnp3 outstation without file handler returns FUNC_NOT_SUPPORTED
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("OutstationNoFileHandlerReturnsNotSupported"))
{
    const uint16_t port = FindEphemeralPort();

    // Outstation without file handler
    opendnp3::DNP3Manager manager(1, opendnp3::ConsoleLogger::Create());

    auto serverChannel
        = manager.AddTCPServer("server", opendnp3::levels::NOTHING, opendnp3::ServerAcceptMode::CloseExisting,
                               opendnp3::IPEndpoint("127.0.0.1", port), nullptr);

    opendnp3::OutstationStackConfig outCfg{opendnp3::DatabaseConfig()};
    outCfg.outstation.params.allowUnsolicited = false;
    outCfg.link.LocalAddr = 1024;
    outCfg.link.RemoteAddr = 1;

    // No file handler passed (nullptr)
    auto outstation = serverChannel->AddOutstation("outstation", opendnp3::SuccessCommandHandler::Create(),
                                                   opendnp3::DefaultOutstationApplication::Create(), outCfg);

    outstation->Enable();

    auto clientChannel = manager.AddTCPClient("client", opendnp3::levels::NOTHING, opendnp3::ChannelRetry::Default(),
                                              {opendnp3::IPEndpoint("127.0.0.1", port)}, "127.0.0.1", nullptr);

    opendnp3::MasterStackConfig masterCfg;
    masterCfg.master.responseTimeout = opendnp3::TimeDuration::Seconds(5);
    masterCfg.master.disableUnsolOnStartup = true;
    masterCfg.master.startupIntegrityClassMask = opendnp3::ClassField::None();
    masterCfg.link.LocalAddr = 1;
    masterCfg.link.RemoteAddr = 1024;

    auto master = clientChannel->AddMaster("master", std::make_shared<CollectingSOEHandler>(),
                                           opendnp3::DefaultMasterApplication::Create(), masterCfg);

    master->Enable();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Read should fail with FUNC_NOT_SUPPORTED (IIN bit set)
    SyncFileReadCallback cb;
    master->ReadFile("/any/file.txt", cb.Callback());
    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    // The task should complete, and the result should indicate
    // a non-success status code
    CHECK(cb.result.statusCode != opendnp3::FileStatus::SUCCESS);

    master->Disable();
}

// ---------------------------------------------------------------------------
// FilePermissions round-trip
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("FilePermissionsRoundTrip"))
{
    // Test all permission bits
    opendnp3::FilePermissions perms;
    perms.owner = opendnp3::FilePermissionSet(true, true, true);
    perms.group = opendnp3::FilePermissionSet(true, false, true);
    perms.world = opendnp3::FilePermissionSet(true, false, false);

    uint16_t raw = perms.ToRaw();
    auto decoded = opendnp3::FilePermissions::FromRaw(raw);

    CHECK(decoded.owner.read == true);
    CHECK(decoded.owner.write == true);
    CHECK(decoded.owner.execute == true);
    CHECK(decoded.group.read == true);
    CHECK(decoded.group.write == false);
    CHECK(decoded.group.execute == true);
    CHECK(decoded.world.read == true);
    CHECK(decoded.world.write == false);
    CHECK(decoded.world.execute == false);

    CHECK(raw == 0x01EC);
}

// ---------------------------------------------------------------------------
// All permission bits individually
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("FilePermissionsAllBits"))
{
    // All bits set = 0x01FF (9 bits)
    auto all = opendnp3::FilePermissions::FromRaw(0x01FF);
    CHECK(all.owner.read == true);
    CHECK(all.owner.write == true);
    CHECK(all.owner.execute == true);
    CHECK(all.group.read == true);
    CHECK(all.group.write == true);
    CHECK(all.group.execute == true);
    CHECK(all.world.read == true);
    CHECK(all.world.write == true);
    CHECK(all.world.execute == true);
    CHECK(all.ToRaw() == 0x01FF);

    // No bits
    auto none = opendnp3::FilePermissions::FromRaw(0x0000);
    CHECK(none.owner.read == false);
    CHECK(none.owner.write == false);
    CHECK(none.owner.execute == false);
    CHECK(none.group.read == false);
    CHECK(none.group.write == false);
    CHECK(none.group.execute == false);
    CHECK(none.world.read == false);
    CHECK(none.world.write == false);
    CHECK(none.world.execute == false);
    CHECK(none.ToRaw() == 0x0000);
}

// ===========================================================================
// Synchronizing callbacks for new features
// ===========================================================================

class SyncDirectoryReadCallback
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    opendnp3::DirectoryReadResult result;

    opendnp3::DirectoryReadCallbackT Callback()
    {
        return [this](const opendnp3::DirectoryReadResult& r) {
            std::lock_guard<std::mutex> lock(mutex);
            result = r;
            completed = true;
            cv.notify_all();
        };
    }

    bool WaitForCompletion(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return completed; });
    }
};

class SyncFileAuthCallback
{
public:
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    opendnp3::FileAuthResult_t result;

    opendnp3::FileAuthCallbackT Callback()
    {
        return [this](const opendnp3::FileAuthResult_t& r) {
            std::lock_guard<std::mutex> lock(mutex);
            result = r;
            completed = true;
            cv.notify_all();
        };
    }

    bool WaitForCompletion(std::chrono::steady_clock::duration timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return completed; });
    }
};

// ===========================================================================
// ReadDirectory tests
// ===========================================================================

// ---------------------------------------------------------------------------
// opendnp3 master reads a directory with multiple entries
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackReadDirectory"))
{
    LoopbackFixture fix;

    // Seed directory entries
    std::vector<opendnp3::FileInfo> dirEntries;
    {
        opendnp3::FileInfo f1;
        f1.fileName = "config.xml";
        f1.type = opendnp3::FileType::SIMPLE_FILE;
        f1.size = 1024;
        f1.timeOfCreation = 1000000;
        f1.permissions = opendnp3::FilePermissions::FromRaw(0x01FF);
        dirEntries.push_back(f1);

        opendnp3::FileInfo f2;
        f2.fileName = "data.bin";
        f2.type = opendnp3::FileType::SIMPLE_FILE;
        f2.size = 50000;
        f2.timeOfCreation = 2000000;
        f2.permissions = opendnp3::FilePermissions::FromRaw(0x01A4);
        dirEntries.push_back(f2);

        opendnp3::FileInfo f3;
        f3.fileName = "subdir";
        f3.type = opendnp3::FileType::DIRECTORY;
        f3.size = 0;
        f3.timeOfCreation = 3000000;
        f3.permissions = opendnp3::FilePermissions::FromRaw(0x01ED);
        dirEntries.push_back(f3);
    }

    fix.fileHandler->AddDirectory("/config", dirEntries);

    SyncDirectoryReadCallback cb;
    fix.master->ReadDirectory("/config", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    REQUIRE(cb.result.entries.size() == 3);

    CHECK(cb.result.entries[0].fileName == "config.xml");
    CHECK(cb.result.entries[0].type == opendnp3::FileType::SIMPLE_FILE);
    CHECK(cb.result.entries[0].size == 1024);
    CHECK(cb.result.entries[0].timeOfCreation == 1000000);
    CHECK(cb.result.entries[0].permissions.ToRaw() == 0x01FF);

    CHECK(cb.result.entries[1].fileName == "data.bin");
    CHECK(cb.result.entries[1].type == opendnp3::FileType::SIMPLE_FILE);
    CHECK(cb.result.entries[1].size == 50000);
    CHECK(cb.result.entries[1].timeOfCreation == 2000000);
    CHECK(cb.result.entries[1].permissions.ToRaw() == 0x01A4);

    CHECK(cb.result.entries[2].fileName == "subdir");
    CHECK(cb.result.entries[2].type == opendnp3::FileType::DIRECTORY);
    CHECK(cb.result.entries[2].size == 0);
    CHECK(cb.result.entries[2].timeOfCreation == 3000000);
}

// ---------------------------------------------------------------------------
// opendnp3 master reads nonexistent directory - error path
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackReadDirectoryNotFound"))
{
    LoopbackFixture fix;

    SyncDirectoryReadCallback cb;
    fix.master->ReadDirectory("/nonexistent/dir", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.statusCode == opendnp3::FileStatus::NOT_EXIST);
    CHECK(cb.result.entries.empty());
}

// ===========================================================================
// AbortFile tests
// ===========================================================================

// ---------------------------------------------------------------------------
// opendnp3 master aborts a file transfer with a known handle
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackAbortFile"))
{
    LoopbackFixture fix;

    // Open a file first to get a handle
    fix.fileHandler->AddFile("/test/abort_me.txt", {1, 2, 3, 4, 5});

    // Read the file (this opens it, reads blocks, closes it)
    // Instead, we manually open a file in the handler to get a known handle
    {
        std::lock_guard<std::mutex> lock(fix.fileHandler->mutex);
        fix.fileHandler->openHandles[42] = {"/test/abort_me.txt", opendnp3::FileMode::READ, 2048, {}};
    }

    SyncFileOpCallback cb;
    fix.master->AbortFile(42, cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);

    // Verify the outstation received the abort
    CHECK(fix.fileHandler->abortCalled);
    CHECK(fix.fileHandler->lastAbortedHandle == 42);

    // Verify the handle was removed
    {
        std::lock_guard<std::mutex> lock(fix.fileHandler->mutex);
        CHECK(fix.fileHandler->openHandles.find(42) == fix.fileHandler->openHandles.end());
    }
}

// ===========================================================================
// AuthenticateFile tests
// ===========================================================================

// ---------------------------------------------------------------------------
// opendnp3 master authenticates with valid credentials
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackAuthenticateFileSuccess"))
{
    LoopbackFixture fix;

    SyncFileAuthCallback cb;
    fix.master->AuthenticateFile("admin", "secret", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    CHECK(cb.result.authKey == 12345);
}

// ---------------------------------------------------------------------------
// opendnp3 master authenticates with invalid credentials
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackAuthenticateFileFailure"))
{
    LoopbackFixture fix;

    SyncFileAuthCallback cb;
    fix.master->AuthenticateFile("wrong_user", "wrong_pass", cb.Callback());

    REQUIRE(cb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(cb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cb.result.statusCode == opendnp3::FileStatus::PERMISSION_DENIED);
    CHECK(cb.result.authKey == 0);
}

// ---------------------------------------------------------------------------
// opendnp3 master authenticates then opens file
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackAuthenticateThenReadFile"))
{
    LoopbackFixture fix;

    std::vector<uint8_t> testData = {0xCA, 0xFE, 0xBA, 0xBE};
    fix.fileHandler->AddFile("/secure/data.bin", testData);

    // Authenticate first
    SyncFileAuthCallback authCb;
    fix.master->AuthenticateFile("admin", "secret", authCb.Callback());
    REQUIRE(authCb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(authCb.result.statusCode == opendnp3::FileStatus::SUCCESS);

    // Then read the file
    SyncFileReadCallback readCb;
    fix.master->ReadFile("/secure/data.bin", readCb.Callback());
    REQUIRE(readCb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(readCb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(readCb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    REQUIRE(readCb.result.data.size() == testData.size());
    CHECK(readCb.result.data == testData);
}

// ===========================================================================
// Additional test cases for coverage gaps
// ===========================================================================

// ---------------------------------------------------------------------------
// opendnp3 master writes file in APPEND mode, verifying data is appended
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackWriteFileAppendMode"))
{
    LoopbackFixture fix;

    // Seed an existing file with initial data
    std::vector<uint8_t> initialData = {0x01, 0x02, 0x03, 0x04};
    fix.fileHandler->AddFile("/test/append.bin", initialData);

    // Write additional data in APPEND mode
    std::vector<uint8_t> appendData = {0x05, 0x06, 0x07, 0x08};
    opendnp3::FilePermissions perms;
    perms.owner = opendnp3::FilePermissionSet(true, true, false);

    SyncFileWriteCallback wcb;
    fix.master->WriteFile("/test/append.bin", appendData, perms, opendnp3::FileMode::APPEND, 0, wcb.Callback());

    REQUIRE(wcb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(wcb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(wcb.result.statusCode == opendnp3::FileStatus::SUCCESS);

    // Verify the file now contains original + appended data
    std::vector<uint8_t> expected = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    auto stored = fix.fileHandler->GetFileData("/test/append.bin");
    REQUIRE(stored.size() == expected.size());
    CHECK(stored == expected);

    // Read back via the master to double-check
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    SyncFileReadCallback rcb;
    fix.master->ReadFile("/test/append.bin", rcb.Callback());
    REQUIRE(rcb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(rcb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(rcb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    REQUIRE(rcb.result.data.size() == expected.size());
    CHECK(rcb.result.data == expected);
}

// ---------------------------------------------------------------------------
// Concurrent file operations: read two files simultaneously
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackConcurrentFileReads"))
{
    LoopbackFixture fix;

    // Seed two distinct files
    std::vector<uint8_t> dataA = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    std::vector<uint8_t> dataB(3000);
    std::iota(dataB.begin(), dataB.end(), 0);

    fix.fileHandler->AddFile("/concurrent/fileA.bin", dataA);
    fix.fileHandler->AddFile("/concurrent/fileB.bin", dataB);

    // Issue both reads concurrently (they will be queued as adhoc tasks)
    SyncFileReadCallback cbA;
    SyncFileReadCallback cbB;
    fix.master->ReadFile("/concurrent/fileA.bin", cbA.Callback());
    fix.master->ReadFile("/concurrent/fileB.bin", cbB.Callback());

    // Both should complete successfully
    REQUIRE(cbA.WaitForCompletion(INTEROP_TIMEOUT));
    REQUIRE(cbB.WaitForCompletion(INTEROP_TIMEOUT));

    CHECK(cbA.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cbA.result.statusCode == opendnp3::FileStatus::SUCCESS);
    REQUIRE(cbA.result.data.size() == dataA.size());
    CHECK(cbA.result.data == dataA);

    CHECK(cbB.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(cbB.result.statusCode == opendnp3::FileStatus::SUCCESS);
    REQUIRE(cbB.result.data.size() == dataB.size());
    CHECK(cbB.result.data == dataB);
}

// ---------------------------------------------------------------------------
// Empty file edge case: write and read a zero-byte file
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackEmptyFileReadWrite"))
{
    LoopbackFixture fix;

    // Write an empty file
    std::vector<uint8_t> emptyData;
    opendnp3::FilePermissions perms;
    perms.owner = opendnp3::FilePermissionSet(true, true, false);

    SyncFileWriteCallback wcb;
    fix.master->WriteFile("/test/empty.bin", emptyData, perms, wcb.Callback());

    REQUIRE(wcb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(wcb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(wcb.result.statusCode == opendnp3::FileStatus::SUCCESS);

    // Verify file exists with zero bytes
    auto stored = fix.fileHandler->GetFileData("/test/empty.bin");
    CHECK(stored.empty());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Read back the empty file
    SyncFileReadCallback rcb;
    fix.master->ReadFile("/test/empty.bin", rcb.Callback());
    REQUIRE(rcb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(rcb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(rcb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    CHECK(rcb.result.data.empty());

    // Also test reading a pre-seeded empty file
    fix.fileHandler->AddFile("/test/preexist_empty.bin", {});
    SyncFileReadCallback rcb2;
    fix.master->ReadFile("/test/preexist_empty.bin", rcb2.Callback());
    REQUIRE(rcb2.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(rcb2.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(rcb2.result.data.empty());
}

// ---------------------------------------------------------------------------
// Auth key propagation: verify AuthenticateFile key is received by OpenFile
// ---------------------------------------------------------------------------
TEST_CASE(SUITE("LoopbackAuthKeyPropagationToOpenFile"))
{
    LoopbackFixture fix;

    std::vector<uint8_t> testData = {0xDE, 0xAD};
    fix.fileHandler->AddFile("/secure/key_test.bin", testData);

    // Authenticate first to get a key
    SyncFileAuthCallback authCb;
    fix.master->AuthenticateFile("admin", "secret", authCb.Callback());
    REQUIRE(authCb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(authCb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    REQUIRE(authCb.result.authKey == 12345);

    uint32_t authKey = authCb.result.authKey;

    // Read the file using the overload that passes the auth key
    SyncFileReadCallback readCb;
    fix.master->ReadFile("/secure/key_test.bin", authKey, readCb.Callback());
    REQUIRE(readCb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(readCb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(readCb.result.statusCode == opendnp3::FileStatus::SUCCESS);
    REQUIRE(readCb.result.data.size() == testData.size());
    CHECK(readCb.result.data == testData);

    // Verify the outstation's file handler received the correct auth key
    CHECK(fix.fileHandler->lastReceivedAuthKey == 12345);

    // Also verify auth key propagation with WriteFile
    std::vector<uint8_t> writeData = {0xCA, 0xFE};
    opendnp3::FilePermissions perms;
    perms.owner = opendnp3::FilePermissionSet(true, true, false);

    SyncFileWriteCallback writeCb;
    fix.master->WriteFile("/secure/write_key_test.bin", writeData, perms, opendnp3::FileMode::WRITE, authKey,
                          writeCb.Callback());
    REQUIRE(writeCb.WaitForCompletion(INTEROP_TIMEOUT));
    CHECK(writeCb.result.summary == opendnp3::TaskCompletion::SUCCESS);
    CHECK(writeCb.result.statusCode == opendnp3::FileStatus::SUCCESS);

    // Verify the outstation received the auth key on the write open
    CHECK(fix.fileHandler->lastReceivedAuthKey == 12345);
}
