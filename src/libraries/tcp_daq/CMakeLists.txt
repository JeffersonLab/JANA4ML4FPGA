project(lib_tcp_daq)

print_header(">>>>   L I B R A R Y :   tcp_daq    <<<<")       # Fancy printing

add_library(tcp_daq STATIC
        tcp_event.cc
        tcp_event.h
        tcp_thread.cc
        tcp_thread.h
        )

