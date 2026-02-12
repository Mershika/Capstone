/*
============================================================
 File Name    : ContentScanner.h
 Module       : Content Scanning Module
 Description  : Declares the ContentScanner class responsible
                for scanning file contents to detect specific
                strings, phrases, or patterns.

 Responsibilities:
  - Perform deep-content inspection on files
  - Search for user-specified patterns
  - Return matching lines or results
  - Support server-side search operations

============================================================
*/

#ifndef CONTENT_SCANNER_H
#define CONTENT_SCANNER_H

#include <string>
#include <vector>

using namespace std;

/*
------------------------------------------------------------
 Class Name    : ContentScanner
 Description   : Provides static utility functionality for
                scanning file contents and identifying
                occurrences of specified patterns.
                
                This class supports the deep-content
                discovery feature of the DCDIU system.
------------------------------------------------------------
*/
class ContentScanner {

public:

    /*
     * --------------------------------------------------------
     * Function Name : scan()
     * --------------------------------------------------------
     * Description   : Scans the specified input file for
     *                occurrences of the given pattern.
     *
     *                Performs line-by-line inspection and
     *                collects all matching results.
     *
     * @param inputFile  Path to the file to be scanned.
     * @param pattern    String or keyword to search for.
     *
     * @return vector<string>
     *         A list containing matched lines or results
     *         where the pattern is detected.
     *
     * Note:
     * This method is static and can be invoked without
     * creating an instance of ContentScanner.
     * --------------------------------------------------------
     */
    static vector<string>
    scan(const string& inputFile,
         const string& pattern);
};

#endif  // CONTENT_SCANNER_H
