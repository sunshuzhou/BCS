
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

#include "param.h"

#define BUFFSIZE 32
void Die(char *mess) { perror(mess); exit(1); }

/**
 * return time in millisecond
 */
double get_unix_time(void) {
    struct timespec tv;
    if (clock_gettime(CLOCK_REALTIME, &tv) != 0) {
        printf("Error at getting unix time...\n");
    }
    return (tv.tv_sec * 1e3 + (tv.tv_nsec * 1e-6));
}

#define N_C (N_FETCHER * N_EACH)

typedef struct {
    uint32_t data[72];
} Request;

Request gRequest[16384];

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in echoserver;
    char buffer[BUFFSIZE];
    unsigned int echolen;
    int received = 0;

    const int message_len = sizeof(uint32_t) * 72;
    printf("message_len: %d\n", message_len);

    FILE *fp = fopen("/home/sunshuzhou/Code/BCS/16384_data.txt", "r");
    if (!fp) {
        perror("Open file error\n");
    }
    for (int i = 0; i < 16384; i++) {
        for (int j = 0; j < 72; j++) {
            fscanf(fp, "%u", &gRequest[i].data[j]);
            gRequest[i].data[j] = htonl(gRequest[i].data[j]);
        }
    }
    fclose(fp);

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
    fp = fopen("/home/sunshuzhou/Code/BCS/log", "w");

#pragma omp parallel for num_threads(N_C) shared(gRequest)
    for (int i = 0; i < N_C; i++) {
        char message[1024];
        //sprintf(message, "Hello from client %d\n", i);
        double start, end;

        for (;;) {
            //sprintf(message, "Hello from client %d\n", i);
            start = get_unix_time();

            size_t len = write(connection[i], gRequest[i].data, message_len);

            //printf("Client(%d): waiting answers\n", connection[i], len);

            len = read(connection[i], message, 1024);
            end = get_unix_time();
            fprintf(fp, "Client(%d): start: %f, end: %f, cost: %f\n", connection[i], start, end, end - start);
        }
    }

    fclose(fp);

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
