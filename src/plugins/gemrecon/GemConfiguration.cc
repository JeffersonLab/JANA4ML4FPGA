#include "GemConfiguration.h"
//ClassImp(GemConfiguration);
using namespace std;

GemConfiguration::GemConfiguration() {
    SetDefaults();
}


GemConfiguration::~GemConfiguration() {
}

bool GemConfiguration::FileExists(const std::string& name) const {
    ifstream f(gSystem->ExpandPathName(name.c_str()));
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }
}

//============================================================================================
GemConfiguration &GemConfiguration::operator=(const GemConfiguration &rhs) {
    fCycleWait = rhs.GetCycleWait();
    fETIPAddress = rhs.GetETIPAddress();
    fETStationName = rhs.GetETStationName();
    fInputFileName = rhs.GetInputFileName();
    fOutputFileName = rhs.GetOutputFileName();
    fInputMapName = rhs.GetInputMapName();
    fPedFileName = rhs.GetPedFileName();
    fRunType = rhs.GetRunType();
    fIsOnlineMode = rhs.GetOnlineMode();
    fNCol = rhs.GetNCol();
    fTCPPort = rhs.GetTCPPort();
    fETMode = rhs.GetETMode();
    fCODAFileDir = rhs.GetCODAFileDir();
    fZeroSupCut = rhs.GetZeroSupCut();
    fComModeCut = rhs.GetComModeCut();
    fStartTimeSamples = rhs.GetStartTimeSamples();
    fStopTimeSamples = rhs.GetStopTimeSamples();
    fNbOfTimeSamples = rhs.GetNbOfTimeSamples();
    fIsHitPeakOrSumADCs = rhs.GetHitPeakOrSumADCs();
    fIsCentralOrAllStripsADCs = rhs.GetCentralOrAllStripsADCs();
    fMaxADCvalue = rhs.GetMaxADCvalue();
    fMinADCvalue = rhs.GetMinADCvalue();
    fMaxClusterSize = rhs.GetMaxClusterSize();
    fMinClusterSize = rhs.GetMinClusterSize();
    fMaxClusterMult = rhs.GetMaxClusterMult();
    fFirstEvent = rhs.GetFirstEvent();
    fLastEvent = rhs.GetLastEvent();
    return *this;
}

//============================================================================================
void GemConfiguration::SetDefaults() {
    fCycleWait = 20; //sec
    fETIPAddress = "129.57.167.225";
    fETStationName = "GEM_monitor";
    fInputFileName = "./db/test_99.dat";
    fOutputFileName = "./root_output/test_99.root";
    fPedFileName = "./db/pedestal.root";
    fInputMapName = "./db/mapping.cfg";
    fCODAFileDir = "/data/prad/";
    fIsOnlineMode = 0;
    fNCol = 9;
    fTCPPort = 11111;
    fETMode = 2;
    fZeroSupCut = 5;
    fComModeCut = 100;
    fRunType = "RAWDATA";
    fStopTimeSamples = 7;
    fStartTimeSamples = 2;
    fNbOfTimeSamples = 9;
    fIsHitPeakOrSumADCs = "sumADCs";
    fIsCentralOrAllStripsADCs = "centralStripADCs";
    fMinADCvalue = 50;
    fMinClusterSize = 2;
    fMaxClusterSize = 50;
    fMaxClusterMult = 5;
    fFirstEvent = 0;
    fLastEvent = 1000;
}

//============================================================================================
void GemConfiguration::Save(const std::string& filename) const {
#ifdef DEBUG
    cout << "saving conf in " << gSystem->ExpandPathName(filename) << endl;
#endif
    ofstream file(gSystem->ExpandPathName(filename.c_str()));
    file << "CYCLEWAIT " << fCycleWait << endl;
    file << "ETIPADDRESS " << fETIPAddress << endl;
    file << "ETSTATIONNAME" << fETStationName << endl;
    file << "INPUTFILENAME " << fInputFileName << endl;
    file << "OUTPUTFILENAME " << fOutputFileName << endl;
    file << "INPUTMAPNAME " << fInputMapName << endl;
    file << "PADFILENAME " << fPedFileName << endl;
    file << "ISONLINEMODE " << fIsOnlineMode << endl;
    file << "NCOL" << fNCol << endl;
    file << "TCPPORT" << fTCPPort << endl;
    file << "ETMODE" << fETMode << endl;
    file << "CODAFILEDIR" << fCODAFileDir << endl;
    file << "RUNTYPE" << fRunType << endl;
    file << "ZEROSUPCUT" << fZeroSupCut << endl;
    file << "COMMODECUT" << fComModeCut << endl;
    file << "NBTIMESAMPLES" << fNbOfTimeSamples << endl;
    file << "STARTTIMESAMPLES" << fStartTimeSamples << endl;
    file << "STOPTIMESAMPLES" << fStopTimeSamples << endl;
    file << "HITPEAKORSUMADCS" << fIsHitPeakOrSumADCs << endl;
    file << "CENTRALORALLSTRIPS" << fIsCentralOrAllStripsADCs << endl;
    file << "MAXADCVALUE" << fMaxADCvalue << endl;
    file << "MINADCVALUE" << fMinADCvalue << endl;
    file << "MINCLUSTSIZE" << fMinClusterSize << endl;
    file << "MAXCLUSTSIZE" << fMaxClusterSize << endl;
    file << "MAXCLUSTMULT" << fMaxClusterMult << endl;
    file << "FIRSTEVENT" << fFirstEvent << endl;
    file << "LASTEVENT" << fLastEvent << endl;
    file.close();
}

