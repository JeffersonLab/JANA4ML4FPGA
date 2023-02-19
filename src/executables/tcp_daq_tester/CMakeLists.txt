project(tcp_tester)

add_executable(tcp_receiver
        tcp_receiver.cc
        tcp_receiver.h
        )

add_executable(tcp_sender
        tcp_sender.cc
        )