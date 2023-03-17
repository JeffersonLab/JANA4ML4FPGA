#pragma once

#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <TDirectory.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>
#include "SrsRecord.h"
#include "F125FDCPulseRecord.h"
#include "F125WindowRawRecord.h"
#include "rawdataparser/Df125FDCPulse.h"
#include "rawdataparser/DGEMSRSWindowRawData.h"
#include "rawdataparser/Df125WindowRawData.h"

class JEvent;
class JApplication;

class FlatTreeWriterProcessor:
        public JEventProcessor,
        public spdlog::extensions::SpdlogMixin<FlatTreeWriterProcessor>   // this automates proper Log initialization
{
public:
    explicit FlatTreeWriterProcessor(JApplication *);
    ~FlatTreeWriterProcessor() override = default;

    //----------------------------
    // Init
    //
    // This is called once before the first call to the Process method
    // below. You may, for example, want to open an output file here.
    // Only one thread will call this.
    void Init() override;

    //----------------------------
    // Process
    //
    // This is called for every event. Multiple threads may call this
    // simultaneously. If you write something to an output file here
    // then make sure to protect it with a mutex or similar mechanism.
    // Minimize what is done while locked since that directly affects
    // the multi-threaded performance.
    void Process(const std::shared_ptr<const JEvent>& event) override;

    //----------------------------
    // Finish
    //
    // This is called once after all events have been processed. You may,
    // for example, want to close an output file here.
    // Only one thread will call this.
    void Finish() override;

private:

    // TODO there should be a FlatIO for each worker thread
    std::recursive_mutex io_mutex;
    TTree *mEventTree;
    std::vector<std::reference_wrapper<flatio::AlignedArraysIO>> m_ios;

    flatio::SrsRecordIO m_srs_record_io;
    flatio::F125WindowRawRecordIO m_f125_wraw_io;
    flatio::F125FDCPulseRecordIO m_f125_pulse_io;

    std::shared_ptr<JGlobalRootLock> m_glb_root_lock;

    uint16_t findBestSrsSamle(std::vector<uint16_t> samples);

    void SaveF125FDCPulse(std::vector<const Df125FDCPulse *> records);

    void SaveGEMSRSWindowRawData(std::vector<const DGEMSRSWindowRawData *> records);

    void SaveF125WindowRawData(std::vector<const Df125WindowRawData *> records);
};

