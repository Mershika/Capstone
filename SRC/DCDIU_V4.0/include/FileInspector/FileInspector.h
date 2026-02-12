/*
============================================================
 File Name    : FileInspector.h
 Module       : File Inspection Module
 Description  : Declares the FileInspector class responsible
                for analyzing and processing individual files
                within the DCDIU system.

 Responsibilities:
  - Inspect individual files during traversal
  - Extract relevant file-level information
  - Stream inspection results to connected client
  - Support deep inspection workflow of DCDIU

============================================================
*/

#ifndef FILE_INSPECTOR_H
#define FILE_INSPECTOR_H

#include <string>

/*
------------------------------------------------------------
 Class Name    : FileInspector
 Description   : Provides static utility functionality for
                inspecting individual files identified
                during directory traversal.

                This class acts as a file-level processing
                component within the DCDIU architecture.
------------------------------------------------------------
*/
class FileInspector {

public:

    /*
     * --------------------------------------------------------
     * Function Name : inspect()
     * --------------------------------------------------------
     * Description   : Performs inspection on the specified
     *                file and streams the results to the
     *                connected client.
     *
     *                This may include:
     *                  - Validating file accessibility
     *                  - Extracting metadata
     *                  - Sending file details through socket
     *
     * @param filePath   Absolute or relative path of the file
     *                   to be inspected.
     * @param client_fd  Client socket file descriptor used
     *                   to transmit inspection results.
     *
     * Return            : void
     *
     * Note:
     * This method is static and does not require
     * instantiation of FileInspector.
     * --------------------------------------------------------
     */
    static void inspect(const std::string& filePath,
                        int client_fd);
};

#endif  // FILE_INSPECTOR_H