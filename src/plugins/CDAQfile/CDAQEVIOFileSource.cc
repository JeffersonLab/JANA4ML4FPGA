//
// Created by xme@jlab.org on 2/9/23.
//

#include "CDAQEVIOFileSource.h"

using namespace std;
using namespace std::chrono;


/**
 * Constructor
 */
EVIOFileSource::CDAQEVIOFileSource(std::string resource_name, JApplication *app) : JEventSource(
        resource_name, app) {
    SetTypeName(NAME_OF_THIS); // Provide JANA with class name
}

//----------------
// Destructor
//----------------
CDAQEVIOFileSource::~CDAQEVIOFileSource() {

    // as well as anyone in a wait state
    DONE = true;

    if(VERBOSE>0) evioout << "Closing hdevio event source \"" << GetResourceName() << "\"" <<endl;

    // Delete HDEVIO and print stats
    if(hdevio){
        hdevio->PrintStats();
        delete hdevio;
    }
}

/**
 *  Set plugin configuration parameters.
 *  Taken from ./rawdataparser/JEventSource_EVIOpp.cc/h class JEventSource_EVIOpp constructor
 */
void CDAQEVIOFileSource::SetEVIODefaultConfigParams(JApplication *app) {

    VERBOSE = 0;
    VERBOSE_ET = 0;
    NTHREADS = 2;
    MAX_PARSED_EVENTS = 128;
    MAX_EVENT_RECYCLES = 1000;
    MAX_OBJECT_RECYCLES = 1000;
    LOOP_FOREVER = false;
    USER_RUN_NUMBER = 0;
    ET_STATION_NEVENTS = 10;
    ET_STATION_CREATE_BLOCKING = false;
    PRINT_STATS = true;
    SWAP = true;
    LINK = true;
    LINK_TRIGGERTIME = true;
    LINK_BORCONFIG = true;
    LINK_CONFIG = true;
    PARSE = true;
    PARSE_F250 = true;
    PARSE_F125 = true;
    PARSE_F1TDC = true;
    PARSE_CAEN1290TDC = true;
    PARSE_CONFIG = true;
    PARSE_BOR = true;
    PARSE_EPICS = true;
    PARSE_EVENTTAG = true;
    PARSE_TRIGGER = true;
    PARSE_SSP = true;
    PARSE_GEMSRS = false;
    NSAMPLES_GEMSRS = 9;
    APPLY_TRANSLATION_TABLE = true;
    IGNORE_EMPTY_BOR = false;
    F250_EMULATION_MODE = kEmulationAuto;
    F125_EMULATION_MODE = kEmulationAuto;
    F250_EMULATION_VERSION = 3;
    RECORD_CALL_STACK = false;
    TREAT_TRUNCATED_AS_ERROR = false;
    SYSTEMS_TO_PARSE = "";
    SYSTEMS_TO_PARSE_FORCE = 0;
    BLOCKS_TO_SKIP = 0;

    app->SetDefaultParameter("EVIO:VERBOSE", VERBOSE,
                             "Set verbosity level for processing and debugging statements while parsing. 0=no debugging messages. 10=all messages");
    app->SetDefaultParameter("ET:VERBOSE", VERBOSE_ET,
                             "Set verbosity level for processing and debugging statements while reading from ET. 0=no debugging messages. 10=all messages");
    app->SetDefaultParameter("EVIO:NTHREADS", NTHREADS,
                             "Set the number of worker threads to use for parsing the EVIO data");
    app->SetDefaultParameter("EVIO:MAX_PARSED_EVENTS", MAX_PARSED_EVENTS,
                             "Set maximum number of events to allow in EVIO parsed events queue");
    app->SetDefaultParameter("EVIO:MAX_EVENT_RECYCLES", MAX_EVENT_RECYCLES,
                             "Set maximum number of EVIO (i.e. block of) events  a worker thread should process before pruning excess DParsedEvent objects from its pool");
    app->SetDefaultParameter("EVIO:MAX_OBJECT_RECYCLES", MAX_OBJECT_RECYCLES,
                             "Set maximum number of events a DParsedEvent should be used for before pruning excess hit objects from its pools");
    app->SetDefaultParameter("EVIO:LOOP_FOREVER", LOOP_FOREVER,
                             "If reading from EVIO file, keep re-opening file and re-reading events forever (only useful for debugging) If reading from ET, this is ignored.");
    app->SetDefaultParameter("EVIO:RUN_NUMBER", USER_RUN_NUMBER,
                             "User-supplied run number. Override run number from other sources with this.(will be ignored if set to zero)");
    app->SetDefaultParameter("EVIO:ET_STATION_NEVENTS", ET_STATION_NEVENTS,
                             "Number of events to use if we have to create the ET station. Ignored if station already exists.");
    app->SetDefaultParameter("EVIO:ET_STATION_CREATE_BLOCKING", ET_STATION_CREATE_BLOCKING,
                             "Set this to 0 to create station in non-blocking mode (default is to create it in blocking mode). Ignored if station already exists.");
    app->SetDefaultParameter("EVIO:PRINT_STATS", PRINT_STATS,
                             "Print some additional stats from event source when it's finished processing events");

    app->SetDefaultParameter("EVIO:SWAP", SWAP,
                             "Allow swapping automatic swapping. Turning this off should only be used for debugging.");
    app->SetDefaultParameter("EVIO:LINK", LINK,
                             "Link associated objects. Turning this off should only be used for debugging.");
    app->SetDefaultParameter("EVIO:LINK_TRIGGERTIME", LINK_TRIGGERTIME,
                             "Link D*TriggerTime associated objects. This is on by default and may be OK to turn off (but please check output if you do!)");
    app->SetDefaultParameter("EVIO:LINK_BORCONFIG", LINK_BORCONFIG,
                             "Link BORConfig associated objects. This is on by default. If turned off, it will break emulation (and possibly other things in the future).");
    app->SetDefaultParameter("EVIO:LINK_CONFIG", LINK_CONFIG, "Link Config associated objects. This is on by default.");

    app->SetDefaultParameter("EVIO:PARSE", PARSE,
                             "Set this to 0 to disable parsing of event buffers and generation of any objects (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_F250", PARSE_F250,
                             "Set this to 0 to disable parsing of data from F250 ADC modules (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_F125", PARSE_F125,
                             "Set this to 0 to disable parsing of data from F125 ADC modules (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_F1TDC", PARSE_F1TDC,
                             "Set this to 0 to disable parsing of data from F1TDC modules (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_CAEN1290TDC", PARSE_CAEN1290TDC,
                             "Set this to 0 to disable parsing of data from CAEN 1290 TDC modules (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_CONFIG", PARSE_CONFIG,
                             "Set this to 0 to disable parsing of ROC configuration data in the data stream (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_BOR", PARSE_BOR,
                             "Set this to 0 to disable parsing of BOR events from the data stream (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_EPICS", PARSE_EPICS,
                             "Set this to 0 to disable parsing of EPICS events from the data stream (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_EVENTTAG", PARSE_EVENTTAG,
                             "Set this to 0 to disable parsing of event tag data in the data stream (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_TRIGGER", PARSE_TRIGGER,
                             "Set this to 0 to disable parsing of the built trigger bank from CODA (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_SSP", PARSE_SSP,
                             "Set this to 0 to disable parsing of the SSP (DIRC data) bank from CODA (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:PARSE_GEMSRS", PARSE_GEMSRS,
                             "Set this to 0 to disable parsing of the SRS (GEM data) bank from CODA (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:NSAMPLES_GEMSRS", NSAMPLES_GEMSRS,
                             "Set this to number of readout samples for SRS (GEM data) bank from CODA (for benchmarking/debugging)");
    app->SetDefaultParameter("EVIO:APPLY_TRANSLATION_TABLE", APPLY_TRANSLATION_TABLE,
                             "Apply the translation table to create DigiHits (you almost always want this on)");
    app->SetDefaultParameter("EVIO:IGNORE_EMPTY_BOR", IGNORE_EMPTY_BOR,
                             "Set to non-zero to continue processing data even if an empty BOR event is encountered.");
    app->SetDefaultParameter("EVIO:TREAT_TRUNCATED_AS_ERROR", TREAT_TRUNCATED_AS_ERROR,
                             "Set to non-zero to have a truncated EVIO file the JANA return code to non-zero indicating the program errored.");

    app->SetDefaultParameter("EVIO:F250_EMULATION_MODE", F250_EMULATION_MODE,
                             "Set f250 emulation mode. 0=no emulation, 1=always, 2=auto. Default is 2 (auto).");
    app->SetDefaultParameter("EVIO:F125_EMULATION_MODE", F125_EMULATION_MODE,
                             "Set f125 emulation mode. 0=no emulation, 1=always, 2=auto. Default is 2 (auto).");

    app->SetDefaultParameter("EVIO:SYSTEMS_TO_PARSE", SYSTEMS_TO_PARSE,
                             "Comma separated list of systems to parse EVIO data for. "
                             "Default is empty string which means to parse all. System "
                             "names should be what is returned by DTranslationTable::DetectorName() .");
    app->SetDefaultParameter("EVIO:SYSTEMS_TO_PARSE_FORCE", SYSTEMS_TO_PARSE_FORCE,
                             "How to handle mismatches between hard coded map and one read from CCDB "
                             "when EVIO:SYSTEMS_TO_PARSE is set. 0=Treat as error, 1=Use CCDB, 2=Use hardcoded");

    app->SetDefaultParameter("EVIO:BLOCKS_TO_SKIP", BLOCKS_TO_SKIP,
                             "Number of EVIO blocks to skip parsing at start of file (typically 1 block=40 events)");

    // TODO: JANA1 function, check later.
    // if(gPARMS->Exists("RECORD_CALL_STACK")) gPARMS->GetParameter("RECORD_CALL_STACK", RECORD_CALL_STACK);

}

