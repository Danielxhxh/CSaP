# Computer Systems and Programming Project

### Daniel Xhakalliu (Matriculation Number: 2090065)

The project involves creating a log server capable of accommodating an unlimited number of clients through stateful connections. Every client's activity is recorded in a designated log file, with additional mechanisms in place for efficient log file management. It's worth noting that both bonus points outlined in the specifications have been implemented.

This README.md serves as a guide, providing step-by-step instructions for the installation, compilation, and testing of the project developed by me.

### Project Structure

```bash
├── server.c
├── client.c
├── Makefile
├── README.md
├── logs
│   └── 'generated log files'
└── configurations
    ├── serverConfiguration.txt
    └── clientConfiguration.txt
```

- `server.c`: Source code file for the server.
- `client.c`: Source code file for the client.
- `Makefile`: Script for building and compiling the code.
- `README.md`: Documentation file.
- `logs`: Directory containing generated log files.
  - 'generated log files'
- `configurations`:
  - `serverConfiguration.txt`: Server configuration file.
  - `clientConfiguration.txt`: Client configuration file.

### 1. Installation

Begin by extracting the contents of the `tar` or `zip` file.

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

This will lunch the server with two clients, which will send one message each to the server. The messages will be written in the log file and all the running programms will be closed.
