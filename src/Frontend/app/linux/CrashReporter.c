/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <libgen.h>

#define MAX_RCV_LEN 2048
#define HTTP_BOUNDARY "----------------------------23639bb5ee29\r\n"
#define HTTP_BOUNDARY_END "\r\n------------------------------23639bb5ee29--\r\n"

#define FORGE(msg, d)\
    sprintf(d, "%s", msg);\
    d = d + strlen(msg);

#define SEND(msg) \
    send(sock, msg, strlen(msg), 0);

char *read_dump(const char *path, int *data_len)
{
    FILE *fd;
    size_t filesize;
    size_t readsize;
    char *data;

    fd = fopen(path, "rb");
    if (!fd) {
        fprintf(stderr, "Failed to open dump : %s\n", path);
        return NULL;
    }

    fseek(fd, 0L, SEEK_END);
    filesize = ftell(fd);
    fseek(fd, 0L, SEEK_SET);

    *data_len = filesize;
    data = (char *)malloc(filesize + 1);

    readsize = fread(data, 1, filesize, fd);
    data[readsize] = '\0';

    fclose(fd);

    return data;
}

int main(int argc, char **argv)
{
    char reply_buffer[MAX_RCV_LEN + 1];
    int minidum_size;
    char *minidump;
    char data[2048];
    char *data_ptr = &data[0];
    struct sockaddr_in dest;
    struct hostent *hostaddr;
    int sock;

    if (argc < 2) {
        fprintf(stderr, "No dump specified\n");
        return -1;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Failed to create socket, err=%d\n", errno);
        return -2;
    }

    if ((hostaddr = gethostbyname(NIDIUM_CRASH_COLLECTOR_HOST)) == NULL) {
        fprintf(stderr, "Unable to get ip of %s host\n", NIDIUM_CRASH_COLLECTOR_HOST);
        return -2;
    }

    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = ((struct in_addr *)hostaddr->h_addr_list[0])->s_addr;
    dest.sin_port = htons(NIDIUM_CRASH_COLLECTOR_PORT);

    if (connect(sock, static_cast<const struct sockaddr *>(&dest), sizeof(struct sockaddr)) != 0) {
        fprintf(stderr, "Failed to connect\n");
        return -2;
    }

    if ((minidump = read_dump(argv[1], &minidum_size)) == NULL) {
        return -3;
    }

    char cd_minidump[2048];
    snprintf(cd_minidump, 2048, "Content-Disposition: form-data; name=\"minidump\"; filename=\"%s\"\r\n", basename(argv[1]));

    // Forge the header (needed to get the actual content length)
    FORGE("\r\n", data_ptr);
    FORGE("--"HTTP_BOUNDARY, data_ptr);
    FORGE("Content-Disposition: form-data; name=\"product\"\r\n\r\n", data_ptr);
    FORGE("Nidium\r\n", data_ptr);
    FORGE("--"HTTP_BOUNDARY, data_ptr);
    FORGE("Content-Disposition: form-data; name=\"build\"\r\n\r\n", data_ptr);
    FORGE(NIDIUM_BUILD"\r\n", data_ptr);
    FORGE("--"HTTP_BOUNDARY, data_ptr);
    FORGE("Content-Disposition: form-data; name=\"version\"\r\n\r\n", data_ptr);
    FORGE(NIDIUM_VERSION_STR"\r\n", data_ptr);
    FORGE("--"HTTP_BOUNDARY, data_ptr);
    FORGE(cd_minidump, data_ptr);
    FORGE("Content-Type: application/octet-stream\r\n\r\n", data_ptr);

    // Forge the content-length header
    char cl_header[64];
    sprintf(cl_header, "Content-Length:%d\r\n", strlen(data) + strlen(HTTP_BOUNDARY_END) + minidum_size);
    // Send the data
    SEND("POST "NIDIUM_CRASH_COLLECTOR_ENDPOINT" HTTP/1.1\r\n");
    SEND("User-Agent: Nidium crash reporter V0.1\r\n");
    SEND("Host: "NIDIUM_CRASH_COLLECTOR_HOST"\r\n");
    SEND(cl_header);
    SEND("Content-Type: multipart/form-data; boundary="HTTP_BOUNDARY"\r\n\r\n");
    //fprintf(stderr, "CrashData=%s\n", data);
    SEND(data);
    send(sock, minidump, minidum_size, 0);
    SEND(HTTP_BOUNDARY_END)

    int len;
    len = recv(sock, reply_buffer, MAX_RCV_LEN, 0);
    reply_buffer[len] = '\0';

    fprintf(stderr, "reply (%d) %s\n", len, reply_buffer);

    close(sock);
    free(minidump);
    //unlink(argv[1]);

    return 0;
}

