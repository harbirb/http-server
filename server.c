#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>

int sockfd, client_sockfd;
int PORT = 8080;
int BUFFER_SIZE = 4096;

typedef struct requestLine
{
    char method[256];
    char URI[256];
} requestLine;

// Map from file type to MIME type
typedef struct mimes
{
    char *ext;
    char *mime_type;
} mimes;

mimes mimes_map[] = {
    {"html", "text/html"},
    {"htm", "text/html"},
    {"css", "text/css"},
    {"js", "application/javascript"},
    {"json", "application/json"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"png", "image/png"},
    {"gif", "image/gif"},
    {"svg", "image/svg+xml"},
    {"ico", "image/x-icon"},
    {"txt", "text/plain"},
    {"xml", "application/xml"},
    {"pdf", "application/pdf"},
    {"zip", "application/zip"},
    {"mp4", "video/mp4"},
    {"mp3", "audio/mpeg"},
    {NULL, NULL}};

// get the file ext after the '.' (eg. jpg, png, html)
char *get_file_extension(char *uri)
{
    char *dot = strrchr(uri, '.');
    if (dot == NULL)
    {
        return "";
    }
    return dot + 1;
}

// LINEAR SEARCH over MIME map
// TODO: Implement a Hashmap
char *get_mime_type(char *uri)
{
    char *file_ext = get_file_extension(uri);
    for (size_t i = 0; mimes_map[i].ext != NULL; i++)
    {
        if (strcmp(mimes_map[i].ext, file_ext) == 0)
        {
            return mimes_map[i].mime_type;
        }
    }
    // Return generic binary data (true type unknown)
    return "text";
}

void handleGet(int client_sockfd, char *uri)
{
    // access public files using ./public/filename (uri='/filename')
    // TODO: VULNERABLE TO DIRECTORY TRAVERSAL ATTACK
    char sanitized_uri[256];
    snprintf(sanitized_uri, sizeof(sanitized_uri), "./public%s", uri);
    if (strcmp(uri, "/") == 0)
    {
        strcpy(sanitized_uri, "./public/index.html");
    }
    int file_fd = open(sanitized_uri, O_RDONLY);
    if (file_fd < 0)
    {
        perror("failed to find resource");
        // TODO: 404 not found page, when file does not exist
        // file_fd = open("404page.......")
        char *resp = "HTTP/1.1 404 Not Found\r\n\r\nNOT FOUND!!\r\n";
        send(client_sockfd, resp, strlen(resp), 0);
        return;
    }

    char *mime_type = get_mime_type(uri);
    char header[BUFFER_SIZE];
    memset(header, 0, BUFFER_SIZE);
    // content-length is optional when specifying Connection: close OR Transfer-encoding: chunked
    snprintf(header, BUFFER_SIZE,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             mime_type);

    // send headers
    send(client_sockfd, header, strlen(header), 0);

    // send body
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0)
    {
        ssize_t bytes_sent = send(client_sockfd, buffer, bytes_read, 0);
    }
    close(file_fd);
}

void handlePost(char *reqBuffer)
{
}

// Given a request buffer, returns the method and URI
void parseRequest(char *reqBuffer, requestLine *reqLine)
{
    char *endOfMethod = strchr(reqBuffer, ' ');
    char *endOfURI = strchr(endOfMethod + 1, ' ');
    if (!endOfMethod || !endOfURI)
    {
        fprintf(stderr, "Invalid request line: missing spaces\n");
        return;
    }
    size_t methodLength = endOfMethod - reqBuffer;
    strncpy(reqLine->method, reqBuffer, methodLength);
    reqLine->method[methodLength] = '\0';
    printf("Method: %s\n", reqLine->method);

    size_t URILength = endOfURI - (endOfMethod + 1);
    strncpy(reqLine->URI, endOfMethod + 1, URILength);
    reqLine->URI[URILength] = '\0';
    printf("URI: %s\n", reqLine->URI);
}

void *handleClient(void *arg)
{
    int client_sockfd = *(int *)arg;
    // Read data from client_sockfd into buffer
    char *buf = (char *)malloc(BUFFER_SIZE);
    int bytes_read = recv(client_sockfd, buf, BUFFER_SIZE, 0);
    if (bytes_read > 0)
    {
        buf[bytes_read] = '\0';
        requestLine reqLine;
        parseRequest(buf, &reqLine);
        if (strcmp(reqLine.method, "GET") == 0)
        {
            printf("GET received\n");
            handleGet(client_sockfd, reqLine.URI);
        }
        else if (strcmp(reqLine.method, "POST") == 0)
        {
            printf("POST received\n");
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

    // Set socket option SO_REUSEADDR
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt failed");
        close(sockfd);
        exit(EXIT_FAILURE);
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
