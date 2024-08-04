#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "peanut_butter.h"
#include <stdio.h>
#include <string.h>
#include "../include/civetweb.h"
#include "mime_map.h"

#define MAX_SIZE 1024
#define MAX_DIR_DEPTH 10

#define IS_SEP(x) ( ((x)=='/') || ((x)=='\\') )


void _pb_add_route(char* url, ViewCallback callback){
    Route route = {
        .url = url,
        .callback = callback
    };
    ROUTE_TABLE.routes[ROUTE_TABLE.count] = route;
    ROUTE_TABLE.count++;
}


char * virtual_path_traverse(const char *file_path){
    // to prevent file_traversal attakcs
    char *clean_path = malloc(strlen(file_path)+1);
    char *temp = clean_path;
    uint8_t seprators[MAX_DIR_DEPTH] ={0};
    uint8_t ptr = 0,sep_count=1,cpy_pointer;
    while(file_path[ptr]){
        // unix like systemfilepath    , windows file system
        if(IS_SEP(file_path[ptr]) && file_path[ptr+1]!='.'){
            // keep track of seprators for easier traverse back
            seprators[sep_count++] = cpy_pointer;
            //consume unnecessay '/' or '\'
            while(IS_SEP(file_path[ptr+1])){
                ptr++;
            }

        }else if(file_path[ptr] == '.' && file_path[ptr+1]=='.'){
            sep_count =  sep_count>1?sep_count-1:0;
            cpy_pointer =  seprators[sep_count];
            ptr+=2;//conmsue '../'
            while(IS_SEP(file_path[ptr+1])){
                ptr++;
            }

        }
        //consume single dots also
        else if (file_path[ptr]=='.' && IS_SEP(file_path[ptr+1]) ){
            ptr+=1;//consume './'
            while(IS_SEP(file_path[ptr+1])){
                ptr++;
            }
            
            ptr++;//increment pointer to skill '/'
            continue;
        }
        
        if(!cpy_pointer && (file_path[ptr]=='/' ||file_path[ptr+1]=='\\') ){
            ptr++;
            continue;
        }
        clean_path[cpy_pointer] = file_path[ptr];
        cpy_pointer++;
        clean_path[cpy_pointer] = 0;
        ptr++;
    }
    return clean_path;
}

const char * get_mime_type(const char *file_path){
    char *file_ext = malloc(10);
    uint8_t path_ptr = 0,ext_ptr=0;
    while(file_path[path_ptr]!='.' && file_path[path_ptr]) path_ptr++;
    path_ptr++;//consume '.'
    while(file_path[path_ptr] &&  ext_ptr<10)
        file_ext[ext_ptr++]=file_path[path_ptr++];
    MIME_TYPE_MATCH(file_ext);
}

static int serve_file(const char *file_path,Request request){
    char *path = virtual_path_traverse(file_path);
    const char *mime_type = get_mime_type(path);
    mg_send_mime_file(request,path,mime_type);
    free(path);
    return 0;
}


static int begin_request_handler(struct mg_connection *conn){
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    for (size_t i = 0; i < ROUTE_TABLE.count; ++i){
        if(!strcmp(request_info->uri,ROUTE_TABLE.routes[i].url)){
            ROUTE_TABLE.routes[i].callback(request_info->request_method,conn);
            return 1;
        }
    }
    if (!serve_file((request_info->uri+1),conn)) {
        return 1;
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
