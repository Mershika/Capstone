/*
============================================================
 File Name    : DirectoryTraverser.cpp
 Module       : Directory Tree Traversal Module
 Description  : Implements recursive directory traversal
                for the DCDIU file inspection system.

 Functionality:
  - Recursively traverse directory tree from root path
  - Identify files and subdirectories
  - Count total files encountered
  - Send real-time results to client over socket
  - Persist results to output file for later scanning
  - Handle access errors and permissions issues

 Algorithm:
  1. Open directory at basePath
  2. Iterate through all entries
  3. Skip "." (current) and ".." (parent) entries
  4. For each entry:
     - Get full path and file stats
     - If directory: recursively traverse
     - If regular file: count and output
  5. Close directory handle
  6. Handle errors gracefully

 Performance:
  - Depth-First Search (DFS) traversal
  - Linear O(n) where n = total entries
  - Memory depends on directory depth
  - I/O bound (reading filesystem metadata)

 Integration:
  Called by ClientHandler when:
  - TRAVERSE command received
  - SEARCH command needs file list
  Provides file list to ContentScanner for searching

============================================================
*/

#include <DirectoryTraverser/DirectoryTraverser.h>
#include <Logger/Logger.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cerrno>
#include <iostream>

using namespace std;

/*
================================================================
   Function Name : traverse()
   
   Description:
   Recursively traverses a directory tree, identifying
   all files and subdirectories. Sends results to client
   in real-time and writes to output file for later processing.
   
   Parameters:
   - basePath   : Root directory path to start traversal
   - client_fd  : Socket file descriptor to stream results to
   - fileCount  : Reference to counter (accumulates total files)
   - outputFile : Path to file where results are written
   
   Return Value:
   void (results sent via socket and file)
   
   Workflow:
   1. Open directory at basePath
   2. Open output file for appending
   3. Send directory info to client
   4. Read directory entries using readdir()
   5. For each entry:
      - Skip . and .. (navigation entries)
      - Use stat() to determine type
      - If directory: recursively call traverse()
      - If file: increment counter, output path
   6. Close directory and file handles
   
   Output Format:
   To Client (via socket):
   ├─ "Directory: /path/to/dir\n"
   └─ One "File: /path/to/file\n" per file
   
   To File (outputFile):
   ├─ One absolute path per line
   └─ Example:
      /home/user/doc.txt
      /home/user/pic.jpg
   
   Error Handling:
   - If directory cannot open: send error to client, throw
   - If output file cannot open: throw exception
   - If stat() fails on entry: skip and continue
   - If socket send fails: continue (best effort)
   
   Side Effects:
   - Sends data over network socket
   - Modifies fileCount (by reference)
   - Writes to outputFile (append mode)
   - Logs errors using Logger
   
   Recursion Depth:
   Limited by system stack and kernel depth limits.
   Deep directory trees may cause stack overflow.
   
================================================================
*/
void DirectoryTraverser::traverse(const string& basePath,
                                  int client_fd,
                                  int& fileCount,
                                  const string& outputFile) {

    try {

        /* ========================================
           STEP 1: Open Directory
           ======================================== */
        DIR* dir = opendir(basePath.c_str());

        if (!dir) {
            string errMsg = "ERROR: Cannot open directory: " + basePath + "\n";
            send(client_fd, errMsg.c_str(), errMsg.size(), 0);

            throw runtime_error("opendir failed: " + string(strerror(errno)));
        }

        /* ========================================
           STEP 2: Open Output File
           ======================================== */
        ofstream out(outputFile, ios::app);

        if (!out.is_open()) {
            closedir(dir);
            throw runtime_error("Failed to open output file: " + outputFile);
        }

        /* ========================================
           STEP 3: Notify Client of Directory
           ======================================== */
        string dirMsg = "Directory: " + basePath + "\n";

        if (send(client_fd, dirMsg.c_str(), dirMsg.size(), 0) < 0) {
            out.close();
            closedir(dir);
            throw runtime_error("send() failed while sending directory info");
        }

        struct dirent* entry;

        /* ========================================
           STEP 4: Iterate Through Directory Entries
           ======================================== */
        while ((entry = readdir(dir)) != nullptr) {

            /* Skip current (.) and parent (..) directories */
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
                continue;

            /* Construct full path */
            string fullPath = basePath + "/" + entry->d_name;

            /* Get file statistics */
            struct stat st;

            if (stat(fullPath.c_str(), &st) == -1) {
                perror("stat failed");
                continue;   /* Skip entries we can't stat */
            }

            /* ========================================
               STEP 5a: Handle Subdirectories
               RECURSIVELY TRAVERSE
               ======================================== */
            if (S_ISDIR(st.st_mode)) {

                traverse(fullPath, client_fd, fileCount, outputFile);
            }

            /* ========================================
               STEP 5b: Handle Regular Files
               COUNT AND OUTPUT
               ======================================== */
            else if (S_ISREG(st.st_mode)) {

                /* Increment file counter */
                fileCount++;

                /* Send file info to client */
                string fileMsg = "File: " + fullPath + "\n";

                if (send(client_fd,
                         fileMsg.c_str(),
                         fileMsg.size(),
                         0) < 0) {

                    perror("send failed");
                    continue;
                }

                /* Write file path to output file */
                if (!(out << fullPath << "\n")) {
                    throw runtime_error("Failed writing to output file");
                }
            }
        }

        /* ========================================
           STEP 6: Close Resources
           ======================================== */
        out.close();

        if (closedir(dir) == -1) {
            perror("closedir failed");
        }

    }
    catch (const exception& e) {

        /* Log fatal error */
        LOG(
            FATAL,
            "TRAVERSAL",
            e.what()
        );

        /* Notify client of error */
        string errMsg = "ERROR: ";
        errMsg += e.what();
        errMsg += "\n";

        send(client_fd, errMsg.c_str(), errMsg.size(), 0);
    }
}
