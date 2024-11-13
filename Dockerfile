
FROM alpine:latest as base

# Install necessary packages (e.g., build-essential for C/C++ development)
RUN apk add --no-cache gcc g++ make libc-dev

# Change working directory to /app
# WORKDIR /app


