/*! \file   main.c
    \brief  Main module functions
    This is \ref main.c */

/* Includes */
#include "main.h"

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "async.h"
#include "debug.h"
#include "error_local.h"
#include "globalDefinitions.h"
#include "globalOperations.h"
#include "packet.h"
#include "timer.h"
#include "version.h"

/* Globals */
/* Externs */
unsigned char stop = 0;    /*!< This global can be set by any module and will
                              gently    stop the program if necessary. */
unsigned char restart = 0; /*!< This global can be set by any module and will
                                cause the system to reboot after a stop is
                                received. */

volatile unsigned char newCANMsg = 0; /*!< This variable is a semaphore which will
                                           notify the entire program of the arrival
                                           of a new CAN message. It is cleared
                                           once the message has been dealt with. */
CAN_MESSAGE CANMessage;               /*!< This variable hold the latest message
                                           received. */
unsigned char currentModule = 0;      /*!< This variable stores the current module
                                           information. This is the front end item
                                           the request are currently directed to. */
unsigned char currentClass = 0;       /*!< This variable stores the current class
                                           information. This is a specifier of the
                                           type of message: monitor, control or
                                           special that has been received. */

#define MAX 64
#define PORT 2300
#define SA struct sockaddr

int amb_port = 0;
int abm_mach = 0;

unsigned char buff_sock[MAX] = {0};
unsigned char out_buff_sock[MAX] = {0};

// Function designed for chat between client and server.
static inline void func_sock(int connfd) {
    int n = 0;
    // infinite loop for chat
    for (;;) {
        bzero(buff_sock, MAX);

        // read the message from client and copy it in buffer
        int x = read(connfd, buff_sock, MAX);

        // print buffer which contains the client contents
        switch (x) {
            case -1:
                printf("Exiting server\n");
                return;
            case 0:
                printf("Empty\n");
                break;
            case 18:
                amb_port = buff_sock[0];
                abm_mach = buff_sock[1];
                unsigned long rca =
                    ((buff_sock[4] & 0xFF) << 24) + ((buff_sock[5] & 0xFF) << 16) + (buff_sock[6] << 8) + buff_sock[7];
                unsigned long type = buff_sock[8];
                unsigned long length = buff_sock[9];
                switch (rca & 0xFFFFF) {
                    case 0x30004:
                        out_buff_sock[4] = 3;
                        out_buff_sock[5] = 1;
                        out_buff_sock[6] = 2;
                        out_buff_sock[7] = 3;
                        write(connfd, out_buff_sock, 13 * sizeof(char));
                        break;
                    case 0x30000:
                        out_buff_sock[4] = 3;
                        out_buff_sock[5] = 1;
                        out_buff_sock[6] = 2;
                        out_buff_sock[7] = 3;
                        write(connfd, out_buff_sock, 13 * sizeof(char));
                        break;
                    case 0x30002:
                        out_buff_sock[4] = 4;
                        out_buff_sock[5] = 0;
                        out_buff_sock[6] = 0;
                        out_buff_sock[7] = 0;
                        out_buff_sock[8] = 1;
                        write(connfd, out_buff_sock, 13 * sizeof(unsigned char));
                        break;
                    case 0x30003:
                        out_buff_sock[4] = 4;
                        out_buff_sock[5] = 0x40;
                        out_buff_sock[6] = 1;
                        out_buff_sock[7] = 1;
                        out_buff_sock[8] = 1;
                        write(connfd, out_buff_sock, 13 * sizeof(unsigned char));
                        break;
                    case 0x30001:
                        out_buff_sock[4] = 4;
                        write(connfd, out_buff_sock, 13 * sizeof(unsigned char));
                        break;
                    case 0x2000B:
                        n++;
                        if (n != 2) {
                            out_buff_sock[4] = 8;
                            out_buff_sock[5] = 1;
                            out_buff_sock[6] = 2;
                            out_buff_sock[7] = 3;
                            out_buff_sock[8] = 4;
                            out_buff_sock[9] = 1;
                            out_buff_sock[10] = 2;
                            out_buff_sock[11] = 3;
                            out_buff_sock[12] = 4;
                        } else {
                            out_buff_sock[4] = 8;
                            n = 0;
                        }
                        write(connfd, out_buff_sock, 13 * sizeof(unsigned char));
                        break;
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
                bzero(out_buff_sock, MAX);
                break;
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

int main(void) {
    /* Print version information */
    displayVersion();

    /* Initialize the frontend */
    if (initialization() == ERROR) {
        return ERROR;
    }

    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
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

    // Function for chatting between client and server

    /* Main loop */
    func_sock(connfd);

    /* Shut down the frontend */
    if (shutDown() == ERROR) {
    }

    // After chatting close the socket
    shutdown(sockfd, SHUT_RDWR);

    return NO_ERROR;
}
