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
#ifndef OPENDNP3_IFILEHANDLER_H
#define OPENDNP3_IFILEHANDLER_H

#include "opendnp3/master/FileOperationResult.h"

#include <cstdint>
#include <string>
#include <vector>

namespace opendnp3
{

/// Result of an OPEN_FILE request
struct FileOpenResult
{
    FileStatus status = FileStatus::NOT_EXIST;
    uint32_t fileHandle = 0;
    uint32_t fileSize = 0;
    uint16_t maxBlockSize = 2048;
};

/// Result of a READ_BLOCK request
struct FileBlockResult
{
    FileStatus status = FileStatus::SUCCESS;
    std::vector<uint8_t> data;
    bool lastBlock = false;
};

/// Result of a GET_FILE_INFO request
struct FileCommandResult
{
    FileStatus status = FileStatus::NOT_EXIST;
    FileInfo info;
};

/// Result of an AUTHENTICATE_FILE request
struct FileAuthResult
{
    FileStatus status = FileStatus::PERMISSION_DENIED;
    uint32_t authKey = 0;
};

/**
 * Interface for outstation file transfer operations.
 *
 * Applications implement this interface to provide file serving capability
 * on the outstation side. When a master sends file transfer function codes
 * (OPEN_FILE, CLOSE_FILE, READ, DELETE_FILE, GET_FILE_INFO, etc.),
 * the outstation will dispatch to the corresponding method.
 *
 * If no IFileHandler is provided, the outstation returns IIN FUNC_NOT_SUPPORTED.
 */
class IFileHandler
{
public:
    virtual ~IFileHandler() = default;

    /// Called for GET_FILE_INFO (FC=0x1C)
    virtual FileCommandResult GetFileInfo(const std::string& filename) = 0;

    /// Called for OPEN_FILE (FC=0x19)
    virtual FileOpenResult OpenFile(const std::string& filename,
                                    uint32_t authKey,
                                    FilePermissions permissions,
                                    FileMode mode,
                                    uint16_t maxBlockSize,
                                    uint16_t requestId)
        = 0;

    /// Called for READ of Group70Var5 (read block from open file)
    virtual FileBlockResult ReadBlock(uint32_t fileHandle, uint32_t blockNum) = 0;

    /// Called for WRITE of Group70Var5 (write block to open file)
    virtual FileStatus WriteBlock(
        uint32_t fileHandle, uint32_t blockNum, bool lastBlock, const uint8_t* data, size_t size)
        = 0;

    /// Called for CLOSE_FILE (FC=0x1A)
    virtual FileStatus CloseFile(uint32_t fileHandle, uint16_t requestId) = 0;

    /// Called for DELETE_FILE (FC=0x1B)
    virtual FileStatus DeleteFile(const std::string& filename) = 0;

    /// Called for ABORT_FILE (FC=0x1E)
    virtual void AbortFile(uint32_t fileHandle) = 0;

    /// Called for AUTHENTICATE_FILE (FC=0x1D)
    virtual FileAuthResult AuthenticateFile(const std::string& username, const std::string& password) = 0;
};

} // namespace opendnp3

#endif
