#pragma once

#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <spdlog/logger.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <sys/socket.h>

class CDaqEventSource : public JEventSource, spdlog::extensions::SpdlogMixin<CDaqEventSource>
{

    /// Add member variables here

public:
    CDaqEventSource(std::string resource_name, JApplication* app);

    virtual ~CDaqEventSource() = default;

    void Open() override;

    void GetEvent(std::shared_ptr<JEvent>) override;
    
    static std::string GetDescription();

private:

    void ThrowOnErrno (const std::string& comment);

    int32_t TcpReadData(int socket_fd, int *data, int data_len) {
        int bytes_left = data_len;
        auto data_bytes = (char *) data;
        while (bytes_left > 0) {
            // Read data
            int read_len = recv(socket_fd, data_bytes, bytes_left, 0);
            if (read_len <= 0) {
                perror("    ERROR at tcp_get recv()");
                return -1;
            }

            // Prepare for the next read
            bytes_left -= read_len;
            data_bytes += read_len;
        }
        return bytes_left;
    }

    void WaitForClient();




    int m_port;
    std::string m_remote_host;


    int m_socket_fd;                            /// Socket file descriptor that is used in all socket C funcs
    std::shared_ptr<spdlog::logger> m_log;      /// logger
    int TCP_FLAG;                               /// 0 failed state, 1 working state
    std::atomic<bool> m_is_connected;           /// Flag that client is connected now
    std::thread m_listen_thread;                /// Thread which is used to wait for client connection
};

template <>
double JEventSourceGeneratorT<CDaqEventSource>::CheckOpenable(std::string);



