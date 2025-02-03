# Osn_final

# Distributed File System


A distributed network system that enables users to communicate and manage files across multiple storage servers through a centralized naming server.


## Team Members

1. Pratyush Jena

2. Saideekshith Vaddineni  

3. Swaroop C


## Architecture


The system consists of three main components:


1. **Naming Server (NM)**

   - Acts as the central coordinator

   - Maintains information about all storage servers and their files

   - Routes client requests to appropriate storage servers

   - Handles client authentication and access control

   - Listens on ports:

     - 8080 for Storage Server connections

     - 5566 for Client connections


2. **Storage Servers (SS)**

   - Store and manage actual files

   - Handle file operations (read, write, create, delete)

   - Communicate with naming server to register and update file information

   - Support concurrent client access through multi-threading

   - Default ports:

     - SS1: 8081

     - SS2: 8082


3. **Clients**

   - Connect to naming server to access files

   - Perform file operations through command interface

   - Communicate directly with storage servers for data transfer


## Features


### File Operations

- **READ**: Read contents from files. Command-READ <filename>
- **WRITE**: Write/Append content to files. Command-WRITE<filename> <content>
- **CREATE**: Create new files or directories. Command-CREATE <name> -f # For file CREATE <name> -d # For directory
- **DELETE**: Remove files or directories. Command-DELETE <name> -f # For file DELETE <name> -d # For directory
- **GETINFO**: Retrieve file metadata. Command-GETINFO <filename>


### System Features

- Multi-threaded architecture for concurrent client handling

- Automatic storage server registration

- Dynamic path management

- Error handling and timeout mechanisms

- Thread-safe operations using mutex locks

  ## Technical Implementation


### Core Components

1. **Network Communication**

   - Uses TCP sockets for reliable communication

   - Implements timeout handling (5 seconds)

   - Supports multiple concurrent connections


2. **Threading**

   - POSIX threads for concurrent request handling

   - Mutex locks for thread synchronization

   - Separate threads for client and server connections


3. **File Management**

   - Standard C file operations (fopen, fread, fwrite)

   - Directory operations using dirent.h

   - Recursive directory traversal

   - File permissions and metadata handling


### Setup and Installation


1. **Prerequisites**

   - GCC Compiler

   - POSIX-compliant system (Linux/Unix)

   - pthread library


2. **Compilation**

   ```bash

   #Compile Naming Server

   gcc nm.c -o nm -pthread


   # Compile Storage Servers

   gcc test1/ss1.c -o ss1 -pthread

   gcc test2/ss2.c -o ss2 -pthread


   # Compile Client

   gcc client.c -o client

# Start Naming Server first

./nm


# Start Storage Servers (in separate terminals)

./ss1

./ss2


# Start Client(s)

./client

# Implementation Details

    # Storage Server Registration
        SS connects to NM on startup
        Sends IP, port, and accessible paths
        Maintains active connection for commands

    Client Operations
        Connects to NM for initial request
        Gets SS details for file operations
        Direct connection to SS for data transfer

    File Operations Flow
        Client sends command to NM
        NM identifies appropriate SS
        Client connects to SS directly
        SS performs operation and sends response
        Connection closes after operation

    Directory Management
        Recursive directory creation/deletion
        Path validation and management
        Concurrent access control






    



   

Link to project description: https://karthikv1392.github.io/cs3301_osn/project/


