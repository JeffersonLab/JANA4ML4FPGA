project(tcp_tester)

add_executable(tcp_receiver
        jana_input.cc
        jana_input.h
        jana_tcp.cc
        jana_tcp.h
        tcp_thread.cc
        tcp_thread.h
        )

add_executable(tcp_sender
        jana_main.cc
        jana_tcp.cc
        jana_tcp.h
        tcp_thread.cc
        tcp_thread.h
        )