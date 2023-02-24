#include "tcp_thread.h"

//struct sockaddr_in sinn;
//struct sockaddr_in pin;
//struct hostent *gethostbyname(char *);
//struct hostent *hp;
//int	sd, sd_current;
extern int sig_int, sig_hup, sig_alarm, sig_pipe;
/*====================================================================*/
/*             TCP open                                               */
/*====================================================================*/
int tcp_open_th(int port, char *hostname) {

    int sd = 0;
    printf("  Doing auto local/remote open socket tcp_open_th:: PORT=%d HOST=%s \n", port, hostname);

    if (port < 0) {
        port = -port;
        sd = tcp_open_remote_th(port, hostname);
    } else {
        sd = tcp_open_local_th(port);
    }
    return sd;
}

/*--------------------------------------------------------------------*/
int tcp_open_local_th(int PORT) {
    /*--- LISTEN is implied -----*/
    printf("  Doing tcp_open_local_th (Listening local port):: PORT=%d \n", PORT);
    // Create an endpoint for communication
    // Get socket file descriptor
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("    ERROR creating endpoint at 'socket(...)'");
        return -1;  // -1 is in C specs
    }

    // Set the socket options
    printf("    Setting the socket options SO_REUSEADDR for socket_FD=%d \n", socket_fd);
    int opt_value = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value)) < 0) {
        perror("    ERROR  Set socket options at setsockopt(...)");

        // Cleaning and aborting
        close(socket_fd);
        return -1;
    }

    /* complete the socket structure */
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(PORT);

    printf("    Binding socket socket_FD=%d\n", socket_fd);
    /* bind the socket to the port number */
    if (bind(socket_fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == -1) {
        perror("    ERROR binding socket at bind(...)");
        close(socket_fd);
        return -1;
    }

    printf("    Success! Socket is bound\n");
    return socket_fd;
}

/*--------------------------------------------------------------------*/
int tcp_open_remote_th(int PORT, char *hostname) {   /*----- CONNECT  !!!  ----*/
    struct sockaddr_in pin;
    struct hostent *hp;
    int sd_current;
    if (!sig_hup) printf(" go find out about the desired host machine \n");
    if ((hp = gethostbyname(hostname)) == 0) {
        if (!sig_hup) perror("gethostbyname");
        return -1;
    }
    if (!sig_hup) printf(" name=%s \n", hp->h_name);
    if (!sig_hup) printf(" alias=%s \n", (char *) hp->h_aliases);
    if (!sig_hup) printf(" adr_type=0x%x \n", hp->h_addrtype);
    if (!sig_hup) printf(" adr_len=%d \n", hp->h_length);
    if (!sig_hup) printf(" address=%s \n", hp->h_addr);

    if (!sig_hup) printf(" fill in the socket structure with host information \n");
    //        sprintf(hp->h_addr,"131.169.46.71");

    /* fill in the socket structure with host information */
    memset(&pin, 0, sizeof(pin));

    if (!sig_hup) printf(" after memset info \n");

    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
    pin.sin_port = htons(PORT);

    if (!sig_hup) printf(" grab an Internet domain socket\n ");

    if ((sd_current = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        if (!sig_hup) perror("socket");
        return -1;
    }

    if (!sig_hup) printf(" get socket option \n ");
    // getsockopt(sd,0,SO_SNDBUF,optval,optlen);

    // printf("optval,optlen=  %d  %d \n",*optval,*optlen);
    //printf("optval,optlen=  %s  %d \n",*optval,*optlen);


    if (!sig_hup) printf(" try to connect \n");
    /* connect to PORT on HOST */
    if (connect(sd_current, (struct sockaddr *) &pin, sizeof(pin)) == -1) {
        if (!sig_hup) perror("connect");
        return -1;
    }
    return sd_current;
}


/*====================================================================*/
/*             TCP send    data    2                                  */
/*====================================================================*/
int tcp_send_th(int sd_current, int *DATA, int lenDATA) {
    int nleft, nsent;
    char *snd;
    //-----------------------------------------------------
    nleft = lenDATA;
    snd = (char *) DATA;
    while (nleft > 0) {
        // printf("try to send = %d of %d\n",PACKSIZE,nleft);
        nsent = send(sd_current, snd, nleft, 0);
        if (nsent <= 0) {
            perror("send");
            return -1;
        }
        nleft -= nsent;
        snd += nsent;
    }
    return nleft;
}

/*====================================================================*/
int tcp_get(int socket_fd, int *data, int data_len) {
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

/*====================================================================*/
int tcp_listen3(int socket_fd, char *host_name, int len, int *sd_current) {
    printf("  Doing tcp_listen3::, waiting for replay at  \n");
    printf("    socket_fd=%d\n len=%d", socket_fd, len);

    int rem_port;

    /* show that we are willing to listen */
    if (listen(socket_fd, 5) == -1) {
        perror("    ERROR at listen for");
        return -1;
    }

    /* wait for a client to talk to us */
    struct sockaddr_in pin;
    socklen_t addr_len = sizeof(pin);
    if ((*sd_current = accept(socket_fd, (struct sockaddr *) &pin, &addr_len)) == -1) {
        perror("accept");
        return -1;
    }
    rem_port = ntohs(pin.sin_port);
    printf("listen:: hp->h_name:: rem_port=%d \n", rem_port);

    printf("listen:: try get host by addr %s \n", inet_ntoa(pin.sin_addr));

    if ((hp = gethostbyaddr(&pin.sin_addr, sizeof(pin.sin_addr), AF_INET)) == 0) {
        perror("tcp_listen3()::gethostbyaddr");
        strncpy(host_name, inet_ntoa(pin.sin_addr), len);
        host_name[len - 1] = 0;
    } else {
        printf("listen3:: hp->h_name:: ptr=%p \n", hp->h_name);
        strncpy(host_name, hp->h_name, len);
        host_name[len - 1] = 0;
    }
    printf("listen3:: Connection from %s(%s) accept at sock=%d rem_port=%d\n", host_name, inet_ntoa(pin.sin_addr),
           *sd_current, rem_port);
    return rem_port;

}
/*==================================================================*/
