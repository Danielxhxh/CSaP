# Project in Computer Systems and Programming

#### Daniel Xhakalliu 2090065

This README.md serves as a guide, providing step-by-step instructions for the installation, compilation, and testing of the project developed by me.

### Project Structure

```bash
├── server.c
├── client.c
├── Makefile
├── README.md
├── logs
│   └── 'log files'
└── configurations
    ├── serverConfiguration.txt
    └── clientConfiguration.txt
```

### 1. Installation

Begin by extracting the contents of the `tar` file.

### 2. Compilation

The Makefile simplifies the build process, helping create `client` and `server` executables from `client.c` and `server.c` source files.
To do so, we use the simple command

```bash
make
```

inside the project folder.

### 3. Run the project

To run the project with default settings, use the following commands:

```bash
# To run the server
./server

# To run the client
./client
```

For custom configurations, add command-line arguments:

For the filepath, either use an absolute (example: `/home/danux/Desktop/Project_CSaP/logs/`) or relative path (example: `./logs/`).

```bash
# To run the server
./server    FILEPATH      LISTENING_PORT

# To run the client
./client    SERVER_IP_or_HOSTNAME  SERVER_PORT
```

### 4. Testing

To run a simple test, just type the command:

```
./Test.sh
```

This will lunch the server with two clients, which send one message each to the server. The message will be written in the log file and all the running programms will be closed.
