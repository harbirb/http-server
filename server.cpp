#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>

int main()
{
    int sockfd, new_sockfd;
    struct addrinfo hints, *res;

    // specify hints for getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // For IPv4; use AF_INET6 for IPv6, or AF_UNSPEC for either
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE;     // Use the IP address of the local machine

    int status = getaddrinfo(NULL, "69", &hints, &res);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 1;
    }

    // create socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1)
    {
        perror("Error in creating socket");
        freeaddrinfo(res);
        return 1;
    }

    // bind socket to local machine addr at port 69
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) != 0)
    {
        perror("Error binding socket");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }

    if (listen(sockfd, 5) != 0)
    {
        perror("Error listening to socket");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }

    printf("Listening on port 69");

    struct sockaddr_in clientaddr;
    socklen_t client_len = sizeof(client_addr);

    while (true)
    {
        if ((new_sockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &client_len)) == -1)
        {
            perror("Error accepting connection");
            close(sockfd);
            return 1;
        }
    }

    printf("Connection accepted");

    close(sockfd);
    freeaddrinfo(res);
    return 0;
}