

# Distributed Network File System


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


## Control Flow

1. **System Startup**
   - Start Naming Server first (./nm)
   - Start Storage Servers (./ss1, ./ss2)
   - Storage Servers register with Naming Server
   - Clients can then connect

2. **Client Operation Flow**
   a. Client Request:
      - Client sends command to Naming Server
      - Naming Server validates request
      - Naming Server identifies appropriate Storage Server
   
   b. File Operation:
      - Naming Server sends Storage Server details to Client
      - Client establishes direct connection with Storage Server
      - Storage Server performs requested operation
      - Results returned to Client
      - Connection closes

3. **Error Handling Flow**
   - Connection failures trigger 5-second timeout
   - Failed operations return error messages to Client
   - System maintains consistency through mutex locks

## Detailed Setup Guide


1. **System Requirements**

   - Linux/Unix-based system

   - GCC compiler

   - pthread library

   - Minimum 512MB RAM

   - Network connectivity between machines (if running distributed)


2. **Directory Structure Setup**

   ```bash

   mkdir test1 test2    # Create directories for storage servers

   touch test1/ss1.txt  # Create initial files if needed

   touch test2/ss2.txt
3. **Compilation Steps**
   
         -make all (or)
   
         -gcc nm.c -o nm -pthread
         
         -gcc test1/ss1.c -o ss1 -pthread
         
         -gcc test2/ss2.c -o ss2 -pthread
4. **Running the System**
   
         -./nm (Terminal 1 - Start Naming Server)
         -./ss1 (Terminal 2 - Start First Storage Server)
         -./ss2 (Terminal 3 - Start Second Storage Server)
         -./client (Terminal 4 - Start Client)

5. #**Verification Steps**
        -Check if Naming Server shows "Waiting for connections..."
        -Verify Storage Servers show successful registration
        -Test basic client commands:
   
               - CREATE test.txt -f
               - WRITE test.txt "Hello World"
               - READ test.txt

6. **Troubleshooting**
                 - Check if ports 8080, 8081, 8082 are available
                  - Ensure all components are running in correct order
                   -Verify network connectivity between components
                   -Check system logs for error messages



# Implementation Details

- Storage Server Registration
    - SS connects to NM on startup
    - Sends IP, port, and accessible paths
    - Maintains active connection for commands

- Client Operations
    - Connects to NM for initial request
    - Gets SS details for file operations
    - Direct connection to SS for data transfer

- File Operations Flow
    - Client sends command to NM
    - NM identifies appropriate SS
    - Client connects to SS directly
    - SS performs operation and sends response
    - Connection closes after operation

- Directory Management
    - Recursive directory creation/deletion
    - Path validation and management
    - Concurrent access control

# Testing

 - Start the naming server
 - Start multiple storage servers with different directories
 - Connect multiple clients
 - Test various file operations:
 - Create files and directories
 - Write content to files
 - Read file contents
 - Delete files and directories
 - Get file information

# Future Enhancements

 - File replication for redundancy
 - Load balancing across storage servers
 - Enhanced security features
 - Support for file locking mechanisms
 - Improved error recovery
 - Add support for IPv6 addressing
 - Health monitoring system







    



   

Link to project description: https://karthikv1392.github.io/cs3301_osn/project/


