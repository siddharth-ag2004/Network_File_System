# Network File System (NFS)

A robust, distributed network file system implemented in C. The architecture consists of a centralized Naming Server, multiple distributed Storage Servers, and various Client interfaces that can perform concurrent file operations with high reliability and low latency.

## Architecture & Components

The system is split into three main components:

1. **Naming Server (`nms.c`)**
   - Acts as the central directory and orchestrator.
   - Manages metadata, tracks active storage servers, and routes client requests to the appropriate storage server.
   - Utilizes a Prefix Trie (`trie.c`) for efficient server-side search and directory structure management.
   - Employs an LRU Cache (`LRU.c`) to minimize lookup times for frequent requests.

2. **Storage Servers (`ss.c`)**
   - Responsible for the actual storage and retrieval of file data.
   - Support redundancy and fault tolerance by backing up data across multiple servers.
   - Handle direct file operations (read, write, create, delete, copy) with clients after the initial naming server lookup.

3. **Clients (`client.c`)**
   - Provide the interface for users to interact with the distributed file system.
   - Connect to the Naming Server to discover file locations and communicate directly with Storage Servers for data transfer.
   - Capable of concurrent access (tested with 10+ clients) using multi-threading.

## Key Features

- **Concurrent File Access:** Supports multi-threaded execution, allowing 10+ clients to access and modify files simultaneously.
- **Fault Tolerance & Redundancy:** Implements robust error handling and data replication across storage servers to prevent data loss.
- **Efficient Search & Caching:** Utilizes a Trie data structure for fast path resolutions and an LRU cache for O(1) repeated access lookups.
- **High Performance:** Capable of executing 1,000+ file operations (read/write/create/delete/copy) with an average response time of `<50 ms`.
- **Comprehensive Logging:** Dedicated logging system (`logging.c`) to track operations, errors, and system states.

## Setup and Usage

### Prerequisites
- **GCC compiler**
- **Make**
- **Linux/Unix environment (POSIX compliance)**

### Build Instructions
To compile the Naming Server, Storage Servers, and Client executables:

```bash
make all
```

*(Refer to the `Makefile` for specific build targets if you want to compile components individually).*

### Running the System

**1. Start the Naming Server:**
```bash
./nms
```

**2. Start the Storage Servers:**
Run multiple instances of the storage server. Each server will register itself with the Naming Server.
```bash
./ss <port> <storage_path>
```

**3. Run a Client:**
```bash
./client
```
Follow the interactive prompt to execute commands like `READ`, `WRITE`, `CREATE`, `DELETE`, and `COPY`.
