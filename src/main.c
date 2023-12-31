/*! \file   main.c
    \brief  Main module functions
    This is \ref main.c */

/* Includes */
#include "main.h"

#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "async.h"
#include "cartridge.h"
#include "cryostat.h"
#include "debug.h"
#include "error_local.h"
#include "fetim.h"
#include "globalDefinitions.h"
#include "globalOperations.h"
#include "packet.h"
#include "serialMux.h"
#include "timer.h"
#include "version.h"

/* Globals */
/* Externs */
CAN_MESSAGE CANMessage; /*!< This variable hold the latest message
                             received. */

unsigned char currentClass = 0; /*!< This variable stores the current class
                                     information. This is a specifier of the
                                     type of message: monitor, control or
                                     special that has been received. */

#define SOCK_BUFF_SIZE 64
#define PORT 2000
#define SA struct sockaddr

int amb_port = 0;
int abm_machine = 0;

unsigned char buff_sock[SOCK_BUFF_SIZE] = {0};
unsigned char out_buff_sock[SOCK_BUFF_SIZE] = {0};

// Function designed for chat between client and server.
static inline void func_sock(int connfd) {
    for (;;) {
        bzero(buff_sock, SOCK_BUFF_SIZE);

        // Read the message from client and copy it in buffer
        int x = read(connfd, buff_sock, SOCK_BUFF_SIZE);

        // print buffer which contains the client contents
        switch (x) {
            case -1:
                // printf("Exiting server\n");
                break;
            case 0:
                printf("Empty\n");
                return;
            case 18:
                amb_port = buff_sock[0];
                abm_machine = buff_sock[1];
                // Get RCA from socket message
                unsigned long rca =
                    ((buff_sock[4] & 0xFF) << 24) + ((buff_sock[5] & 0xFF) << 16) + (buff_sock[6] << 8) + buff_sock[7];
                // Get X from socket message
                unsigned long type = buff_sock[8];
                // Get length from socket message
                unsigned long length = buff_sock[9];
                switch (rca & 0xFFFFF) {
                    /* Slave software revision level */
                    case 0x30004:
                        out_buff_sock[4] = 3;
                        out_buff_sock[5] = 1;
                        out_buff_sock[6] = 2;
                        out_buff_sock[7] = 3;
                        write(connfd, out_buff_sock, 13 * sizeof(char));
                        break;
                    /* Slave protocol revision level */
                    case 0x30000:
                        out_buff_sock[4] = 3;
                        out_buff_sock[5] = 1;
                        out_buff_sock[6] = 2;
                        out_buff_sock[7] = 3;
                        write(connfd, out_buff_sock, 13 * sizeof(char));
                        break;
                    /* Number of transactions */
                    case 0x30002:
                        out_buff_sock[4] = 4;
                        out_buff_sock[5] = 0;
                        out_buff_sock[6] = 0;
                        out_buff_sock[7] = 0;
                        out_buff_sock[8] = 1;
                        write(connfd, out_buff_sock, 13 * sizeof(unsigned char));
                        break;
                    /* Ambient temperature */
                    case 0x30003:
                        out_buff_sock[4] = 4;
                        out_buff_sock[5] = 0x40;
                        out_buff_sock[6] = 1;
                        out_buff_sock[7] = 1;
                        out_buff_sock[8] = 1;
                        write(connfd, out_buff_sock, 13 * sizeof(unsigned char));
                        break;
                    /* Number of errors and last error */
                    case 0x30001:
                        out_buff_sock[4] = 4;
                        write(connfd, out_buff_sock, 13 * sizeof(unsigned char));
                        break;
                    /* Request by LabVIEW I don't understand this yet */
                    case 0:
                        out_buff_sock[4] = 1;
                        write(connfd, out_buff_sock, 13 * sizeof(char));
                        if (type == 2) {
                            out_buff_sock[0] = 0;
                            out_buff_sock[1] = 0;
                            out_buff_sock[2] = 0;
                            out_buff_sock[3] = 0x13;
                            out_buff_sock[4] = 0x10;
                            write(connfd, out_buff_sock, 12 * sizeof(char));
                        }
                        break;
                    /* Process RCAs */
                    default:
                        CAN_ADDRESS = (rca & 0xFFFFF);
                        if (type == 0x1) {
                            CAN_SIZE = length;
                            for (int i = 0; i < CAN_SIZE; i++) {
                                CAN_DATA(i) = buff_sock[10 + i];
                            }
                        } else if (type == 0x0) {
                            CAN_SIZE = 0;
                        }
                        CANMessageHandler();
                        out_buff_sock[4] = CAN_SIZE;
                        for (int i = 0; i < CAN_SIZE; i++) {
                            out_buff_sock[5 + i] = CAN_DATA(i);
                        }
                        write(connfd, out_buff_sock, 13 * sizeof(char));
                        break;
                }
                bzero(out_buff_sock, SOCK_BUFF_SIZE);
                break;
            /* Default printing the message */
            default:
                printf("From client: %d\n", x);
                for (int i = 0; i < x; i++) {
                    printf("0x%02X ", buff_sock[i]);
                }
                printf("\n");
                break;
        }
    }
}

int sockfd, connfd;
unsigned int len;
struct sockaddr_in servaddr, cli;

void create_socket() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else {
        printf("Socket successfully created..\n");
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    } else {
        printf("Socket successfully binded..\n");
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else {
        printf("Server listening..\n");
    }
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA *)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    } else {
        printf("server accept the client...\n");
    }
}

void *cryostatAsyncWrapper(void *arg) {
    for (;;) {
        cryostatAsync();
    }
    return NULL;
}

void *cartridgeAsyncWrapper(void *arg) {
    for (;;) {
        cartridgeAsync();
    }
    return NULL;
}

void *fetimAsyncWrapper(void *arg) {
    for (;;) {
        fetimAsync();
    }
    return NULL;
}

int main(void) {
    /* Print version information */
    displayVersion();

    /* Initialize the frontend */
    if (initialization() == ERROR) {
        return ERROR;
    }

    pthread_t tid[3];
    int error;

    error = pthread_create(&(tid[0]), NULL, &cryostatAsyncWrapper, NULL);
    if (error != 0) printf("\nThread can't be created :[%s]", strerror(error));
    error = pthread_create(&(tid[1]), NULL, &cartridgeAsyncWrapper, NULL);
    if (error != 0) printf("\nThread can't be created :[%s]", strerror(error));
    error = pthread_create(&(tid[2]), NULL, &fetimAsyncWrapper, NULL);
    if (error != 0) printf("\nThread can't be created :[%s]", strerror(error));

    /* Initialize socket */
    while (1) {
        create_socket();

        /* Main loop */
        func_sock(connfd);

        // Close socket
        shutdown(sockfd, SHUT_RDWR);
    }

    /* Shut down the frontend */
    if (shutDown() == ERROR) {
    }

    return NO_ERROR;
}