uint64_t CDAQEVIOFileSource::SearchFileForRunNumber() {
    /// This is called from the constructor when reading
    /// from a file to seed the default run number. This
    /// is needed because the first event is probably a BOR
    /// event which does not contain the run number. The run
    /// number would then be reported as "0" and incorrect
    /// calibrations and field maps would be read in. This
    /// will try searching past the first (BOR) event for
    /// either a physics event, or an EPICS event that has
    /// the run number defined in it.
    ///
    /// The value returned from this will be copied into each
    /// DEVIOWorkerThread object when it is created. It will
    /// be used to initialize the run number for each event
    /// the worker thread processes. If the worker thread
    /// processes a physics event containing a run number,
    /// then that will overwrite the run number in the
    /// DParsedEvent. Finally, if the user specified a run
    /// number, then that will be reported to JANA in lieu of
    /// the one in DParsedEvent.

    string filename = GetResourceName();
    if (VERBOSE > 2) {
        evioout << "     In CDAQEVIOFileSource::SearchFileForRunNumber() filename=";
        evioout << filename << " ..." << endl;
    }

    uint32_t buff_len = 4000000;
    uint32_t *buff = new uint32_t[buff_len];
    HDEVIO *hdevio = new HDEVIO(filename, false, VERBOSE);
    while (hdevio->readNoFileBuff(buff, buff_len)) {

        // Assume first word is number of words in bank
        uint32_t *iptr = buff;
        uint32_t *iend = &iptr[*iptr - 1];
        if (VERBOSE > 2) evioout << "Checking event with header= 0x" << hex << iptr[1] << dec << endl;
        if (*iptr > 2048) iend = &iptr[2048]; // only search the first 2kB of each event
        bool has_timestamps = false;
        while (iptr < iend) {
            iptr++;

            // EPICS event
            if ((*iptr & 0xff000f) == 0x600001) {
                if (VERBOSE > 2) evioout << "     Found EPICS header. Looking for HD:coda:daq:run_number ..." << endl;
                const char *cptr = (const char *) &iptr[1];
                const char *cend = (const char *) iend;
                const char *needle = "HD:coda:daq:run_number=";
                while (cptr < cend) {
                    if (VERBOSE > 4) evioout << "       \"" << cptr << "\"" << endl;
                    if (!strncmp(cptr, needle, strlen(needle))) {
                        if (VERBOSE > 2) evioout << "     Found it!" << endl;
                        uint64_t run_number_seed = atoi(&cptr[strlen(needle)]);
                        if (hdevio) delete hdevio;
                        if (buff) delete[] buff;
                        return run_number_seed;
                    }
                    cptr += 4; // should only start on 4-byte boundary!
                }
            }

            // BOR event from CODA ER
            if ((*iptr & 0xffffffff) == 0x00700E01) continue;

            // BOR event from CDAQ ER
            // Prior to Fall 2019 this was 0x00700e34 where the 34 represented the number of crates
            // Two crates were added in Fall 2019 so this changed to 0x00700e36. To future-proof
            // this, we ignore the last 8 bits and hope this is unique enough not to get confused!
            //if( (*iptr & 0xffffffff) ==  0x00700e34){
            if ((*iptr & 0xffffff00) == 0x00700e00) {
                iptr++;
                uint32_t crate_len = iptr[0];
                uint32_t crate_header = iptr[1];
                uint32_t *iend_crate = &iptr[crate_len];

                // Make sure crate tag is right
                if ((crate_header >> 16) == 0x71) {

                    // Loop over modules
                    iptr += 2;
                    while (iptr < iend_crate) {
                        uint32_t module_header = *iptr++;
                        uint32_t module_len = module_header & 0xFFFF;
                        uint32_t modType = (module_header >> 20) & 0x1f;

                        if (modType == DModuleType::CDAQTSG) {
                            uint64_t run_number_seed = iptr[0];
                            if (hdevio) delete hdevio;
                            if (buff) delete[] buff;
                            return run_number_seed;
                        }
                        iptr = &iptr[module_len];
                    }
                    iptr = iend_crate; // ensure we're pointing past this crate
                }

                continue; // didn't find it in this CDAQ BOR. Keep looking
            }

            // PHYSICS event
            bool not_in_this_buffer = false;
            switch ((*iptr) >> 16) {
                case 0xFF10:
                case 0xFF11:
                case 0xFF20:
                case 0xFF21:
                case 0xFF24:
                case 0xFF25:
                case 0xFF30:
                    not_in_this_buffer = true;
                    break;
                case 0xFF23:
                case 0xFF27:
                    has_timestamps = true;
                case 0xFF22:
                case 0xFF26:
                    break;
                default:
                    continue;
            }

            if (not_in_this_buffer) break; // go to next EVIO buffer

            iptr++;
            if (((*iptr) & 0x00FF0000) != 0x000A0000) {
                iptr--;
                continue;
            }
            uint32_t M = iptr[-3] & 0x000000FF; // Number of events from Physics Event header
            if (VERBOSE > 2)
                evioout << "       ...(epic quest) Trigger bank " << (has_timestamps ? "does" : "doesn't")
                        << " have timestamps. Nevents in block M=" << M << endl;
            iptr++;
            uint64_t *iptr64 = (uint64_t *) iptr;

            uint64_t event_num = *iptr64;
            if (VERBOSE > 3) evioout << "       ....(epic quest) Event num: " << event_num << endl;
            iptr64++;
            if (has_timestamps) iptr64 = &iptr64[M]; // advance past timestamps

            uint64_t run_number_seed = (*iptr64) >> 32;
            if (VERBOSE > 1) evioout << "       .. (epic quest) Found run number: " << run_number_seed << endl;

            if (hdevio) delete hdevio;
            if (buff) delete[] buff;
            return run_number_seed;

        } // while(iptr<iend)

        if (hdevio->Nevents > 500) {
            if (VERBOSE > 2)
                evioout << "       more than 500 events checked and no run number seen! abondoning search" << endl;
            break;
        }

    } // while(hdevio->read(buff, buff_len))

    if (hdevio->err_code != HDEVIO::HDEVIO_OK) evioout << hdevio->err_mess.str() << endl;

    if (hdevio) delete hdevio;
    if (buff) delete[] buff;

    if (VERBOSE > 2) evioout << "     failed to find run number. Returning 0" << endl;

    return 0;
}


