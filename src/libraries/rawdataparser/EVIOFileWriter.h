
#include <mutex>
#include <fstream>

#include <rawdataparser/EVIOBlockedEvent.h>
#include <rawdataparser/swap_bank.h>

class EVIOFileWriter{

    std::mutex    m_mtx;
	std::string   m_outfile_name;
	std::ofstream outfile_ofs;

    uint32_t m_blocks_written = 0; // block number

public:

    //----------------------------------
    // EVIOFileWriter (Constructor)
    //----------------------------------
    EVIOFileWriter(std::string filename): m_outfile_name(filename){

        outfile_ofs.open( m_outfile_name );
        if( outfile_ofs.is_open() ){
            LOG << "Opened EVIO file for writing: " << m_outfile_name << LOG_END;
        }else{
            LOG_ERROR(default_cerr_logger) << "Unable to create EVIO output file: " << m_outfile_name << LOG_END;
            throw JException("Error opening EVIO output file!");
        }
    }

    //----------------------------------
    // ~EVIOFileWriter
    //----------------------------------
    ~EVIOFileWriter(){
        if( outfile_ofs.is_open() ){
         
            // Write EVIO trailer to file

            // Close output file
            outfile_ofs.close();
        }
    }

    //----------------------------------
    // WriteEventBuffer
    //----------------------------------
    void WriteEventBuffer(EVIOBlockedEvent &block){

        if( ! outfile_ofs.is_open() ) throw JException("Attempting to write EVIO event block to file that is not open!");

        std::lock_guard<std::mutex> lck(m_mtx);

        // Write EVIO header to file
        uint32_t header[8];
        uint32_t bitinfo = (1<<9) + (1<<10); //  (1<<9)=Last Event in ET stack, (1<<10)="Physics" payload
        header[0] = block.data.size() + 8; // number of 32bit words in block, including this 8 word header
        header[1] = ++m_blocks_written;  // Block number
        header[2] = 8; // Length of block header
        header[3] = 1; // Event count
        header[4] = 0; // Reserved 1
        header[5] = (bitinfo<<8) + 0x4; // 0x4= EVIO version 4
        header[6] = 0; // Reserved 2
        header[7] = 0xc0da0100; // Magic number

        outfile_ofs.write((const char*)header, 8*4);


        // Write data block to file
        outfile_ofs.write((const char*)block.data.data(), block.data.size()*4);
    }
};