//============================================================================================
void GemConfiguration::Load(const std::string& filename) {
    printf("  GemConfiguration::Load() ==> Loading cfg from %s\n", gSystem->ExpandPathName(filename.c_str()));
    //  ifstream file (gSystem->ExpandPathName(filename), ifstream::in);

    ifstream file;
    if(!FileExists(gSystem->ExpandPathName(filename.c_str()))) {
        throw std::runtime_error(std::string(gSystem->ExpandPathName(filename.c_str()) ) + ": File does not exist or cannot be opened!\n");
    }


    try {
        file.open(gSystem->ExpandPathName(filename.c_str()), std::ifstream::in);
    } catch (ifstream::failure e) {
        std::cerr << "Error opening file" << gSystem->ExpandPathName(filename.c_str()) << std::endl ;
        throw;
    } catch (...) {
        std::cerr << "Non-processed exception!\n";
        throw;
    }
    if (!file.is_open()) {
        throw std::runtime_error("IDK why, but the file is not opened. !file.is_open()\n");
    }


    TString line;
    while (line.ReadLine(file)) {

        // strip leading spaces and skip comments
        line.Remove(TString::kLeading, ' ');
        if (line.BeginsWith("#")) continue;

        if (line.BeginsWith("CYCLEWAIT")) {
            int time;
            sscanf(line.Data(), "CYCLEWAIT %d", &time);
            fCycleWait = time;
        } else if (line.BeginsWith("ETIPADDRESS")) {
            char hfile[150];
            sscanf(line.Data(), "ETIPADDRESS %s", hfile);
            fETIPAddress = hfile;
        } else if (line.BeginsWith("ETSTATIONNAME")) {
            char hfile[150];
            sscanf(line.Data(), "ETSTATIONNAME %s", hfile);
            fETStationName = hfile;
        } else if (line.BeginsWith("INPUTFILENAME")) {
            char hfile[150];
            sscanf(line.Data(), "INPUTFILENAME %s", hfile);
            fInputFileName = hfile;
        } else if (line.BeginsWith("OUTPUTFILENAME")) {
            char hfile[150];
            sscanf(line.Data(), "OUTPUTFILENAME %s", hfile);
            fOutputFileName = hfile;
        } else if (line.BeginsWith("PEDFILENAME")) {
            char hfile[150];
            sscanf(line.Data(), "PEDFILENAME %s", hfile);
            fPedFileName = hfile;
        } else if (line.BeginsWith("INPUTMAPNAME")) {
            char hfile[150];
            sscanf(line.Data(), "INPUTMAPNAME %s", hfile);
            fInputMapName = hfile;
        } else if (line.BeginsWith("CODAFILEDIR")) {
            char hfile[150];
            sscanf(line.Data(), "CODAFILEDIR %s", hfile);
            fCODAFileDir = hfile;
        } else if (line.BeginsWith("ISONLINEMODE")) {
            int n;
            sscanf(line.Data(), "ISONLINEMODE %d", &n);
            fIsOnlineMode = n;
        } else if (line.BeginsWith("NCOL")) {
            int n;
            sscanf(line.Data(), "NCOL %d", &n);
            fNCol = n;
        } else if (line.BeginsWith("TCPPORT")) {
            int n;
            sscanf(line.Data(), "TCPPORT %d", &n);
            fTCPPort = n;
        } else if (line.BeginsWith("ETMODE")) {
            int n;
            sscanf(line.Data(), "ETMODE %d", &n);
            fETMode = n;
        } else if (line.BeginsWith("RUNTYPE")) {
            char hfile[150];
            sscanf(line.Data(), "RUNTYPE %s", hfile);
            fRunType = hfile;
        } else if (line.BeginsWith("NBTIMESAMPLES")) {
            int n;
            sscanf(line.Data(), "NBTIMESAMPLES %d", &n);
            fNbOfTimeSamples = n;
        } else if (line.BeginsWith("STARTTIMESAMPLES")) {
            int n;
            sscanf(line.Data(), "STARTTIMESAMPLES %d", &n);
            fStartTimeSamples = n;
        } else if (line.BeginsWith("STOPTIMESAMPLES")) {
            int n;
            sscanf(line.Data(), "STOPTIMESAMPLES %d", &n);
            fStopTimeSamples = n;
        } else if (line.BeginsWith("ZEROSUPCUT")) {
            int n;
            sscanf(line.Data(), "ZEROSUPCUT %d", &n);
            fZeroSupCut = n;
        } else if (line.BeginsWith("COMMODECUT")) {
            int n;
            sscanf(line.Data(), "COMMODECUT %d", &n);
            fComModeCut = n;
        } else if (line.BeginsWith("HITPEAKORSUMADCS")) {
            char hfile[150];
            sscanf(line.Data(), "HITPEAKORSUMADCS %s", hfile);
            fIsHitPeakOrSumADCs = hfile;
        } else if (line.BeginsWith("CENTRALORALLSTRIPS")) {
            char hfile[150];
            sscanf(line.Data(), "CENTRALORALLSTRIPS %s", hfile);
            fIsCentralOrAllStripsADCs = hfile;
        } else if (line.BeginsWith("MINADCVALUE")) {
            int n;
            sscanf(line.Data(), "MINADCVALUE %d", &n);
            fMinADCvalue = n;
        } else if (line.BeginsWith("MAXADCVALUE")) {
            int n;
            sscanf(line.Data(), "MAXADCVALUE %d", &n);
            fMaxADCvalue = n;
        } else if (line.BeginsWith("MINCLUSTSIZE")) {
            int n;
            sscanf(line.Data(), "MINCLUSTSIZE %d", &n);
            fMinClusterSize = n;
        } else if (line.BeginsWith("MAXCLUSTSIZE")) {
            int n;
            sscanf(line.Data(), "MAXCLUSTSIZE %d", &n);
            fMaxClusterSize = n;
        } else if (line.BeginsWith("MAXCLUSTMULT")) {
            int n;
            sscanf(line.Data(), "MAXCLUSTMULT %d", &n);
            fMaxClusterMult = n;
        } else if (line.BeginsWith("FIRSTEVENT")) {
            int n;
            sscanf(line.Data(), "FIRSTEVENT %d", &n);
            fFirstEvent = n;
        } else if (line.BeginsWith("LASTEVENT")) {
            int n;
            sscanf(line.Data(), "LASTEVENT %d", &n);
            fLastEvent = n;
        }
    }
    Dump();
}

