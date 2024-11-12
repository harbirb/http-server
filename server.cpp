#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

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

    // make the socket listen, with a queue of 5
    if (listen(sockfd, 5) != 0)
    {
        perror("Error listening to socket");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }

    printf("Listening on port 69");

    struct sockaddr_in clientaddr;
    socklen_t client_len = sizeof(clientaddr);

    while (true)
    {
        // accept incoming connection and store client address info
        if ((new_sockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &client_len)) == -1)
        {
            perror("Error accepting connection");
            continue;
        }

        // Read data from new_sockfd into buffer, null terminate, then print out
        char buf[4096];
        int bytes_read = read(new_sockfd, buf, 4095);
        if (bytes_read < 0)
        {
            perror("Error reading fd")
        }
        else
        {
            buf[bytes_read] = '\0';
            printf("\n%s\n", buf);
        }

        close(new_sockfd);
    }

    close(sockfd);
    freeaddrinfo(res);
    return 0;
}