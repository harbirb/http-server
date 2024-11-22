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

void handleGet(char *reqBuffer, int client_sockfd)
{

    char resp[BUFFER_SIZE];
    memset(resp, 0, BUFFER_SIZE);
    // Status line
    strcpy(resp, "HTTP/1.1 200 OK\r\n");
    // optional headers
    strcat(resp, "Content-Type: text/html\r\n");
    // empty line
    strcat(resp, "\r\n");
    // response body
    strcat(resp, "Hello World: you sent a GET request");
    int bytes_written = send(client_sockfd, resp, sizeof(resp), 0);
    free(reqBuffer);
    if (bytes_written > 1)
    {
        // printf("\nSent response:\n%s\n", resp);
    }

    printf("END response\n");
}

void handlePost(char *reqBuffer)
{
}

// Given a request buffer, returns the method and URI
void parseRequest(char *reqBuffer)
{
    // use C std library to compare strings
    char *endOfMethod = strchr(reqBuffer, ' ');
    char *endOfURI = strchr(endOfMethod + 1, ' ');
    if (!endOfMethod || !endOfURI)
    {
        fprintf(stderr, "Invalid request line: missing spaces\n");
        return;
    }
    char method[256], URI[256];
    size_t methodLength = endOfMethod - reqBuffer;
    strncpy(method, reqBuffer, methodLength);
    method[methodLength] = '\0';
    printf("Method: %s\n", method);

    size_t URILength = endOfURI - (endOfMethod + 1);
    strncpy(URI, endOfMethod + 1, URILength);
    URI[URILength] = '\0';
    printf("URI: %s\n", URI);
}

void *handleClient(void *arg)
{
    int client_sockfd = *(int *)arg;

    // Read data from client_sockfd into buffer
    char *buf = (char *)malloc(BUFFER_SIZE);
    int bytes_read = recv(client_sockfd, buf, BUFFER_SIZE, 0);
    if (bytes_read > 0)
    {
        // null terminate and log the request buffer
        buf[bytes_read] = '\0';
        // TODO: call parseRequest, call appropriate method(args)
        parseRequest(buf);
        if (strncmp("GET", buf, 3) == 0)
        {
            // printf("GET received\n");
            handleGet(buf, client_sockfd);
        }
        else if (strncmp("POST", buf, 4) == 0)
        {
            printf("POST received");
            handlePost(buf);
        }
    }
    else
    {
        perror("Error reading fd");
    }
    close(client_sockfd);
    free(arg);
    return NULL;
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
    while (1)
    {
        // accept incoming connection and store client address info
        if ((client_sockfd = accept(sockfd, (struct sockaddr *)NULL, NULL)) == -1)
        {
            perror("Error accepting connection");
            continue; // skip to accepting the next connection
        }

        pthread_t thread_id;

        // allocate space for worker thread's argument
        void *client_socket = malloc(sizeof(int));
        *(int *)client_socket = client_sockfd;

        pthread_create(&thread_id, NULL, handleClient, client_socket);
        pthread_detach(thread_id);
    }
    close(sockfd);
    return 0;
}
