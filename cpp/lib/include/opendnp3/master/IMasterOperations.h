/*
 * Copyright 2013-2022 Step Function I/O, LLC
 * Modified 2024-2026 f0rw4rd (experimental fork)
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
#ifndef OPENDNP3_IMASTEROPERATIONS_H
#define OPENDNP3_IMASTEROPERATIONS_H

#include "opendnp3/StackStatistics.h"
#include "opendnp3/app/ClassField.h"
#include "opendnp3/app/MeasurementTypes.h"
#include "opendnp3/gen/FreezeType.h"
#include "opendnp3/gen/FunctionCode.h"
#include "opendnp3/gen/RestartType.h"
#include "opendnp3/logging/LogLevels.h"
#include "opendnp3/master/FileOperationResult.h"
#include "opendnp3/master/HeaderTypes.h"
#include "opendnp3/master/ICommandProcessor.h"
#include "opendnp3/master/IMasterScan.h"
#include "opendnp3/master/ISOEHandler.h"
#include "opendnp3/master/RestartOperationResult.h"
#include "opendnp3/master/TaskConfig.h"
#include "opendnp3/util/TimeDuration.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace opendnp3
{

/**
 * All the operations that the user can perform on a running master
 */
class IMasterOperations : public ICommandProcessor
{
public:
    virtual ~IMasterOperations() {}

    /**
     *  @param filters Adjust the filters to this value
     */
    virtual void SetLogFilters(const opendnp3::LogLevels& filters) = 0;

