/*
============================================================
 File Name    : FileInspector.cpp
 Module       : File Content Streaming and Inspection
 Description  : Implements the FileInspector utility for
                reading and streaming file contents to
                remote clients over network sockets.

 Functionality:
  - Open files by path
  - Read file contents in chunks
  - Stream data to client via socket
  - Handle large files efficiently
  - Report errors gracefully

 Design:
  - Uses buffered reading (4096 byte chunks)
  - Streams output in real-time (no full buffering)
  - Suitable for large files
  - Includes end-of-transmission marker

 Integration:
  Called by ClientHandler when INSPECT command received.
  Allows clients to remotely view file contents.

 Limitations:
  - Only reads regular files (not directories)
  - Reads files as raw bytes (binary-unsafe)
  - No encoding conversion (UTF-8, etc.)
  - No line numbering or formatting

============================================================
*/

#include <FileInspector/FileInspector.h>
#include <Logger/Logger.h>

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <stdexcept>
#include <cerrno>

#define BUFFER_SIZE 4096
#define END_MARK "<<END>>\n"

/*
================================================================
   Function Name : inspect()
   
   Description:
   Opens a file and streams its entire contents to the
   connected client via socket in binary format.
   
   Workflow:
   1. Attempt to open specified file
   2. If successful:
      a. Read file in BUFFER_SIZE chunks
      b. Send each chunk immediately to client
      c. Close file descriptor
      d. Send end-of-file marker
   3. If error at any step:
      a. Log error with context
      b. Send error message to client
      c. Send end-of-file marker
   
   Parameters:
   - filePath  : Absolute or relative path to file to read
   - client_fd : Socket file descriptor of connected client
   
   Return Value:
   void (sends data via socket)
   
   Network Protocol:
   ────────────────
   1. File chunks sent immediately upon reading
   2. No content-length header
   3. Transmission ends with "<<END>>\n" marker
   4. Client must read until seeing end marker
   
   Example Sequence:
   ─────────────────
   Server → Client: [first 4096 bytes of file]
   Server → Client: [next 4096 bytes]
   ...
   Server → Client: [remaining bytes < 4096]
   Server → Client: <<END>>\n
   
   Error Handling:
   ───────────────
   If file cannot open:
   ├─ Send: "ERROR: Cannot open file\n<<END>>\n"
   └─ Log FATAL error to system logger
   
   If read fails:
   ├─ Close file descriptor
   ├─ Send ERROR message
   └─ Log FATAL error
   
   If socket send fails:
   ├─ Close file descriptor  
   ├─ Log FATAL error
   └─ Throw exception
   
   Memory Usage:
   ─────────────
   Fixed buffer of BUFFER_SIZE (4096 bytes)
   Suitable for streaming large files
   No full file buffering
   
   File Size Limitations:
   ──────────────────────
   - Maximum file size: System dependent
   - Limited by available disk space
   - Network timeout may interrupt large files
   
   Performance Characteristics:
   ───────────────────────────
   - I/O bound operation
   - CPU usage minimal (mostly I/O wait)
   - Network bandwidth is limiting factor
   - Time complexity: O(n) where n = file size
   - Space complexity: O(1) constant buffer
   
================================================================
*/
void FileInspector::inspect(const std::string& filePath,
                            int client_fd) {

    try {

        /* ==========================================
           STEP 1: Open File for Reading
           ========================================== */
        int fd = open(filePath.c_str(), O_RDONLY);

        if (fd < 0) {

            /* File could not be opened */
            std::string err =
                "ERROR: Cannot open file\n" END_MARK;

            send(client_fd,
                 err.c_str(),
                 err.size(),
                 0);

            throw std::runtime_error(
                "Failed to open file: " + filePath +
                " Error: " + std::strerror(errno)
            );
        }

        /* ==========================================
           STEP 2: Stream File Contents to Client
           ========================================== */
        char buffer[BUFFER_SIZE];
        ssize_t bytes;

        /* Read file in chunks and send immediately */
        while ((bytes = read(fd,
                             buffer,
                             BUFFER_SIZE)) > 0) {

            /* Send chunk to client */
            if (send(client_fd,
                     buffer,
                     bytes,
                     0) < 0) {

                close(fd);

                throw std::runtime_error(
                    "send() failed while streaming file: " + filePath
                );
            }
        }

        /* Check for read errors */
        if (bytes < 0) {

            close(fd);

            throw std::runtime_error(
                "read() failed for file: " + filePath +
                " Error: " + std::strerror(errno)
            );
        }

        /* ==========================================
           STEP 3: Close File Descriptor
           ========================================== */
        if (close(fd) < 0) {

            throw std::runtime_error(
                "Failed to close file descriptor for: " + filePath
            );
        }

        /* ==========================================
           STEP 4: Send End-of-Transmission Marker
           ========================================== */
        if (send(client_fd,
                 END_MARK,
                 strlen(END_MARK),
                 0) < 0) {

            throw std::runtime_error(
                "Failed to send END_MARK for file: " + filePath
            );
        }
    }
    catch (const std::exception& e) {

        /* Log any exceptions that occurred */
        LOG(
            FATAL,
            "INSPECT",
            e.what()
        );
    }
}
