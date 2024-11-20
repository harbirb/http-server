#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

int sockfd, client_sockfd;
int PORT = 8080;
int BUFFER_SIZE = 4096;

void requestHandler(int client_sockfd) {
    // Read data from client_sockfd into buffer, null terminate, then print out
    char buf[BUFFER_SIZE];
    int bytes_read = recv(client_sockfd, buf, BUFFER_SIZE, 0);
    if (bytes_read > 0)
    {
        buf[bytes_read] = '\0';
        printf("Recieved request: \n%s\n", buf);
    }
    else
    {
        perror("Error reading fd");
    }

}

void responseHandler(int client_sockfd) {
    char resp[BUFFER_SIZE]{0};
    // Status line
    strcpy(resp, "HTTP/1.1 200 OK\r\n");
    // optional headers
    strcat(resp, "Content-Type: text/html\r\n");
    // empty line
    strcat(resp, "\r\n");
    // response body
    strcat(resp, "Hello World");
    int bytes_written = send(client_sockfd, resp, sizeof(resp), 0);
    if (bytes_written > 1)
    {
        printf("\nSent response:\n%s\n", resp);
    }


    printf("END\n");

}


void* handleClient(void* client_socket) {
    int client_sockfd = *(int*)client_socket;
    requestHandler(client_sockfd);
    responseHandler(client_sockfd);
    
    close(client_sockfd);
    return nullptr;
}


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

    // infinite loop to accept new clients
    while (true)
    {
        // accept incoming connection and store client address info
        if ((client_sockfd = accept(sockfd, (struct sockaddr *)NULL, NULL)) == -1)
        {
            perror("Error accepting connection");
            continue; // skip to accepting the next connection
        }

        pthread_t thread_id;

        // allocate space for worker thread's argument
        void* client_socket = malloc(sizeof(int));
        *(int*)client_socket = client_sockfd;

        pthread_create(&thread_id, NULL, handleClient, client_socket);
        // handleClient(client_sockfd);       

        pthread_detach(thread_id);
    }
    close(sockfd);
    return 0;
}

