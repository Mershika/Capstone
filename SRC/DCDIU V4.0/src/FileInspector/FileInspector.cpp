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

void FileInspector::inspect(const std::string& filePath,
                            int client_fd) {

    try {

        // ==============================
        // Open File
        // ==============================
        int fd = open(filePath.c_str(), O_RDONLY);

        if (fd < 0) {

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

        // ==============================
        // Stream File Contents
        // ==============================
        char buffer[BUFFER_SIZE];
        ssize_t bytes;

        while ((bytes = read(fd,
                             buffer,
                             BUFFER_SIZE)) > 0) {

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

        if (bytes < 0) {

            close(fd);

            throw std::runtime_error(
                "read() failed for file: " + filePath +
                " Error: " + std::strerror(errno)
            );
        }

        // ==============================
        // Close File
        // ==============================
        if (close(fd) < 0) {

            throw std::runtime_error(
                "Failed to close file descriptor for: " + filePath
            );
        }

        // ==============================
        // Send End Marker
        // ==============================
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

        LOG(
            FATAL,
            "INSPECT",
            e.what()
        );
    }
}
