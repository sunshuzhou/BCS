
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFSIZE 32
void Die(char *mess) { perror(mess); exit(1); }

#define N_C 64

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in echoserver;
    char buffer[BUFFSIZE];
    unsigned int echolen;
    int received = 0;

    /*
       if (argc != 4) {
       fprintf(stderr, "USAGE: TCPecho <server_ip> <word> <port>\n");
       exit(1);
       }
       */
    /* Create the TCP socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        Die("Failed to create socket");
    }

    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = inet_addr("127.0.0.1");  /* IP address */
    echoserver.sin_port = htons(9090);       /* server port */
    /* Establish connection */
    int connection[N_C];
    for (int i = 0; i < N_C; i++) {
        connection[i] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connect(connection[i], (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0) {
            Die("Failed to connect with server");
        } else {
            printf("%d connected at socket: %d\n", i, connection[i]);
        }
    }

    sleep(2);



#pragma omp parallel for num_threads(N_C)
    for (int i = 0; i < N_C; i++) {
        char message[1024];
        sprintf(message, "Hello from client %d\n", i);

        for (;;) {
            size_t len = write(connection[i], message, strlen(message));
            printf("Client(%d): %d bytes writed\n", i, len);
            read(connection[i], message, 1024);
            printf("%s", message);
            sleep(1);
        }
    }

    for (int i = 0; i < N_C; i++) {
        shutdown(connection[i], 2);
    }

    /* Send the word to the server */
    echolen = strlen(argv[2]);
    if (send(sock, argv[2], echolen, 0) != echolen) {
        Die("Mismatch in number of sent bytes");
    }

    /* Receive the word back from the server */
    fprintf(stdout, "Received: ");
    while (received < echolen) {
        int bytes = 0;
        if ((bytes = recv(sock, buffer, BUFFSIZE-1, 0)) < 1) {
            Die("Failed to receive bytes from server");
        }
        received += bytes;
        buffer[bytes] = '\0';        /* Assure null terminated string */
        fprintf(stdout, buffer);
    }

    fprintf(stdout, "\n");
    close(sock);
    exit(0);
}
