/*
 * RAII wrapper around the FreyrSCADA DNP3 C API for use in interop tests.
 *
 * The FreyrSCADA DNP3 library is proprietary software owned by
 * FreyrSCADA Embedded Solution Pvt Ltd.  This wrapper is used solely
 * for interoperability testing and is NOT a redistribution of their code.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OPENDNP3_INTEROP_FREYR_WRAPPER_H
#define OPENDNP3_INTEROP_FREYR_WRAPPER_H

extern "C"
{
#include <dnp3api.h>
}

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// FreyrServer: wraps a FreyrSCADA DNP3 outstation (server)
// ---------------------------------------------------------------------------
class FreyrServer
{
public:
    struct Config
    {
        uint16_t tcpPort = 20000;
        uint16_t slaveAddress = 1;
        uint16_t masterAddress = 2;
        uint16_t numBinaryInputs = 10;
        uint16_t numAnalogInputs = 10;
        uint16_t numBinaryOutputs = 10;
        uint16_t numAnalogOutputs = 10;
    };

    explicit FreyrServer(const Config& cfg) : config_(cfg)
    {
        // Create
        Integer16 ec = EC_NONE;
        tErrorValue ev = EV_NONE;

        struct sDNP3Parameters params;
        std::memset(&params, 0, sizeof(params));
        params.eAppFlag = APP_SERVER;
        params.ptSelectCallback = nullptr;
        params.ptOperateCallback = &FreyrServer::StaticOperateCb;
        params.ptWriteCallback = nullptr;
        params.ptReadCallback = nullptr;
        params.ptUpdateCallback = nullptr;
        params.ptDebugCallback = nullptr;
        params.ptColdRestartCallback = nullptr;
        params.ptWarmRestartCallback = nullptr;
        params.ptClientPollStatusCallback = nullptr;
        params.u16ObjectId = 1;
        params.u32Options = 0;

        obj_ = DNP3Create(&params, &ec, &ev);
        if (obj_ == nullptr)
        {
            throw std::runtime_error("FreyrServer: DNP3Create failed ec=" + std::to_string(ec));
        }

        // Load configuration
        struct sDNP3ConfigurationParameters dnp3Cfg;
        std::memset(&dnp3Cfg, 0, sizeof(dnp3Cfg));

        auto& srv = dnp3Cfg.sDNP3ServerSet;

        // TCP settings
        srv.sServerCommunicationSet.eCommMode = TCP_IP_MODE;
        std::strncpy((char*)srv.sServerCommunicationSet.sEthernetCommsSet.sEthernetportSet.ai8FromIPAddress, "0.0.0.0",
                     MAX_IPV4_ADDRSIZE - 1);
        srv.sServerCommunicationSet.sEthernetCommsSet.sEthernetportSet.u16PortNumber = cfg.tcpPort;

        // Protocol settings
        srv.sServerProtSet.u16SlaveAddress = cfg.slaveAddress;
        srv.sServerProtSet.u16MasterAddress = cfg.masterAddress;
        srv.sServerProtSet.u32LinkLayerTimeout = 10000;
        srv.sServerProtSet.u32ApplicationLayerTimeout = 20000;
        srv.sServerProtSet.u32TimeSyncIntervalSeconds = 90;

        // Static variations
        srv.sServerProtSet.sStaticVariation.eDeStVarBI = BI_WITH_FLAGS;
        srv.sServerProtSet.sStaticVariation.eDeStVarDBI = DBBI_WITH_FLAGS;
        srv.sServerProtSet.sStaticVariation.eDeStVarBO = BO_WITH_FLAGS;
        srv.sServerProtSet.sStaticVariation.eDeStVarCI = CI_32BIT_WITHFLAG;
        srv.sServerProtSet.sStaticVariation.eDeStVarFzCI = FCI_32BIT_WITHFLAGANDTIME;
        srv.sServerProtSet.sStaticVariation.eDeStVarAI = AI_SINGLEPREC_FLOATWITHFLAG;
        srv.sServerProtSet.sStaticVariation.eDeStVarFzAI = FAI_SINGLEPRECFLOATWITHFLAG;
        srv.sServerProtSet.sStaticVariation.eDeStVarAID = DAI_SINGLEPRECFLOAT;
        srv.sServerProtSet.sStaticVariation.eDeStVarAO = AO_SINGLEPRECFLOAT_WITHFLAG;

        // Event variations
        srv.sServerProtSet.sEventVariation.eDeEvVarBI = BIE_WITHOUT_TIME;
        srv.sServerProtSet.sEventVariation.eDeEvVarDBI = DBBIE_WITHOUT_TIME;
        srv.sServerProtSet.sEventVariation.eDeEvVarCI = CIE_32BIT_WITHFLAG;
        srv.sServerProtSet.sEventVariation.eDeEvVarAI = AIE_SINGLEPREC_WITHOUTTIME;
        srv.sServerProtSet.sEventVariation.eDeEvVarFzCI = FCIE_32BIT_WITHFLAG;
        srv.sServerProtSet.sEventVariation.eDeEvVarFzAI = FAIE_SINGLEPREC_WITHOUTTIME;
        srv.sServerProtSet.sEventVariation.eDeEvVarBO = BOE_WITHOUT_TIME;
        srv.sServerProtSet.sEventVariation.eDeEvVarAO = AOE_SINGLEPREC_WITHOUTTIME;

        // Event buffers
        srv.sServerProtSet.u16Class1EventBufferSize = 50;
        srv.sServerProtSet.u8Class1EventBufferOverFlowPercentage = 90;
        srv.sServerProtSet.u16Class2EventBufferSize = 50;
        srv.sServerProtSet.u8Class2EventBufferOverFlowPercentage = 90;
        srv.sServerProtSet.u16Class3EventBufferSize = 50;
        srv.sServerProtSet.u8Class3EventBufferOverFlowPercentage = 90;

        // Timestamp
        FillCurrentTimestamp(&srv.sServerProtSet.sTimeStamp);

        // Class 0 includes
        srv.sServerProtSet.bAddBIinClass0 = TRUE;
        srv.sServerProtSet.bAddDBIinClass0 = FALSE;
        srv.sServerProtSet.bAddBOinClass0 = TRUE;
        srv.sServerProtSet.bAddCIinClass0 = FALSE;
        srv.sServerProtSet.bAddFzCIinClass0 = FALSE;
        srv.sServerProtSet.bAddAIinClass0 = TRUE;
        srv.sServerProtSet.bAddFzAIinClass0 = FALSE;
        srv.sServerProtSet.bAddAIDinClass0 = FALSE;
        srv.sServerProtSet.bAddAOinClass0 = TRUE;
        srv.sServerProtSet.bAddOSinClass0 = FALSE;

        // Event includes
        srv.sServerProtSet.bAddBIEvent = TRUE;
        srv.sServerProtSet.bAddDBIEvent = FALSE;
        srv.sServerProtSet.bAddBOEvent = TRUE;
        srv.sServerProtSet.bAddCIEvent = FALSE;
        srv.sServerProtSet.bAddFzCIEvent = FALSE;
        srv.sServerProtSet.bAddAIEvent = TRUE;
        srv.sServerProtSet.bAddFzAIEvent = FALSE;
        srv.sServerProtSet.bAddAIDEvent = FALSE;
        srv.sServerProtSet.bAddAOEvent = TRUE;
        srv.sServerProtSet.bAddOSEvent = FALSE;
        srv.sServerProtSet.bAddVTOEvent = FALSE;

        srv.sServerProtSet.eAIDeadbandMethod = DEADBAND_NONE;
        srv.sServerProtSet.bFrozenAnalogInputSupport = FALSE;
        srv.sServerProtSet.bEnableSelfAddressSupport = FALSE;
        srv.sServerProtSet.bEnableFileTransferSupport = FALSE;
        srv.sServerProtSet.u8IntialdatabaseQualityFlag = ONLINE;
        srv.sServerProtSet.bLocalMode = FALSE;
        srv.sServerProtSet.bUpdateCheckTimestamp = FALSE;

        // Unsolicited disabled for simplicity, but must still set valid params
        srv.sServerProtSet.sUnsolicitedResponseSet.bEnableUnsolicited = FALSE;
        srv.sServerProtSet.sUnsolicitedResponseSet.bEnableResponsesonStartup = FALSE;
        srv.sServerProtSet.sUnsolicitedResponseSet.u32Timeout = 5000;
        srv.sServerProtSet.sUnsolicitedResponseSet.u8Retries = 3;
        srv.sServerProtSet.sUnsolicitedResponseSet.u16MaxNumberofEvents = 10;
        srv.sServerProtSet.sUnsolicitedResponseSet.u16Class1TriggerNumberofEvents = 1;
        srv.sServerProtSet.sUnsolicitedResponseSet.u16Class1HoldTimeAfterResponse = 1;
        srv.sServerProtSet.sUnsolicitedResponseSet.u16Class2TriggerNumberofEvents = 1;
        srv.sServerProtSet.sUnsolicitedResponseSet.u16Class2HoldTimeAfterResponse = 1;
        srv.sServerProtSet.sUnsolicitedResponseSet.u16Class3TriggerNumberofEvents = 1;
        srv.sServerProtSet.sUnsolicitedResponseSet.u16Class3HoldTimeAfterResponse = 1;

        // Debug off
        srv.sDebug.u32DebugOptions = 0;

        // Objects - count how many groups we need
        uint16_t numObjects = 0;
        if (cfg.numBinaryInputs > 0)
            numObjects++;
        if (cfg.numAnalogInputs > 0)
            numObjects++;
        if (cfg.numBinaryOutputs > 0)
            numObjects++;
        if (cfg.numAnalogOutputs > 0)
            numObjects++;

        srv.u16NoofObject = numObjects;
        srv.psDNP3Objects = (struct sDNP3Object*)std::calloc(numObjects, sizeof(struct sDNP3Object));

        uint16_t idx = 0;
        if (cfg.numBinaryInputs > 0)
        {
            std::strncpy((char*)srv.psDNP3Objects[idx].ai8Name, "binary input", APP_OBJNAMESIZE - 1);
            srv.psDNP3Objects[idx].eGroupID = BINARY_INPUT;
            srv.psDNP3Objects[idx].u16NoofPoints = cfg.numBinaryInputs;
            srv.psDNP3Objects[idx].eClassID = CLASS_ONE;
            srv.psDNP3Objects[idx].eControlModel = INPUT_STATUS_ONLY;
            srv.psDNP3Objects[idx].u32SBOTimeOut = 0;
            srv.psDNP3Objects[idx].f32AnalogInputDeadband = 0;
            srv.psDNP3Objects[idx].eAnalogStoreType = AS_FLOAT;
            idx++;
        }
        if (cfg.numAnalogInputs > 0)
        {
            std::strncpy((char*)srv.psDNP3Objects[idx].ai8Name, "analog input", APP_OBJNAMESIZE - 1);
            srv.psDNP3Objects[idx].eGroupID = ANALOG_INPUT;
            srv.psDNP3Objects[idx].u16NoofPoints = cfg.numAnalogInputs;
            srv.psDNP3Objects[idx].eClassID = CLASS_ONE;
            srv.psDNP3Objects[idx].eControlModel = INPUT_STATUS_ONLY;
            srv.psDNP3Objects[idx].u32SBOTimeOut = 0;
            srv.psDNP3Objects[idx].f32AnalogInputDeadband = 0;
            srv.psDNP3Objects[idx].eAnalogStoreType = AS_FLOAT;
            idx++;
        }
        if (cfg.numBinaryOutputs > 0)
        {
            std::strncpy((char*)srv.psDNP3Objects[idx].ai8Name, "binary output", APP_OBJNAMESIZE - 1);
            srv.psDNP3Objects[idx].eGroupID = BINARY_OUTPUT;
            srv.psDNP3Objects[idx].u16NoofPoints = cfg.numBinaryOutputs;
            srv.psDNP3Objects[idx].eClassID = NO_CLASS;
            srv.psDNP3Objects[idx].eControlModel = DIRECT_OPERATION;
            srv.psDNP3Objects[idx].u32SBOTimeOut = 0;
            srv.psDNP3Objects[idx].f32AnalogInputDeadband = 0;
            srv.psDNP3Objects[idx].eAnalogStoreType = AS_FLOAT;
            idx++;
        }
        if (cfg.numAnalogOutputs > 0)
        {
            std::strncpy((char*)srv.psDNP3Objects[idx].ai8Name, "analog output", APP_OBJNAMESIZE - 1);
            srv.psDNP3Objects[idx].eGroupID = ANALOG_OUTPUTS;
            srv.psDNP3Objects[idx].u16NoofPoints = cfg.numAnalogOutputs;
            srv.psDNP3Objects[idx].eClassID = CLASS_ONE;
            srv.psDNP3Objects[idx].eControlModel = DIRECT_OPERATION;
            srv.psDNP3Objects[idx].u32SBOTimeOut = 0;
            srv.psDNP3Objects[idx].f32AnalogInputDeadband = 0;
            srv.psDNP3Objects[idx].eAnalogStoreType = AS_FLOAT;
            idx++;
        }

        ec = DNP3LoadConfiguration(obj_, &dnp3Cfg, &ev);
        std::free(srv.psDNP3Objects);
        if (ec != EC_NONE)
        {
            DNP3Free(obj_, &ev);
            throw std::runtime_error("FreyrServer: DNP3LoadConfiguration failed ec=" + std::to_string(ec)
                                     + " ev=" + std::to_string(ev));
        }
    }

    void Start()
    {
        tErrorValue ev = EV_NONE;
        Integer16 ec = DNP3Start(obj_, &ev);
        if (ec != EC_NONE)
        {
            throw std::runtime_error("FreyrServer: DNP3Start failed ec=" + std::to_string(ec));
        }
    }

    void Stop()
    {
        if (obj_)
        {
            tErrorValue ev = EV_NONE;
            DNP3Stop(obj_, &ev);
        }
    }

    ~FreyrServer()
    {
        if (obj_)
        {
            tErrorValue ev = EV_NONE;
            DNP3Stop(obj_, &ev);
            DNP3Free(obj_, &ev);
            obj_ = nullptr;
        }
    }

    // Update a binary input point
    void UpdateBinaryInput(uint16_t index, bool value)
    {
        struct sDNP3DataAttributeID daid;
        struct sDNP3DataAttributeData newVal;
        std::memset(&daid, 0, sizeof(daid));
        std::memset(&newVal, 0, sizeof(newVal));

        daid.u16SlaveAddress = config_.slaveAddress;
        daid.eGroupID = BINARY_INPUT;
        daid.u16IndexNumber = index;

        Unsigned8 data = value ? 1 : 0;
        newVal.eDataSize = SINGLE_POINT_SIZE;
        newVal.eDataType = SINGLE_POINT_DATA;
        newVal.tQuality = ONLINE;
        newVal.pvData = &data;
        FillCurrentTimestamp(&newVal.sTimeStamp);

        tErrorValue ev = EV_NONE;
        Integer16 ec = DNP3Update(obj_, &daid, &newVal, 1, UPDATE_DEFAULT_EVENT, &ev);
        if (ec != EC_NONE)
        {
            std::fprintf(stderr, "FreyrServer::UpdateBinaryInput failed ec=%d ev=%d\n", ec, ev);
        }
    }

    // Update an analog input point
    void UpdateAnalogInput(uint16_t index, float value)
    {
        struct sDNP3DataAttributeID daid;
        struct sDNP3DataAttributeData newVal;
        std::memset(&daid, 0, sizeof(daid));
        std::memset(&newVal, 0, sizeof(newVal));

        daid.u16SlaveAddress = config_.slaveAddress;
        daid.eGroupID = ANALOG_INPUT;
        daid.u16IndexNumber = index;

        Float32 data = value;
        newVal.eDataSize = FLOAT32_SIZE;
        newVal.eDataType = FLOAT32_DATA;
        newVal.tQuality = ONLINE;
        newVal.pvData = &data;
        FillCurrentTimestamp(&newVal.sTimeStamp);

        tErrorValue ev = EV_NONE;
        Integer16 ec = DNP3Update(obj_, &daid, &newVal, 1, UPDATE_DEFAULT_EVENT, &ev);
        if (ec != EC_NONE)
        {
            std::fprintf(stderr, "FreyrServer::UpdateAnalogInput failed ec=%d ev=%d\n", ec, ev);
        }
    }

    // Read back a binary output value from the server database
    bool ReadBinaryOutput(uint16_t index)
    {
        struct sDNP3DataAttributeID daid;
        struct sDNP3DataAttributeData retVal;
        std::memset(&daid, 0, sizeof(daid));
        std::memset(&retVal, 0, sizeof(retVal));

        daid.u16SlaveAddress = config_.slaveAddress;
        daid.eGroupID = BINARY_OUTPUT;
        daid.u16IndexNumber = index;

        tErrorValue ev = EV_NONE;
        Integer16 ec = DNP3Read(obj_, &daid, &retVal, &ev);
        if (ec != EC_NONE)
        {
            return false;
        }
        if (retVal.pvData)
        {
            Unsigned8 val = 0;
            std::memcpy(&val, retVal.pvData, sizeof(Unsigned8));
            return val != 0;
        }
        return false;
    }

    // Read back an analog output value from the server database
    float ReadAnalogOutput(uint16_t index)
    {
        struct sDNP3DataAttributeID daid;
        struct sDNP3DataAttributeData retVal;
        std::memset(&daid, 0, sizeof(daid));
        std::memset(&retVal, 0, sizeof(retVal));

        daid.u16SlaveAddress = config_.slaveAddress;
        daid.eGroupID = ANALOG_OUTPUTS;
        daid.u16IndexNumber = index;

        tErrorValue ev = EV_NONE;
        Integer16 ec = DNP3Read(obj_, &daid, &retVal, &ev);
        if (ec != EC_NONE)
        {
            return 0.0f;
        }
        if (retVal.pvData)
        {
            Float32 val = 0.0f;
            std::memcpy(&val, retVal.pvData, sizeof(Float32));
            return val;
        }
        return 0.0f;
    }

    DNP3Object GetHandle()
    {
        return obj_;
    }

    // Public so tests can inspect
    static std::atomic<int> operateCallCount;

private:
    DNP3Object obj_ = nullptr;
    Config config_;

    static void FillCurrentTimestamp(struct sTargetTimeStamp* ts)
    {
        time_t now;
        time(&now);
        struct tm* ti = localtime(&now);
        ts->u8Day = (Unsigned8)ti->tm_mday;
        ts->u8Month = (Unsigned8)(ti->tm_mon + 1);
        ts->u16Year = (Unsigned16)(ti->tm_year + 1900);
        ts->u8Hour = (Unsigned8)ti->tm_hour;
        ts->u8Minute = (Unsigned8)ti->tm_min;
        ts->u8Seconds = (Unsigned8)ti->tm_sec;
        ts->u16MilliSeconds = 0;
        ts->u16MicroSeconds = 0;
        ts->i8DSTTime = 0;
        ts->u8DayoftheWeek = 0;
    }

    static Integer16 StaticOperateCb(Unsigned16 u16ObjectId,
                                     struct sDNP3DataAttributeID* psOperateID,
                                     struct sDNP3DataAttributeData* psOperateValue,
                                     struct sDNP3CommandParameters* psOperateParams,
                                     tErrorValue* ptErrorValue)
    {
        operateCallCount.fetch_add(1, std::memory_order_relaxed);
        *ptErrorValue = EV_NONE;
        return EC_NONE;
    }
};

// Static member definition (in header since these are test-only files)
inline std::atomic<int> FreyrServer::operateCallCount{0};

// ---------------------------------------------------------------------------
// FreyrClient: wraps a FreyrSCADA DNP3 master (client)
// ---------------------------------------------------------------------------
class FreyrClient
{
public:
    struct Config
    {
        uint16_t tcpPort = 20000;
        std::string serverIP = "127.0.0.1";
        uint16_t masterAddress = 2;
        uint16_t slaveAddress = 1;
        uint16_t numBinaryInputs = 10;
        uint16_t numAnalogInputs = 10;
        uint16_t numBinaryOutputs = 10;
        uint16_t numAnalogOutputs = 10;
    };

    // Track updates via callback
    struct UpdateRecord
    {
        uint16_t groupId;
        uint16_t index;
        // Cached value from callback
        float floatVal = 0.0f;
        bool boolVal = false;
        uint8_t dataType = 0;
    };

    std::mutex updateMutex;
    std::vector<UpdateRecord> updates;
    std::atomic<int> updateCount{0};

    // Cached point data from update callbacks (group << 16 | index -> record)
    std::mutex dataMutex;
    std::map<uint32_t, UpdateRecord> dataCache;

    explicit FreyrClient(const Config& cfg) : config_(cfg)
    {
        Integer16 ec = EC_NONE;
        tErrorValue ev = EV_NONE;

        struct sDNP3Parameters params;
        std::memset(&params, 0, sizeof(params));
        params.eAppFlag = APP_CLIENT;
        params.ptReadCallback = nullptr;
        params.ptWriteCallback = nullptr;
        params.ptUpdateCallback = &FreyrClient::StaticUpdateCb;
        params.ptSelectCallback = nullptr;
        params.ptOperateCallback = nullptr;
        params.ptDebugCallback = nullptr;
        params.ptColdRestartCallback = nullptr;
        params.ptWarmRestartCallback = nullptr;
        params.ptUpdateIINCallback = nullptr;
        params.ptClientStatusCallback = nullptr;
        params.ptDeviceAttrCallback = nullptr;
        params.ptClientPollStatusCallback = nullptr;
        params.u16ObjectId = 1;
        params.u32Options = 0;

        // Store pointer for callback routing
        activeInstance_ = this;

        obj_ = DNP3Create(&params, &ec, &ev);
        if (obj_ == nullptr)
        {
            throw std::runtime_error("FreyrClient: DNP3Create failed ec=" + std::to_string(ec));
        }

        struct sDNP3ConfigurationParameters dnp3Cfg;
        std::memset(&dnp3Cfg, 0, sizeof(dnp3Cfg));

        auto& clt = dnp3Cfg.sDNP3ClientSet;

        FillCurrentTimestamp(&clt.sTimeStamp);
        clt.sDebug.u32DebugOptions = 0;
        clt.benabaleUTCtime = FALSE;
        clt.bUpdateCallbackCheckTimestamp = FALSE;

        clt.u16NoofClient = 1;
        clt.psClientObjects = (struct sClientObject*)std::calloc(1, sizeof(struct sClientObject));

        auto& co = clt.psClientObjects[0];
        co.eCommMode = TCP_IP_MODE;
        co.sClientCommunicationSet.sEthernetCommsSet.u16PortNumber = cfg.tcpPort;
        std::strncpy((char*)co.sClientCommunicationSet.sEthernetCommsSet.ai8ToIPAddress, cfg.serverIP.c_str(),
                     MAX_IPV4_ADDRSIZE - 1);

        co.sClientProtSet.u16MasterAddress = cfg.masterAddress;
        co.sClientProtSet.u16SlaveAddress = cfg.slaveAddress;
        co.sClientProtSet.u32LinkLayerTimeout = 10000;
        co.sClientProtSet.u32ApplicationTimeout = 20000;
        co.sClientProtSet.u32Class0123pollInterval = 5000;
        co.sClientProtSet.u32Class123pollInterval = 3000;
        co.sClientProtSet.u32Class0pollInterval = 0;
        co.sClientProtSet.u32Class1pollInterval = 0;
        co.sClientProtSet.u32Class2pollInterval = 0;
        co.sClientProtSet.u32Class3pollInterval = 0;
        co.sClientProtSet.bFrozenAnalogInputSupport = FALSE;
        co.sClientProtSet.bEnableFileTransferSupport = FALSE;
        co.sClientProtSet.bDisableUnsolicitedStatup = FALSE;
        co.sClientProtSet.bDisableResetofRemotelink = FALSE;

        co.u32CommandTimeout = 50000;
        co.u32FileOperationTimeout = 200000;

        // Objects
        uint16_t numObjs = 0;
        if (cfg.numBinaryInputs > 0)
            numObjs++;
        if (cfg.numAnalogInputs > 0)
            numObjs++;
        if (cfg.numBinaryOutputs > 0)
            numObjs++;
        if (cfg.numAnalogOutputs > 0)
            numObjs++;

        co.u16NoofObject = numObjs;
        co.psDNP3Objects = (struct sDNP3clObject*)std::calloc(numObjs, sizeof(struct sDNP3clObject));

        uint16_t idx = 0;
        if (cfg.numBinaryInputs > 0)
        {
            std::strncpy((char*)co.psDNP3Objects[idx].ai8Name, "BI", APP_OBJNAMESIZE - 1);
            co.psDNP3Objects[idx].eGroupID = BINARY_INPUT;
            co.psDNP3Objects[idx].u16StartingIndexAddress = 0;
            co.psDNP3Objects[idx].u16NoofPoints = cfg.numBinaryInputs;
            co.psDNP3Objects[idx].eClassID = CLASS_ONE;
            co.psDNP3Objects[idx].eControlModel = INPUT_STATUS_ONLY;
            idx++;
        }
        if (cfg.numAnalogInputs > 0)
        {
            std::strncpy((char*)co.psDNP3Objects[idx].ai8Name, "AI", APP_OBJNAMESIZE - 1);
            co.psDNP3Objects[idx].eGroupID = ANALOG_INPUT;
            co.psDNP3Objects[idx].u16StartingIndexAddress = 0;
            co.psDNP3Objects[idx].u16NoofPoints = cfg.numAnalogInputs;
            co.psDNP3Objects[idx].eClassID = CLASS_ONE;
            co.psDNP3Objects[idx].eControlModel = INPUT_STATUS_ONLY;
            idx++;
        }
        if (cfg.numBinaryOutputs > 0)
        {
            std::strncpy((char*)co.psDNP3Objects[idx].ai8Name, "BO", APP_OBJNAMESIZE - 1);
            co.psDNP3Objects[idx].eGroupID = BINARY_OUTPUT;
            co.psDNP3Objects[idx].u16StartingIndexAddress = 0;
            co.psDNP3Objects[idx].u16NoofPoints = cfg.numBinaryOutputs;
            co.psDNP3Objects[idx].eClassID = NO_CLASS;
            co.psDNP3Objects[idx].eControlModel = DIRECT_OPERATION;
            idx++;
        }
        if (cfg.numAnalogOutputs > 0)
        {
            std::strncpy((char*)co.psDNP3Objects[idx].ai8Name, "AO", APP_OBJNAMESIZE - 1);
            co.psDNP3Objects[idx].eGroupID = ANALOG_OUTPUTS;
            co.psDNP3Objects[idx].u16StartingIndexAddress = 0;
            co.psDNP3Objects[idx].u16NoofPoints = cfg.numAnalogOutputs;
            co.psDNP3Objects[idx].eClassID = NO_CLASS;
            co.psDNP3Objects[idx].eControlModel = DIRECT_OPERATION;
            idx++;
        }

        ec = DNP3LoadConfiguration(obj_, &dnp3Cfg, &ev);
        std::free(co.psDNP3Objects);
        std::free(clt.psClientObjects);
        if (ec != EC_NONE)
        {
            DNP3Free(obj_, &ev);
            throw std::runtime_error("FreyrClient: DNP3LoadConfiguration failed ec=" + std::to_string(ec)
                                     + " ev=" + std::to_string(ev));
        }
    }

    void Start()
    {
        tErrorValue ev = EV_NONE;
        Integer16 ec = DNP3Start(obj_, &ev);
        if (ec != EC_NONE)
        {
            throw std::runtime_error("FreyrClient: DNP3Start failed ec=" + std::to_string(ec));
        }
    }

    void Stop()
    {
        if (obj_)
        {
            tErrorValue ev = EV_NONE;
            DNP3Stop(obj_, &ev);
        }
    }

    ~FreyrClient()
    {
        if (obj_)
        {
            tErrorValue ev = EV_NONE;
            DNP3Stop(obj_, &ev);
            DNP3Free(obj_, &ev);
            obj_ = nullptr;
        }
        if (activeInstance_ == this)
        {
            activeInstance_ = nullptr;
        }
    }

    // Read a binary input from cached update callback data
    bool ReadBinaryInput(uint16_t index)
    {
        // First try the data cache populated by update callbacks
        uint32_t key = (static_cast<uint32_t>(BINARY_INPUT) << 16) | index;
        {
            std::lock_guard<std::mutex> lk(dataMutex);
            auto it = dataCache.find(key);
            if (it != dataCache.end())
            {
                return it->second.boolVal;
            }
        }

        // Fallback to DNP3Read
        struct sDNP3DataAttributeID daid;
        struct sDNP3DataAttributeData retVal;
        std::memset(&daid, 0, sizeof(daid));
        std::memset(&retVal, 0, sizeof(retVal));

        daid.eCommMode = TCP_IP_MODE;
        daid.u16PortNumber = config_.tcpPort;
        std::strncpy((char*)daid.ai8IPAddress, config_.serverIP.c_str(), MAX_IPV4_ADDRSIZE - 1);
        daid.u16SlaveAddress = config_.slaveAddress;
        daid.eGroupID = BINARY_INPUT;
        daid.u16IndexNumber = index;

        tErrorValue ev = EV_NONE;
        Integer16 ec = DNP3Read(obj_, &daid, &retVal, &ev);
        if (ec != EC_NONE)
        {
            return false;
        }
        if (retVal.pvData)
        {
            Unsigned8 val = 0;
            std::memcpy(&val, retVal.pvData, sizeof(Unsigned8));
            return val != 0;
        }
        return false;
    }

    // Read an analog input from cached update callback data
    float ReadAnalogInput(uint16_t index)
    {
        // First try the data cache populated by update callbacks
        uint32_t key = (static_cast<uint32_t>(ANALOG_INPUT) << 16) | index;
        {
            std::lock_guard<std::mutex> lk(dataMutex);
            auto it = dataCache.find(key);
            if (it != dataCache.end())
            {
                return it->second.floatVal;
            }
        }

        // Fallback to DNP3Read
        struct sDNP3DataAttributeID daid;
        struct sDNP3DataAttributeData retVal;
        std::memset(&daid, 0, sizeof(daid));
        std::memset(&retVal, 0, sizeof(retVal));

        daid.eCommMode = TCP_IP_MODE;
        daid.u16PortNumber = config_.tcpPort;
        std::strncpy((char*)daid.ai8IPAddress, config_.serverIP.c_str(), MAX_IPV4_ADDRSIZE - 1);
        daid.u16SlaveAddress = config_.slaveAddress;
        daid.eGroupID = ANALOG_INPUT;
        daid.u16IndexNumber = index;

        tErrorValue ev = EV_NONE;
        Integer16 ec = DNP3Read(obj_, &daid, &retVal, &ev);
        if (ec != EC_NONE)
        {
            return 0.0f;
        }
        if (retVal.pvData)
        {
            Float32 val = 0.0f;
            std::memcpy(&val, retVal.pvData, sizeof(Float32));
            return val;
        }
        return 0.0f;
    }

    // Issue a direct operate on binary output via FreyrSCADA client API
    bool DirectOperateBinaryOutput(uint16_t index, bool value)
    {
        struct sDNP3DataAttributeID daid;
        struct sDNP3DataAttributeData sVal;
        struct sDNP3CommandParameters sParams;
        std::memset(&daid, 0, sizeof(daid));
        std::memset(&sVal, 0, sizeof(sVal));
        std::memset(&sParams, 0, sizeof(sParams));

        daid.eCommMode = TCP_IP_MODE;
        daid.u16PortNumber = config_.tcpPort;
        std::strncpy((char*)daid.ai8IPAddress, config_.serverIP.c_str(), MAX_IPV4_ADDRSIZE - 1);
        daid.eGroupID = BINARY_OUTPUT;
        daid.u16SlaveAddress = config_.slaveAddress;
        daid.u16IndexNumber = index;

        Unsigned8 data = value ? 1 : 0;
        sVal.eDataSize = SINGLE_POINT_SIZE;
        sVal.eDataType = SINGLE_POINT_DATA;
        sVal.tQuality = ONLINE;
        sVal.pvData = &data;
        FillCurrentTimestamp(&sVal.sTimeStamp);

        sParams.eCommandVariation = CROB_G12V1;
        sParams.eOPType = value ? PULSE_ON : PULSE_OFF;
        sParams.u8Count = 1;
        sParams.u32ONtime = 1000;
        sParams.u32OFFtime = 1000;
        sParams.bCR = FALSE;

        tErrorValue ev = EV_NONE;
        Integer16 ec = DNP3DirectOperate(obj_, &daid, &sVal, &sParams, &ev);
        if (ec != EC_NONE)
        {
            std::fprintf(stderr, "FreyrClient::DirectOperateBinaryOutput failed ec=%d ev=%d\n", ec, ev);
        }
        return ec == EC_NONE;
    }

    // Issue a direct operate on analog output via FreyrSCADA client API
    bool DirectOperateAnalogOutput(uint16_t index, float value)
    {
        struct sDNP3DataAttributeID daid;
        struct sDNP3DataAttributeData sVal;
        struct sDNP3CommandParameters sParams;
        std::memset(&daid, 0, sizeof(daid));
        std::memset(&sVal, 0, sizeof(sVal));
        std::memset(&sParams, 0, sizeof(sParams));

        daid.eCommMode = TCP_IP_MODE;
        daid.u16PortNumber = config_.tcpPort;
        std::strncpy((char*)daid.ai8IPAddress, config_.serverIP.c_str(), MAX_IPV4_ADDRSIZE - 1);
        daid.eGroupID = ANALOG_OUTPUTS;
        daid.u16SlaveAddress = config_.slaveAddress;
        daid.u16IndexNumber = index;

        Float32 data = value;
        sVal.eDataSize = FLOAT32_SIZE;
        sVal.eDataType = FLOAT32_DATA;
        sVal.tQuality = ONLINE;
        sVal.pvData = &data;
        FillCurrentTimestamp(&sVal.sTimeStamp);

        sParams.eCommandVariation = ANALOG_OUTPUT_BLOCK_FLOAT32;
        sParams.eOPType = NUL;
        sParams.u8Count = 1;

        tErrorValue ev = EV_NONE;
        Integer16 ec = DNP3DirectOperate(obj_, &daid, &sVal, &sParams, &ev);
        if (ec != EC_NONE)
        {
            std::fprintf(stderr, "FreyrClient::DirectOperateAnalogOutput failed ec=%d ev=%d\n", ec, ev);
        }
        return ec == EC_NONE;
    }

    // Wait for the update callback to be called at least `count` times
    bool WaitForUpdates(int count, std::chrono::seconds timeout)
    {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline)
        {
            if (updateCount.load(std::memory_order_relaxed) >= count)
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        return updateCount.load(std::memory_order_relaxed) >= count;
    }

    DNP3Object GetHandle()
    {
        return obj_;
    }

private:
    DNP3Object obj_ = nullptr;
    Config config_;

    static FreyrClient* activeInstance_;

    static void FillCurrentTimestamp(struct sTargetTimeStamp* ts)
    {
        time_t now;
        time(&now);
        struct tm* ti = localtime(&now);
        ts->u8Day = (Unsigned8)ti->tm_mday;
        ts->u8Month = (Unsigned8)(ti->tm_mon + 1);
        ts->u16Year = (Unsigned16)(ti->tm_year + 1900);
        ts->u8Hour = (Unsigned8)ti->tm_hour;
        ts->u8Minute = (Unsigned8)ti->tm_min;
        ts->u8Seconds = (Unsigned8)ti->tm_sec;
        ts->u16MilliSeconds = 0;
        ts->u16MicroSeconds = 0;
        ts->i8DSTTime = 0;
        ts->u8DayoftheWeek = 0;
    }

    static Integer16 StaticUpdateCb(Unsigned16 u16ObjectId,
                                    struct sDNP3DataAttributeID* ptUpdateID,
                                    struct sDNP3DataAttributeData* ptUpdateValue,
                                    struct sDNP3UpdateParameters* ptUpdateParams,
                                    tErrorValue* ptErrorValue)
    {
        if (activeInstance_)
        {
            UpdateRecord rec;
            rec.groupId = static_cast<uint16_t>(ptUpdateID->eGroupID);
            rec.index = ptUpdateID->u16IndexNumber;

            // Extract value from callback data
            if (ptUpdateValue && ptUpdateValue->pvData)
            {
                switch (ptUpdateValue->eDataType)
                {
                case SINGLE_POINT_DATA: {
                    Unsigned8 val = 0;
                    std::memcpy(&val, ptUpdateValue->pvData, sizeof(Unsigned8));
                    rec.boolVal = (val != 0);
                    rec.floatVal = static_cast<float>(val);
                    break;
                }
                case FLOAT32_DATA: {
                    Float32 val = 0.0f;
                    std::memcpy(&val, ptUpdateValue->pvData, sizeof(Float32));
                    rec.floatVal = val;
                    rec.boolVal = (val != 0.0f);
                    break;
                }
                case SIGNED_DWORD_DATA: {
                    int32_t val = 0;
                    std::memcpy(&val, ptUpdateValue->pvData, sizeof(int32_t));
                    rec.floatVal = static_cast<float>(val);
                    rec.boolVal = (val != 0);
                    break;
                }
                case UNSIGNED_DWORD_DATA: {
                    uint32_t val = 0;
                    std::memcpy(&val, ptUpdateValue->pvData, sizeof(uint32_t));
                    rec.floatVal = static_cast<float>(val);
                    rec.boolVal = (val != 0);
                    break;
                }
                case SIGNED_LWORD_DATA: {
                    double val = 0.0;
                    std::memcpy(&val, ptUpdateValue->pvData, sizeof(double));
                    rec.floatVal = static_cast<float>(val);
                    rec.boolVal = (val != 0.0);
                    break;
                }
                case SIGNED_WORD_DATA: {
                    int16_t val = 0;
                    std::memcpy(&val, ptUpdateValue->pvData, sizeof(int16_t));
                    rec.floatVal = static_cast<float>(val);
                    rec.boolVal = (val != 0);
                    break;
                }
                case UNSIGNED_WORD_DATA: {
                    uint16_t val = 0;
                    std::memcpy(&val, ptUpdateValue->pvData, sizeof(uint16_t));
                    rec.floatVal = static_cast<float>(val);
                    rec.boolVal = (val != 0);
                    break;
                }
                default: {
                    // Try to interpret as float32 anyway
                    if (ptUpdateValue->eDataSize >= FLOAT32_SIZE)
                    {
                        Float32 val = 0.0f;
                        std::memcpy(&val, ptUpdateValue->pvData, sizeof(Float32));
                        rec.floatVal = val;
                    }
                    break;
                }
                }
                rec.dataType = static_cast<uint8_t>(ptUpdateValue->eDataType);
            }

            // Cache the data by group|index key
            uint32_t key = (static_cast<uint32_t>(ptUpdateID->eGroupID) << 16) | ptUpdateID->u16IndexNumber;
            {
                std::lock_guard<std::mutex> lk(activeInstance_->dataMutex);
                activeInstance_->dataCache[key] = rec;
            }
            {
                std::lock_guard<std::mutex> lk(activeInstance_->updateMutex);
                activeInstance_->updates.push_back(rec);
            }
            activeInstance_->updateCount.fetch_add(1, std::memory_order_relaxed);
        }
        *ptErrorValue = EV_NONE;
        return EC_NONE;
    }
};

// Static member definition
inline FreyrClient* FreyrClient::activeInstance_ = nullptr;

#endif // OPENDNP3_INTEROP_FREYR_WRAPPER_H
