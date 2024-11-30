# A HTTP server from scratch in C

### What + why:
I implemented a basic web server that serves files over HTTP. Although basic, it was a great exercise to "learn by building" and get a better understanding of server-client interactions.

### Features:
- **Sockets**: Creates and binds a socket to an address and port. It listens and accepts incoming connections
- **Multi-threading**: Each client connection is handled in a separate thread
- **Basic Request Parsing**: Extracts HTTP method and URI from the request
- **MIME Type Handling**: Maps file extensions to MIME types using a linear search of an array
- **File Serving for GET Requests**: Reads files from a ./public directory and sends them as responses
- 

### Things that I learned:
- **Sockets**: Provide a communication interface between processes over a network using the TCP/IP stack
- **TCP**: Transmission Control Protocol - a reliable, ordered, and error-checked delivery of a stream of bytes
  - whereas UDP is a fire-and-forget transmission, doesn't check for packet delivery
- **Binding**: Associates a socket with a specific IP address and port, enabling the server to accept incoming connections on that endpoint
- **Multithreading**: Each client connection is handled in a separate thread, enabling the server to support multiple clients concurrently
- **Resource Management**: Importance of cleaning up resources (closing sockets, freeing memory) after use
- **Request Parsing**: Learned to parse HTTP request lines to extract the method (GET/POST) and URI
- **Response Headers**: Built and sent HTTP headers with status codes, content types, and lengths
- **Manpages**: Utilized manual pages to understand system call arguments, return values, and error handling
- **Directory Traversal Attacks**: Learned the risk of malicious users exploiting URIs to access files (../ sequences)
Usefulness of Docker
- **Portability**: For sharing and running applications across different system (UNIX sys calls on a Windows machine)
- **Development Environments**: Dev Containers set up reproducible and development environments.
