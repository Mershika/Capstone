#include <DirectoryTraverser/DirectoryTraverser.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <string>

using namespace std;


/*
 * ============================================================
 *  Directory Traversal Module
 * ============================================================
 *  Responsible for recursively scanning directories,
 *  identifying files, and sending results to the client.
 *
 *  This module performs:
 *   - Recursive directory traversal
 *   - File counting
 *   - Sending file/directory info to client
 *   - Storing file paths in output file
 * ============================================================
 */


/*
 * ============================================================
 *  Function Name : traverse()
 * ============================================================
 *  Description   : Recursively traverses a directory tree
 *                  and processes all files and subdirectories.
 *
 *  Input:
 *   - basePath   : Root directory to traverse
 *   - client_fd : Client socket descriptor
 *   - fileCount : Reference counter for files
 *   - outputFile: File used to store discovered paths
 *
 *  Output:
 *   - Sends traversal results to client
 *   - Updates fileCount
 *
 *  Workflow:
 *   1. Open target directory
 *   2. Send directory info to client
 *   3. Iterate through entries
 *   4. Skip special directories (. and ..)
 *   5. Identify file type using stat()
 *   6. Recursively traverse subdirectories
 *   7. Log regular files
 *
 * ============================================================
 */
void DirectoryTraverser::traverse(const string& basePath,
                                  int client_fd,
                                  int& fileCount,
                                  const string& outputFile) {

    /*
     * Open directory stream.
     */
    DIR* dir = opendir(basePath.c_str());

    if (!dir) {

        /*
         * Send error message if directory access fails.
         */
        string err = "ERROR: Cannot open directory\n";
        send(client_fd, err.c_str(), err.size(), 0);
        return;
    }


    /*
     * Open output file in append mode.
     */
    ofstream out(outputFile, ios::app);


    /*
     * Notify client about current directory.
     */
    string dirMsg = "Directory: " + basePath + "\n";
    send(client_fd, dirMsg.c_str(), dirMsg.size(), 0);


    struct dirent* entry;


    /*
     * --------------------------------------------------------
     *  Directory Entry Processing Loop
     * --------------------------------------------------------
     *  Iterates through each file and folder.
     * --------------------------------------------------------
     */
    while ((entry = readdir(dir)) != nullptr) {

        /*
         * Skip current and parent directory entries.
         */
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;


        /*
         * Construct absolute file path.
         */
        string fullPath = basePath + "/" + entry->d_name;


        struct stat st;

        /*
         * Retrieve file information.
         */
        if (stat(fullPath.c_str(), &st) == -1)
            continue;


        /*
         * ----------------------------------------------------
         *  Directory Handling
         * ----------------------------------------------------
         *  If entry is a directory, recursively traverse it.
         * ----------------------------------------------------
         */
        if (S_ISDIR(st.st_mode)) {

            traverse(fullPath, client_fd, fileCount, outputFile);
        }


        /*
         * ----------------------------------------------------
         *  Regular File Handling
         * ----------------------------------------------------
         *  If entry is a regular file, log and count it.
         * ----------------------------------------------------
         */
        else if (S_ISREG(st.st_mode)) {

            /*
             * Increment file counter.
             */
            fileCount++;


            /*
             * Send file info to client.
             */
            string fileMsg = "File: " + fullPath + "\n";
            send(client_fd, fileMsg.c_str(), fileMsg.size(), 0);


            /*
             * Store file path in output file.
             */
            out << fullPath << "\n";
        }
    }


    /*
     * Close output file stream.
     */
    out.close();


    /*
     * Close directory stream.
     */
    closedir(dir);
}
