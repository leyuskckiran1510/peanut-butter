// this is copied from civetweb examples,
#include <stdio.h>
#include <string.h>
#include "civetweb.h"


#define MAX_SIZE 1024

// This function will be called by civetweb on every new request.
static int begin_request_handler(struct mg_connection *conn)
{
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    char content[MAX_SIZE];
    FILE *fp = fopen
    int content_length = snprintf(content, sizeof(content),
                                  "Hello from civetweb! Remote port: %d",
                                  request_info->remote_port);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              content_length, content);
    return 1;
}

int main(void)
{
    struct mg_context *ctx;
    struct mg_callbacks callbacks;


    const char *options[] = {"listening_ports", "8080", NULL};

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.begin_request = begin_request_handler;

    ctx = mg_start(&callbacks, NULL, options);
    getchar();

    mg_stop(ctx);

    return 0;
}
