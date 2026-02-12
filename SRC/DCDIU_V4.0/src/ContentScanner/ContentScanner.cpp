/*
============================================================
 File Name    : ContentScanner.cpp
 Module       : Content Scanning and Pattern Matching
 Description  : Implements the ContentScanner utility class
                for searching files for specific patterns
                or strings.

 Functionality:
  - Read list of files from input file
  - Open each file individually
  - Search for pattern occurrences in content
  - Return list of matching files
  - Handle I/O errors gracefully

 Algorithm:
  1. Open input file containing list of file paths
  2. For each file path:
     a. Open file (skip if inaccessible)
     b. Read entire file into memory
     c. Search for pattern string
     d. If found, add to matches list
     e. Close file descriptor
  3. Return vector of matched file paths
  4. Log any errors encountered

 Performance Notes:
  - Reads entire files into memory (may be slow for huge files)
  - Linear search (O(n) where n = file size)
  - No indexing or caching

 Integration:
  Called by ClientHandler::handle() when SEARCH command received.
  Files to search come from DirectoryTraverser output.

============================================================
*/

#include <ContentScanner/ContentScanner.h>
#include <ExceptionHandler/ExceptionHandler.h>
#include <Logger/Logger.h>

#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cerrno>
#include <cstring>

using namespace std;

/*
================================================================
   Function Name : scan()
   
   Description:
   Searches a list of files for occurrences of a pattern string.
   
   Parameters:
   - inputFile : Path to file containing list of paths to search
   - pattern   : String pattern to search for
   
   Return Value:
   vector<string> - List of file paths where pattern was found
   
   Algorithm:
   1. Open input file (file list)
   2. Read each file path from input
   3. Open each file
   4. Read entire file content
   5. Check if pattern exists in content using string::find()
   6. If found, add to matches vector
   7. Return matches
   
   Error Handling:
   - If input file cannot open: exception thrown
   - If individual files cannot open: skipped silently
   - If read fails after file opened: exception thrown
   - Logs all fatal errors using Logger
   
   Input File Format:
   Each line contains one file path:
   /path/to/file1
   /path/to/file2
   /path/to/dir/file3
   
   Time Complexity:
   O(m * n) where:
   - m = number of files in list
   - n = average file size
   
   Space Complexity:
   O(m + n) where:
   - m = size of matches vector
   - n = size of largest file (buffered in memory)
   
================================================================
*/
vector<string>
ContentScanner::scan(const string& inputFile,
                     const string& pattern) {

    vector<string> matches;

    try {

        /* ===================================
           STEP 1: Open File List
           =================================== */
        ifstream in(inputFile);

        if (!in.is_open()) {

            ExceptionHandler::handle("Cannot open file list");

            throw runtime_error(
                "Failed to open input file list: " + inputFile
            );
        }

        string filePath;
        char buffer[4096];

        /* ===================================
           STEP 2: Iterate Through File Paths
           =================================== */
        while (getline(in, filePath)) {

            /* ===============================
               STEP 3: Open Individual File
               =============================== */
            int fd = open(filePath.c_str(), O_RDONLY);

            if (fd < 0) {
                /* Non-fatal: skip inaccessible files */
                continue;
            }

            string content;
            ssize_t bytes;

            /* ===============================
               STEP 4: Read File Contents
               =============================== */
            while ((bytes = read(fd, buffer, sizeof(buffer))) > 0) {

                content.append(buffer, bytes);
            }

            if (bytes < 0) {
                close(fd);
                throw runtime_error(
                    "Read failed for file: " + filePath +
                    " Error: " + strerror(errno)
                );
            }

            /* ===============================
               STEP 5: Search for Pattern
               =============================== */
            if (content.find(pattern) != string::npos) {

                /* Pattern found - add to results */
                matches.push_back(filePath);
            }

            /* ===============================
               STEP 6: Close File Descriptor
               =============================== */
            if (close(fd) < 0) {
                throw runtime_error(
                    "Failed to close file descriptor for: " + filePath
                );
            }
        }

        in.close();
    }
    catch (const exception& e) {

        LOG(
            FATAL,
            "SCANNER",
            e.what()
        );

        /* Return whatever was collected so far */
    }

    return matches;
}