void GemConfiguration::Dump() const {
    printf("  GemConfiguration::Load() ==> CYCLEWAIT           %d\n", fCycleWait);
    printf("  GemConfiguration::Load() ==> ETIPADDRESS         %s\n", fETIPAddress.Data());
    printf("  GemConfiguration::Load() ==> ETSTATIONNAME       %s\n", fETStationName.Data());
    printf("  GemConfiguration::Load() ==> INPUTFILENAME       %s\n", fInputFileName.Data());
    printf("  GemConfiguration::Load() ==> OUTPUTFILENAME      %s\n", fOutputFileName.Data());
    printf("  GemConfiguration::Load() ==> PEDFILENAME         %s\n", fPedFileName.Data());
    printf("  GemConfiguration::Load() ==> INPUTMAPNAME        %s\n", fInputMapName.Data());
    printf("  GemConfiguration::Load() ==> RUNTYPE             %s\n", fRunType.Data());
    printf("  GemConfiguration::Load() ==> ISONLINEMODE        %d\n", fIsOnlineMode);
    printf("  GemConfiguration::Load() ==> NCOL                %d\n", fNCol);
    printf("  GemConfiguration::Load() ==> TCPPORT             %d\n", fTCPPort);
    printf("  GemConfiguration::Load() ==> ETMODE              %d\n", fETMode);
    printf("  GemConfiguration::Load() ==> ZEROSUPCUT          %d\n", fZeroSupCut);
    printf("  GemConfiguration::Load() ==> COMMODECUT          %d\n", fComModeCut);
    printf("  GemConfiguration::Load() ==> STOPTIMESAMPLES     %d\n", fStopTimeSamples);
    printf("  GemConfiguration::Load() ==> STARTTIMESAMPLES    %d\n", fStartTimeSamples);
    printf("  GemConfiguration::Load() ==> NBTIMESAMPLES       %d\n", fNbOfTimeSamples);
    printf("  GemConfiguration::Load() ==> CENTRALORALLSTRIPS  %s\n", fIsCentralOrAllStripsADCs.Data());
    printf("  GemConfiguration::Load() ==> HITPEAKORSUMADCS    %s\n", fIsHitPeakOrSumADCs.Data());
    printf("  GemConfiguration::Load() ==> MAXADCVALUE         %d\n", fMaxADCvalue);
    printf("  GemConfiguration::Load() ==> MINADCVALUE         %d\n", fMinADCvalue);
    printf("  GemConfiguration::Load() ==> MINCLUSTSIZE        %d\n", fMinClusterSize);
    printf("  GemConfiguration::Load() ==> FIRSTEVENT          %d\n", fFirstEvent);
    printf("  GemConfiguration::Load() ==> LASTEVENT           %d\n", fLastEvent);
}
