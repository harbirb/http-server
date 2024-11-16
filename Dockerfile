
FROM gcc:latest

WORKDIR /app

# Install basic development tools
RUN apt-get update && apt-get install -y \
    gdb \
    make \
    vim \
    && rm -rf /var/lib/apt/lists/*

# Keep container running
CMD ["tail", "-f", "/dev/null"]