    /**
     * Add a recurring user-defined scan from a vector of headers
     * @ return A proxy class used to manipulate the scan
     */
    virtual std::shared_ptr<IMasterScan> AddScan(TimeDuration period,
                                                 const std::vector<Header>& headers,
                                                 std::shared_ptr<ISOEHandler> soe_handler,
                                                 const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Add a scan that requests all objects using qualifier code 0x06
     * @ return A proxy class used to manipulate the scan
     */
    virtual std::shared_ptr<IMasterScan> AddAllObjectsScan(GroupVariationID gvId,
                                                           TimeDuration period,
                                                           std::shared_ptr<ISOEHandler> soe_handler,
                                                           const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Add a class-based scan to the master
     * @return A proxy class used to manipulate the scan
     */
    virtual std::shared_ptr<IMasterScan> AddClassScan(const ClassField& field,
                                                      TimeDuration period,
                                                      std::shared_ptr<ISOEHandler> soe_handler,
                                                      const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Add a start/stop (range) scan to the master
     * @return A proxy class used to manipulate the scan
     */
    virtual std::shared_ptr<IMasterScan> AddRangeScan(GroupVariationID gvId,
                                                      uint16_t start,
                                                      uint16_t stop,
                                                      TimeDuration period,
                                                      std::shared_ptr<ISOEHandler> soe_handler,
                                                      const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Initiate a single user defined scan via a vector of headers
     */
    virtual void Scan(const std::vector<Header>& headers,
                      std::shared_ptr<ISOEHandler> soe_handler,
                      const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Initiate a single scan that requests all objects (0x06 qualifier code) for a certain group and variation
     */
    virtual void ScanAllObjects(GroupVariationID gvId,
                                std::shared_ptr<ISOEHandler> soe_handler,
                                const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Initiate a single class-based scan
     */
    virtual void ScanClasses(const ClassField& field,
                             std::shared_ptr<ISOEHandler> soe_handler,
                             const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Initiate a single start/stop (range) scan
     */
    virtual void ScanRange(GroupVariationID gvId,
                           uint16_t start,
                           uint16_t stop,
                           std::shared_ptr<ISOEHandler> soe_handler,
                           const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Write a time and interval object to a specific index
     */
    virtual void Write(const TimeAndInterval& value, uint16_t index, const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Perform a cold or warm restart and get back the time-to-complete value
     */
    virtual void Restart(RestartType op,
                         const RestartOperationCallbackT& callback,
                         TaskConfig config = TaskConfig::Default())
        = 0;

    /**
     * Perform any operation that requires just a function code
     */
    virtual void PerformFunction(const std::string& name,
                                 FunctionCode func,
                                 const std::vector<Header>& headers,
                                 const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Issue a freeze request to the outstation. Freezes all points of the specified
     * object group, or a range of points if headers are provided.
     *
     * @param type The type of freeze operation
     * @param headers Object headers specifying which points to freeze (empty = all counters)
     * @param config Optional task configuration
     */
    virtual void Freeze(FreezeType type,
                        const std::vector<Header>& headers,
                        const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Write analog input dead-band values to the outstation.
     * Sends a WRITE request with Group34 objects.
     *
     * @param deadBands Vector of (index, deadBandValue) pairs
     * @param callback Called with the result
     * @param config Optional task configuration
     */
    virtual void WriteDeadBands(const std::vector<Indexed<AnalogInputDeadband>>& deadBands,
                                const FileOperationCallbackT& callback,
                                const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Read a file from the outstation using the DNP3 file transfer protocol.
     * Sends OPEN_FILE, reads all blocks, then CLOSE_FILE.
     *
     * @param filename Remote file path
     * @param callback Called with the result (data or error)
     * @param config Optional task configuration
     */
    virtual void ReadFile(const std::string& filename,
                          const FileReadCallbackT& callback,
                          const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Get file information (metadata) from the outstation.
     * Sends GET_FILE_INFO with Group70Var8 (filename), expects Group70Var7 response.
     *
     * @param filename Remote file path
     * @param callback Called with the file info result
     * @param config Optional task configuration
     */
    virtual void GetFileInfo(const std::string& filename,
                             const FileInfoCallbackT& callback,
                             const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Delete a file on the outstation.
     * Sends DELETE_FILE with Group70Var8 (filename).
     *
     * @param filename Remote file path
     * @param callback Called with the operation result
     * @param config Optional task configuration
     */
    virtual void DeleteFile(const std::string& filename,
                            const FileOperationCallbackT& callback,
                            const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Write a file to the outstation using the DNP3 file transfer protocol.
     * Sends OPEN_FILE (mode=WRITE), writes all blocks, then CLOSE_FILE.
     *
     * @param filename Remote file path
     * @param data File content to write
     * @param permissions UNIX-style file permissions
     * @param callback Called with the result
     * @param config Optional task configuration
     */
    virtual void WriteFile(const std::string& filename,
                           const std::vector<uint8_t>& data,
                           FilePermissions permissions,
                           const FileWriteCallbackT& callback,
                           const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Read a directory listing from the outstation.
     * Opens the directory path, reads the raw listing data, parses Group70Var7
     * entries, then closes.
     *
     * @param directoryPath Remote directory path
     * @param callback Called with the directory listing result
     * @param config Optional task configuration
     */
    virtual void ReadDirectory(const std::string& directoryPath,
                               const DirectoryReadCallbackT& callback,
                               const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Abort an in-progress file transfer.
     * Sends ABORT_FILE (FC=0x1E) with Group70Var4 containing the file handle.
     *
     * @param fileHandle The file handle from a previous open operation
     * @param callback Called with the operation result
     * @param config Optional task configuration
     */
    virtual void AbortFile(uint32_t fileHandle,
                           const FileOperationCallbackT& callback,
                           const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Authenticate for file transfer operations.
     * Sends AUTHENTICATE_FILE (FC=0x1D) with Group70Var2 containing credentials.
     *
     * @param username User name for authentication
     * @param password Password for authentication
     * @param callback Called with the authentication result (includes auth key)
     * @param config Optional task configuration
     */
    virtual void AuthenticateFile(const std::string& username,
                                  const std::string& password,
                                  const FileAuthCallbackT& callback,
                                  const TaskConfig& config = TaskConfig::Default())
        = 0;

    /**
     * Manually trigger a link-layer REQUEST_LINK_STATUS to the outstation.
     * The callback is invoked with true on success (LINK_STATUS received)
     * or false on failure (timeout, unexpected response, or offline).
     *
     * @param callback Called with the result (true = success, false = failure)
     */
    virtual void CheckLinkStatus(const std::function<void(bool)>& callback) = 0;
};

} // namespace opendnp3

#endif
