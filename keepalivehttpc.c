#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/tcp.h>

#define BUFFERLEN 1024

int usage() {
    printf("usage: keepalivehttpc [-I] <url>\n");
    return 0;
}

int main (int argc, char** argv) {
    if (argc == 1) {
        return usage();
    }
    char *url = NULL;
    char *showRequest = NULL;

    int i;
    for (i = 1; i < argc; ++i) {
        // Don't need to check length, if argv[i][0] is '\0' then
        // we won't carry on anyway
        if (argv[i][0] == '-' && argv[i][1] == 'I') {
            showRequest = argv[i];
        } else if (!url) {
            url = argv[i];
        }
    }

    char *scheme = NULL;
    char *hostname = NULL;
    char *path = NULL;
    int port = 0;
    char *urlPort = NULL;
    char last = '\0';
    char *current = url;
    char *sectionStart = url;
    int len;
    while (1) {
        if (scheme == NULL && *current == '/' && last == '/') {
            len = current - sectionStart +2;
            scheme = malloc(sizeof(char)*len);
            strncpy(scheme,sectionStart,len);
            scheme[len-1] = '\0';
            sectionStart = current + 1;
        } else if (scheme && hostname == NULL && ( *current == ':' || *current == '/' || *current == '\0' ) ) {
            len = current - sectionStart + 1;
            hostname = malloc(sizeof(char)*len);
            strncpy(hostname,sectionStart,len);
            hostname[len-1] = '\0';

            if (*current == ':') {
                sectionStart = current+1;
                urlPort = current+1;
            }
        }

        if (hostname && (*current == '/' || *current == '\0') ) {
            if (urlPort) {
                len = current - sectionStart + 1;
                if (len > 1) {
                    urlPort = malloc(sizeof(char)*len);
                    strncpy(urlPort,sectionStart,len);
                    urlPort[len-1] = '\0';
                } else {
                    urlPort = NULL;
                }
            }

            if (*current != '\0') {
                len = strlen(current)+1;
                if (len>1) {
                    path = malloc(sizeof(char)*len);
                    strncpy(path,current,len);
                    path[len-1] = '\0';
                }
            }

            break;
        }
        last = *current;
        if (*current == '\0') {
            break;
        }
        ++current;
    }

    if (urlPort) {
        port = atoi(urlPort);
        if (port == 0) {
            port = 80;
        }
    }
    if (path == NULL || path[0] == '\0') {
        path = "/";
    }
    if (hostname == NULL || hostname[0] == '\0') {
        return usage();
    }
    int httpSock = clientsock(hostname, 80);

    int optval;
    socklen_t optlen = sizeof(optval);

    if(getsockopt(httpSock, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0) {
        perror("getsockopt()");
        close(httpSock);
        return 1;
    }
    //printf("SO_KEEPALIVE is %s\n", (optval ? "ON" : "OFF"));

    /* Set the option active */
    optval = 1;
    optlen = sizeof(optval);
    if(setsockopt(httpSock, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        perror("setsockopt()");
        close(httpSock);
        return 1;
    }
    //printf("SO_KEEPALIVE set on socket\n");

    /* Check the status again */
    if(getsockopt(httpSock, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen) < 0) {
        perror("getsockopt()");
        close(httpSock);
        return 1;
    }
    //printf("SO_KEEPALIVE is %s\n", (optval ? "ON" : "OFF"));

    char *stringRequestFormat = "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: keepalivehttpc/1.0\r\n\r\n";
    int requestLen = strlen(stringRequestFormat);
    requestLen += strlen(path) + strlen(hostname);
    
    // Will be a bit too long as it includes the format characters too, that will allow for the null character at least.
    char stringRequest[requestLen];
    snprintf(stringRequest,requestLen,stringRequestFormat, path, hostname);

    if (showRequest) {
        printf("%s\n", stringRequest);
    }
    send(httpSock, stringRequest, strlen(stringRequest), 0);

    char buf[BUFFERLEN];
    int bufsize = read( httpSock, buf, BUFFERLEN);
    buf[bufsize] = '\0';
    printf("%s\n", buf);

    return 0;
}
