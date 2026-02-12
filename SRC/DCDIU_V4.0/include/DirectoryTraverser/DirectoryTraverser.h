/*
============================================================
 File Name    : DirectoryTraverser.h
 Module       : Directory Traversal Module
 Description  : Declares the DirectoryTraverser class
                responsible for performing recursive
                directory traversal operations on the
                server side.

 Responsibilities:
  - Perform recursive directory traversal
  - Identify and process files within directories
  - Maintain file count statistics
  - Stream traversal results to connected client
  - Optionally write results to an output file

============================================================
*/

#ifndef DIRECTORY_TRAVERSER_H
#define DIRECTORY_TRAVERSER_H

#include <string>

using namespace std;

/*
------------------------------------------------------------
 Class Name    : DirectoryTraverser
 Description   : Provides static utility functionality for
                recursively traversing directories starting
                from a specified base path.

                This class supports the remote directory
                inspection feature of the DCDIU system.
------------------------------------------------------------
*/
class DirectoryTraverser {

public:

    /*
     * --------------------------------------------------------
     * Function Name : traverse()
     * --------------------------------------------------------
     * Description   : Performs recursive traversal of the
     *                directory specified by basePath.
     *
     *                During traversal:
     *                  - Identifies files and subdirectories
     *                  - Updates file count
     *                  - Sends traversal results to client
     *                  - Writes results to output file (if required)
     *
     * @param basePath    Root directory path to start traversal.
     * @param client_fd   Client socket file descriptor used
     *                    to stream results.
     * @param fileCount   Reference variable to maintain total
     *                    number of files processed.
     * @param outputFile  Path to output file for storing
     *                    traversal results.
     *
     * Return             : void
     *
     * Note:
     * This method is static and does not require
     * instantiation of DirectoryTraverser.
     * --------------------------------------------------------
     */
    static void traverse(const string& basePath,
                         int client_fd,
                         int& fileCount,
                         const string& outputFile);
};

#endif  // DIRECTORY_TRAVERSER_H