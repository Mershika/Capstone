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


vector<string>
ContentScanner::scan(const string& inputFile,
                     const string& pattern) {

    vector<string> matches;

    try {

        // ==============================
        // Open File List
        // ==============================
        ifstream in(inputFile);

        if (!in.is_open()) {

            ExceptionHandler::handle("Cannot open file list");

            throw runtime_error(
                "Failed to open input file list: " + inputFile
            );
        }

        string filePath;
        char buffer[4096];

        // ==============================
        // Iterate Through File List
        // ==============================
        while (getline(in, filePath)) {

            // ------------------------------
            // Open Each File
            // ------------------------------
            int fd = open(filePath.c_str(), O_RDONLY);

            if (fd < 0) {
                // Non-fatal: skip and continue
                continue;
            }

            string content;
            ssize_t bytes;

            // ------------------------------
            // Read File Contents
            // ------------------------------
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

            // ------------------------------
            // Pattern Matching
            // ------------------------------
            if (content.find(pattern) != string::npos) {

                matches.push_back(filePath);
            }

            // ------------------------------
            // Close File Descriptor
            // ------------------------------
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

        // Return whatever was collected so far
    }

    return matches;
}