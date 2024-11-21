# TODO

- Handle different requests GET vs POST
- Parse url path parameters
- Parse query strings
- Serving static files
- Sending Large Files to a network device - phone to PC
- Stream a video between devices

#### Docker container ports

- Container runs inside a virtualized network
- Container ports are exposed by apps running inside a container, accessible only to things running within the container/other containers
- Container ports are isolated so need to explicitly bind ports from container to host machine

#### Previously, I setup container with a command to compile/run code upon startup. This was incorrect:

- For a development container, I should keep the container running and recompile code as needed, instead of restarting the container each time.
- Docker compose runs in the background. Compiling and running in the interactive shell works because it runs in the foreground with proper terminal attachment.
- Networking stack of the container is fully initialized when the command is run manually

#### Running the server

- Build image from dockerfile using "docker compose build"
- Run container from image using "docker compose up -d" (-d runs in detached mode, containers run in background, terminal is freed up)
  - Or both steps at once using "docker compose up --build -d"
- Run the interactive shell with "docker compose exec -it app bash"
- Compile and run with "gcc -o server server.cpp", "./server"


#### Docker for Development
- Dev container - using existing docker compose 
- Rebuild container to reflect changes to mounted volumes, restart container to test runtime changes
