#include <FileInspector/FileInspector.h>

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>


/*
 * ============================================================
 *  Communication Configuration
 * ============================================================
 *  Defines buffer size and protocol end-of-message marker
 *  used during file streaming to the client.
 * ============================================================
 */
#define BUFFER_SIZE 4096
#define END_MARK "<<END>>\n"



/*
 * ============================================================
 *  Inspect File
 * ============================================================
 *  Performs file-level inspection by reading file content
 *  and streaming it directly to the connected client.
 *
 *  Responsibilities:
 *   - Open file in read-only mode
 *   - Stream file content in chunks
 *   - Handle file access errors
 *   - Append protocol end marker after completion
 *
 *  This function supports the INSPECT command of
 *  the DCDIU server.
 * ============================================================
 */
void FileInspector::inspect(const std::string& filePath,
                            int client_fd) {

    /*
     * --------------------------------------------------------
     *  File Opening
     * --------------------------------------------------------
     */
    int fd = open(filePath.c_str(), O_RDONLY);

    /*
     * Handle file access failure.
     */
    if (fd < 0) {

        std::string err =
            "ERROR: Cannot open file\n" END_MARK;

        send(client_fd,
             err.c_str(),
             err.size(),
             0);

        return;
    }


    /*
     * --------------------------------------------------------
     *  File Streaming
     * --------------------------------------------------------
     *  Read file in chunks and send to client.
     */
    char buffer[BUFFER_SIZE];
    ssize_t bytes;

    while ((bytes = read(fd,
                         buffer,
                         BUFFER_SIZE)) > 0) {

        send(client_fd,
             buffer,
             bytes,
             0);
    }


    /*
     * --------------------------------------------------------
     *  Cleanup
     * --------------------------------------------------------
     */
    close(fd);

    /*
     * Send protocol end marker to indicate
     * completion of file transmission.
     */
    send(client_fd,
         END_MARK,
         strlen(END_MARK),
         0);
}