project(tcp_tester)

print_header(">>>>   E X E C U T A B L E S : tcp_receiver, tcp_sender   <<<<")       # Fancy printing

# TCP receiver executable
add_executable(tcp_receiver tcp_receiver.cc tcp_receiver.h)
target_link_libraries(tcp_receiver tcp_daq)

# TCP sender executable
add_executable(tcp_sender tcp_sender.cc)
target_link_libraries(tcp_sender tcp_daq)