/**
 * Open the evio file.
 * Taken from ./rawdataparser/JEventSource_EVIOpp.cc/h class JEventSource_EVIOpp constructor
 */
void CDAQEVIOFileSource::OpenEVIOFile(std::string filename) {
    source_type = kNoSource;
    hdevio = NULL;
    et_quit_next_timeout = false;

    if (VERBOSE > 0) {
        evioout << "Attempting to open \"" << filename << "\" as EVIO file..." << endl;
    }

    hdevio = new HDEVIO(filename, true, VERBOSE);
    if (!hdevio->is_open) {
        cerr << hdevio->err_mess.str() << endl;
        throw JException("Failed to open EVIO file: " + filename, __FILE__, __LINE__);
        // throw exception indicating error
    }
    source_type = kFileSource;
    hdevio->IGNORE_EMPTY_BOR = IGNORE_EMPTY_BOR;

    hdevio->PrintFileSummary();

    if (VERBOSE > 0) evioout << "Success opening event source \"" << filename << "\"!" << endl;
}


void CDAQEVIOFileSource::Open() {

    // Refer to JANA2 tutorial https://github.com/JeffersonLab/JANA2/tree/master/src/examples/Tutorial
    JApplication *app = GetApplication();

    CDAQEVIOFileSource::SetEVIODefaultConfigParams(app);

    CDAQEVIOFileSource::OpenEVIOFile(GetResourceName());
}

std::string CDAQEVIOFileSource::GetDescription() {
    return "CDAQEVIOFileSource - Reads from *.evio file and print file summary.";
}


template<>
double JEventSourceGeneratorT<CDAQEVIOFileSource>::CheckOpenable(std::string resource_name) {

    /// CheckOpenable() decides how confident we are that this EventSource can handle this resource.
    ///    0.0        -> 'Cannot handle'
    ///    (0.0, 1.0] -> 'Can handle, with this confidence level'

    /// To determine confidence level, feel free to open up the file and check for magic bytes or metadata.
    /// Returning a confidence <- {0.0, 1.0} is perfectly OK!

//    if( resource_name.find(".evio") != std::string::npos) return 0.5;

    return (resource_name == "CDAQEVIOFileSource") ? 1.0 : 0.0;

}

