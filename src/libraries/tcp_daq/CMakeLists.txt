project(tcp_daq_lib)

add_library(tcp_daq STATIC
        tcp_event.cc
        tcp_event.h
        tcp_thread.cc
        tcp_thread.h
        )