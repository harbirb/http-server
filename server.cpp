#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>

int sockfd, new_sockfd;
int PORT = 8080;

// close sockets upon signal interruption, enable re-use of ports
void handle_sigint(int sig)
{
    close(sockfd);
    printf("\nSocket closed. Exiting gracefully.\n");
    exit(0);
}

int main()
{
    struct sockaddr_in server_addr;

    // create socket, IPV4, type=STREAM
    signal(SIGINT, handle_sigint);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Error in creating socket");
        return 1;
    }

    // socket bind configuration - use loopback address (localhost), ipv4, port 8080
    // htonl - host-to-netword conversion - IP is 4-byte long, port is 2 byte short
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        perror("Error binding socket");
        close(sockfd);
        return 1;
    }

    // stored in network byte order (big-endian), need to convert to human-readable format
    uint32_t local_ip = server_addr.sin_addr.s_addr;
    printf("\nServer is live at: http://%d.%d.%d.%d:%d\n", local_ip & 0xff, local_ip >> 8 & 0x0ff, local_ip >> 16 & 0x0ff, local_ip >> 24 & 0x0ff, PORT);
    printf("aka: https://localhost:%d\n", PORT);

    // make the socket listen, with a max queue of pending connections = 10
    if (listen(sockfd, 10) != 0)
    {
        perror("Error listening to socket");
        close(sockfd);
        return 1;
    }

    while (true)
    {
        struct sockaddr_in clientaddr;
        socklen_t client_len = sizeof(clientaddr);

        // accept incoming connection and store client address info
        if ((new_sockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &client_len)) == -1)
        {
            perror("Error accepting connection");
            continue; // skip to accepting the next connection
        }

        // Read data from new_sockfd into buffer, null terminate, then print out
        char buf[4096];
        int bytes_read = read(new_sockfd, buf, 4095);
        if (bytes_read > 0)
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
        else
        {
            perror("Error reading fd");
        }
        printf("END\n");

        close(new_sockfd);
    }

    close(sockfd);
    return 0;
}