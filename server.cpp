#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>

int sockfd, new_sockfd;

// close sockets upon signal interruption
// enable re-use of ports
void handle_sigint(int sig)
{
    close(sockfd);
    printf("\nSocket closed. Exiting gracefully.\n");
    exit(0);
}

int main()
{
    signal(SIGINT, handle_sigint);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Error in creating socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    // bind socket to local machine addr at port specified in getaddrinfo
    // ports 1024 and below are priveleged
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        perror("Error binding socket");
        close(sockfd);
        return 1;
    }
    printf("Listening on localhost:8080");

    // stored in network byte order (big-endian)
    uint32_t local_ip = server_addr.sin_addr.s_addr;
    printf("\nIP is: %d.%d.%d.%d\n", local_ip & 0xff, local_ip >> 8 & 0x0ff, local_ip >> 16 & 0x0ff, local_ip >> 24 & 0x0ff);
    // printf("\nPort is: %d\n", local_addr.sin_port);

    // make the socket listen, with a queue of 5
    if (listen(sockfd, 900) != 0)
    {
        perror("Error listening to socket");
        close(sockfd);
        return 1;
    }

    struct sockaddr_in clientaddr;
    socklen_t client_len = sizeof(clientaddr);

    while (true)
    {
        // accept incoming connection and store client address info
        if ((new_sockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &client_len)) == -1)
        {
            perror("Error accepting connection");
        }
        printf("START\n");

        // Read data from new_sockfd into buffer, null terminate, then print out
        char buf[4096];
        int bytes_read = read(new_sockfd, buf, 4096);
        if (bytes_read < 0)
        {
            perror("Error reading fd");
        }
        else
        {

            buf[bytes_read] = '\0';
            printf("\n%s\n", buf);

            // return basic response
            char resp[4096]{0};
            // Status line
            strcpy(resp, "HTTP/1.1 404 Not Found\r\n");
            // optional headers
            strcat(resp, "Content-Type: text/html\r\n");
            // empty line
            strcat(resp, "\r\n");
            // response body
            strcat(resp, "HELLO WORLD");
            int bytes_written = write(new_sockfd, resp, sizeof(resp));

            if (bytes_written > 1)
            {
                printf("Sent response!");
            }
        }
        printf("END\n");

        close(new_sockfd);
    }

    close(sockfd);
    return 0;
}