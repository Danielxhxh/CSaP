
# Project in Computer Systems and Programming
#### Daniel Xhakalliu 2090065

This README.md serves as a guide, providing step-by-step instructions for the installation, compilation, and testing of the project developed by me.

### 1. Installation
Begin by extracting the contents of the ```tar``` file.

### 2. Project Structure
The extraction will unveil the following set of files:

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

### 3. Compilation
The Makefile simplifies the build process, helping create `client` and `server` executables from `client.c` and `server.c` source files.
To do so, we use the simple command 
```bash
make
```
inside the project folder.

### 4. Run the project

To run the project with default settings, use the following commands:

```bash
# To run the server 
./server

# To run the client
./client
```

For custom configurations, add command-line arguments:


```bash
# To run the server 
./server    FILEPATH      LISTENING_PORT 

# To run the client
./client    SERVER_IP_or_HOSTNAME  SERVER_PORT   
```