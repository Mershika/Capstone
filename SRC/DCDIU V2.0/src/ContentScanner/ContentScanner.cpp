#include <ContentScanner/ContentScanner.h>
#include <ExceptionHandler/ExceptionHandler.h>

#include <fstream>
#include <fcntl.h>
#include <unistd.h>

using namespace std;


/*
 * File Name: ContentScanner.cpp
 * ============================================================
 *  Content Scanning Module
 * ============================================================
 *  Implements logic for scanning multiple files and
 *  detecting the presence of a given search pattern.
 *
 *  This module is responsible for:
 *   - Reading file paths from an input list
 *   - Opening and processing each file
 *   - Searching for specified content
 *   - Returning matched file paths
 * ============================================================
 */


/*
 * ============================================================
 *  Function Name : scan()
 * ============================================================
 *  Description   : Scans files listed in the input file and
 *                  identifies those containing the given
 *                  search pattern.
 *
 *  Input:
 *   - inputFile : File containing list of file paths
 *   - pattern   : Search string to be matched
 *
 *  Output:
 *   - Returns a vector containing paths of matched files
 *
 *  Workflow:
 *   1. Open input file containing file paths
 *   2. Read each file path line by line
 *   3. Open corresponding file in read-only mode
 *   4. Read file contents into memory
 *   5. Search for the given pattern
 *   6. Store matching paths
 *
 * ============================================================
 */
vector<string>
ContentScanner::scan(const string& inputFile,
                     const string& pattern) {

    /*
     * Container to store matched file paths.
     */
    vector<string> matches;

    /*
     * Open input file containing list of file paths.
     */
    ifstream in(inputFile);

    if (!in.is_open()) {

        /*
         * Handle file access failure.
         */
        ExceptionHandler::handle("Cannot open file list");
        return matches;
    }


    string filePath;
    char buffer[4096];


    /*
     * --------------------------------------------------------
     *  File Processing Loop
     * --------------------------------------------------------
     *  Iterates through each file path and scans its contents.
     * --------------------------------------------------------
     */
    while (getline(in, filePath)) {

        /*
         * Open target file in read-only mode.
         */
        int fd = open(filePath.c_str(), O_RDONLY);

        if (fd < 0)
            continue;


        string content;
        ssize_t bytes;


        /*
         * ----------------------------------------------------
         *  File Read Loop
         * ----------------------------------------------------
         *  Reads file content in chunks and appends it
         *  to a local buffer.
         * ----------------------------------------------------
         */
        while ((bytes = read(fd, buffer, sizeof(buffer))) > 0) {

            content.append(buffer, bytes);
        }


        /*
         * ----------------------------------------------------
         *  Pattern Matching
         * ----------------------------------------------------
         *  Checks whether the specified pattern exists
         *  in the current file content.
         * ----------------------------------------------------
         */
        if (content.find(pattern) != string::npos) {

            /*
             * Store matched file path.
             */
            matches.push_back(filePath);
        }


        /*
         * Close current file descriptor.
         */
        close(fd);
    }


    /*
     * Close input file stream.
     */
    in.close();


    /*
     * Return all matched file paths.
     */
    return matches;
}
