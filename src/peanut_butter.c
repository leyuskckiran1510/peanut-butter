#include <stdio.h>
#include "peanut_butter.h"
#include <stdio.h>
#include <string.h>
#include "../include/civetweb.h"


#define MAX_SIZE 1024

void _pb_add_route(char* url, ViewCallback callback){
    Route route = {
        .url = url,
        .callback = callback
    };
    ROUTE_TABLE.routes[ROUTE_TABLE.count] = route;
    ROUTE_TABLE.count++;
}



static int begin_request_handler(struct mg_connection *conn){
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    for (size_t i = 0; i < ROUTE_TABLE.count; ++i){
        if(!strcmp(request_info->uri,ROUTE_TABLE.routes[i].url)){
            ROUTE_TABLE.routes[i].callback(request_info->request_method,conn);
            return 1;
        }
    }
    char content[MAX_SIZE];
    FILE *fp = fopen("htmls/404.html","r");
    int content_length = fread(content,sizeof(char),MAX_SIZE,fp);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              content_length, content);
    return 1;
}

void render_html(Request request,const char* file_name){
    char content[MAX_SIZE];
    FILE *fp = fopen(file_name,"r");
    int content_length = fread(content,sizeof(char),MAX_SIZE,fp);
    mg_printf(request,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %d\r\n"
              "\r\n"
              "%s",
              content_length, content);
}


int server_run(char *port){
    struct mg_context *ctx;
    struct mg_callbacks callbacks;
    const char *options[] = {"listening_ports", port, NULL};
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.begin_request = begin_request_handler;
    ctx = mg_start(&callbacks, NULL, options);
    getchar();
    mg_stop(ctx);
    return 0;
}
