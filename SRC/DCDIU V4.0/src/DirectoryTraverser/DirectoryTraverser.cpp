
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

void DirectoryTraverser::traverse(const string& basePath,
                                  int client_fd,
                                  int& fileCount,
                                  const string& outputFile) {

    try {

        // ==============================
        // Open Directory
        // ==============================
        DIR* dir = opendir(basePath.c_str());

        if (!dir) {
            string errMsg = "ERROR: Cannot open directory: " + basePath + "\n";
            send(client_fd, errMsg.c_str(), errMsg.size(), 0);

            throw runtime_error("opendir failed: " + string(strerror(errno)));
        }

        // ==============================
        // Open Output File
        // ==============================
        ofstream out(outputFile, ios::app);

        if (!out.is_open()) {
            closedir(dir);
            throw runtime_error("Failed to open output file: " + outputFile);
        }

        // ==============================
        // Send Directory Message
        // ==============================
        string dirMsg = "Directory: " + basePath + "\n";

        if (send(client_fd, dirMsg.c_str(), dirMsg.size(), 0) < 0) {
            out.close();
            closedir(dir);
            throw runtime_error("send() failed while sending directory info");
        }

        struct dirent* entry;

        // ==============================
        // Iterate Directory Entries
        // ==============================
        while ((entry = readdir(dir)) != nullptr) {

            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
                continue;

            string fullPath = basePath + "/" + entry->d_name;

            struct stat st;

            if (stat(fullPath.c_str(), &st) == -1) {
                perror("stat failed");
                continue;   // keep logic unchanged
            }

            // ==============================
            // If Subdirectory → Recursive Call
            // ==============================
            if (S_ISDIR(st.st_mode)) {

                traverse(fullPath, client_fd, fileCount, outputFile);
            }

            // ==============================
            // If Regular File
            // ==============================
            else if (S_ISREG(st.st_mode)) {

                fileCount++;

                string fileMsg = "File: " + fullPath + "\n";

                if (send(client_fd,
                         fileMsg.c_str(),
                         fileMsg.size(),
                         0) < 0) {

                    perror("send failed");
                    continue;
                }

                if (!(out << fullPath << "\n")) {
                    throw runtime_error("Failed writing to output file");
                }
            }
        }

        // ==============================
        // Close Resources
        // ==============================
        out.close();

        if (closedir(dir) == -1) {
            perror("closedir failed");
        }

    }
    catch (const exception& e) {

        LOG(
            FATAL,
            "TRAVERSAL",
            e.what()
        );

        string errMsg = "ERROR: ";
        errMsg += e.what();
        errMsg += "\n";

        send(client_fd, errMsg.c_str(), errMsg.size(), 0);
    }
}
