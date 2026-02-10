/*
 * Copyright 2013-2022 Step Function I/O, LLC
 *
 * Licensed to Green Energy Corp (www.greenenergycorp.com) and Step Function I/O
 * LLC (https://stepfunc.io) under one or more contributor license agreements.
 * See the NOTICE file distributed with this work for additional information
 * regarding copyright ownership. Green Energy Corp and Step Function I/O LLC license
 * this file to you under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OPENDNP3_FILE_OPERATION_RESULT_H
#define OPENDNP3_FILE_OPERATION_RESULT_H

#include "opendnp3/gen/TaskCompletion.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace opendnp3
{

/// DNP3 file transfer mode (Group70Var3 mode field)
enum class FileMode : uint16_t
{
    READ = 1,
    WRITE = 2,
    APPEND = 3
};

/// DNP3 file status codes returned in Group70Var4 and Group70Var6
enum class FileStatus : uint8_t
{
    SUCCESS = 0,
    PERMISSION_DENIED = 1,
    INVALID_MODE = 2,
    FILE_NOT_FOUND = 3,
    FILE_LOCKED = 4,
    NOT_OPENED = 5,
    CLOSE_ABORT = 6,
    NOT_EXIST = 7,
    HANDLE_EXPIRED = 16,
    BUFFER_OVERFLOW = 17,
    FATAL = 18,
    BLOCK_SEQ = 19
};

/// DNP3 file type field in Group70Var7
enum class FileType : uint16_t
{
    DIRECTORY = 0x0000,
    SIMPLE_FILE = 0x0001
};

/// UNIX-style permission set for a single user/group/world class
struct FilePermissionSet
{
    bool read = false;
    bool write = false;
    bool execute = false;

    FilePermissionSet() = default;
    FilePermissionSet(bool r, bool w, bool x) : read(r), write(w), execute(x) {}
};

/// DNP3 file permissions (16-bit UNIX-style)
/// Bit layout (from LSB): world_execute(0), world_write(1), world_read(2),
///   group_execute(3), group_write(4), group_read(5),
///   owner_execute(6), owner_write(7), owner_read(8)
struct FilePermissions
{
    FilePermissionSet owner;
    FilePermissionSet group;
    FilePermissionSet world;

    FilePermissions() = default;
    FilePermissions(FilePermissionSet o, FilePermissionSet g, FilePermissionSet w) : owner(o), group(g), world(w) {}

    /// Construct from the raw 16-bit DNP3 permission field
    static FilePermissions FromRaw(uint16_t raw)
    {
        FilePermissions p;
        p.world.execute = (raw & 0x001) != 0;
        p.world.write = (raw & 0x002) != 0;
        p.world.read = (raw & 0x004) != 0;
        p.group.execute = (raw & 0x008) != 0;
        p.group.write = (raw & 0x010) != 0;
        p.group.read = (raw & 0x020) != 0;
        p.owner.execute = (raw & 0x040) != 0;
        p.owner.write = (raw & 0x080) != 0;
        p.owner.read = (raw & 0x100) != 0;
        return p;
    }

    /// Convert to the raw 16-bit DNP3 permission field
    uint16_t ToRaw() const
    {
        uint16_t raw = 0;
        if (world.execute)
            raw |= 0x001;
        if (world.write)
            raw |= 0x002;
        if (world.read)
            raw |= 0x004;
        if (group.execute)
            raw |= 0x008;
        if (group.write)
            raw |= 0x010;
        if (group.read)
            raw |= 0x020;
        if (owner.execute)
            raw |= 0x040;
        if (owner.write)
            raw |= 0x080;
        if (owner.read)
            raw |= 0x100;
        return raw;
    }
};

/// Information about a file returned from GET_FILE_INFO (Group70Var7)
struct FileInfo
{
    std::string fileName;
    FileType type = FileType::SIMPLE_FILE;
    uint32_t size = 0;
    uint64_t timeOfCreation = 0;
    FilePermissions permissions;
    uint16_t requestId = 0;
};

/// Status returned by OPEN_FILE, CLOSE_FILE, DELETE_FILE (Group70Var4)
struct FileCommandStatus
{
    uint32_t fileHandle = 0;
    uint32_t fileSize = 0;
    uint16_t maxBlockSize = 0;
    uint16_t requestId = 0;
    FileStatus statusCode = FileStatus::SUCCESS;
    std::string optionalText;
};

/// Result of a file read operation
struct FileReadResult
{
    TaskCompletion summary = TaskCompletion::FAILURE_NO_COMMS;
    FileStatus statusCode = FileStatus::SUCCESS;
    std::vector<uint8_t> data;
};

/// Result of a get-file-info operation
struct FileInfoResult
{
    TaskCompletion summary = TaskCompletion::FAILURE_NO_COMMS;
    FileStatus statusCode = FileStatus::SUCCESS;
    FileInfo info;
};

/// Result of a simple file operation (delete, etc.)
struct FileOperationResult
{
    TaskCompletion summary = TaskCompletion::FAILURE_NO_COMMS;
    FileStatus statusCode = FileStatus::SUCCESS;
};

/// Result of a file write operation
struct FileWriteResult
{
    TaskCompletion summary = TaskCompletion::FAILURE_NO_COMMS;
    FileStatus statusCode = FileStatus::SUCCESS;
};

/// Result of a directory read operation
struct DirectoryReadResult
{
    TaskCompletion summary = TaskCompletion::FAILURE_NO_COMMS;
    FileStatus statusCode = FileStatus::SUCCESS;
    std::vector<FileInfo> entries;
};

/// Result of a file authentication operation
struct FileAuthResult_t
{
    TaskCompletion summary = TaskCompletion::FAILURE_NO_COMMS;
    FileStatus statusCode = FileStatus::SUCCESS;
    uint32_t authKey = 0;
};

typedef std::function<void(const FileReadResult&)> FileReadCallbackT;
typedef std::function<void(const FileInfoResult&)> FileInfoCallbackT;
typedef std::function<void(const FileOperationResult&)> FileOperationCallbackT;
typedef std::function<void(const FileWriteResult&)> FileWriteCallbackT;
typedef std::function<void(const DirectoryReadResult&)> DirectoryReadCallbackT;
typedef std::function<void(const FileAuthResult_t&)> FileAuthCallbackT;

} // namespace opendnp3

#endif
