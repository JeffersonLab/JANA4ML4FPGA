
The cdaq plugin supports both reading and writing CDAQ events over TCP. It can do this simultaneously so that any number of JANA processes can be linked together into a pipeline. 

Below is an example that connects 3 processes where one process reads events from a file and sends them to another process via TCP. The second process then sends the events on to the 3rd process via TCP. The process should generally be started in the order starting with the last process in the chain and going towards the first (i.e. the process reading events from the file or live data stream.)

~~~
jana4ml4fpga -Pplugins=cdaq -Pcdaq:port=20250 tcpcdaqevio
jana4ml4fpga -Pplugins=cdaq -Pcdaq:port=20249 -Pcdaq:remote_port=20250 tcpcdaqevio
jana4ml4fpga -Pplugins=CDAQfile,cdaq DATA/hd_rawdata_003101_000.evio -Pcdaq:remote_port=20249
~~~

In the above commands, the `cdaq:port` configuration parameter is used to specify the port the process will listen on to receive events. The `cdaq:remote_port` is used to specify the port to connect to and send events over. There are corresponding `cdaq:host` and `cdaq:remote_host` variables that can be used to specify alternative hosts. If a host name is not specified, it defaults to `localhost`.


