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
#define URL_ARG_STRING_INCR_RATE 10
#define MAX_READ_CHUNK 1024*3                  // 3kb

#define IS_SEP(x) ( ((x)=='/') || ((x)=='\\') )


void PB(add_route(char* url, ViewCallback callback)){
    log_debug("Added Route '%s' ",url);
    Route route = {
        .url = url,
        .callback = callback
    };
    ROUTE_TABLE.routes[ROUTE_TABLE.count] = route;
    ROUTE_TABLE.count++;
}

void PB(add_var_route(char*  var_route,ViewCallbackArgs callback)){
    log_debug("Added Route '%s' ",var_route);
    Route route = {
        .url = var_route,
        .callbackargs = callback
    };
    VAR_ROUTE_TABLE.routes[VAR_ROUTE_TABLE.count] = route;
    VAR_ROUTE_TABLE.count++;
}


char * virtual_path_traverse(const char *file_path){
    
    // to prevent file_traversal attacks
    char *clean_path = malloc(strlen(file_path)+1);
    uint8_t seprators[MAX_DIR_DEPTH] ={0};
    uint8_t ptr = 0,sep_count=1,cpy_pointer=0;
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

const char *get_mime_type(const char *file_path){
    char *file_ext = malloc(10);
    uint8_t path_ptr = 0,ext_ptr=0;
    while(file_path[path_ptr]!='.' && file_path[path_ptr]) path_ptr++;
    path_ptr++;//consume '.'
    while(file_path[path_ptr] &&  ext_ptr<10)
        file_ext[ext_ptr++]=file_path[path_ptr++];
    file_ext[ext_ptr]=0;
    MIME_TYPE_MATCH(file_ext);
}

static int serve_file(Request request,const char *file_path){
    char *path = virtual_path_traverse(file_path);
    FILE *fp = fopen(path,"r");
    if(fp==NULL){
        return 1;
    }
    fclose(fp);

    const char *mime_type = get_mime_type(path);
    log_debug("Serving File... %s [%s]",path,mime_type);
    mg_send_mime_file(request,path,mime_type);
    free(path);
    return 0;
}

int consume_digit(const char *str,UrlVariable *arg){
    int count = 0;
    arg->value = 0;
    arg->type = UAT_INTEGER;
    while(str[count]>='0' && str[count]<='9'){
        arg->value = arg->value * 10 + str[count]-'0';
        count++;
    }
    return count;
}
int consume_char(const char *str,UrlVariable *arg){
    arg->value = str[0];
    arg->type = UAT_CHARACTER;
    return 1;
}

int print_able_range(char letter){
    // printable character range
    // SPACE to ~
    return 32<=letter && letter<=126;
}

int consume_str(const char *str,UrlVariable *arg){
    char *new_str = malloc(URL_ARG_STRING_INCR_RATE);
    int count = 0,resized=0;

    arg->type = UAT_STRING;

    while(print_able_range(str[count]) && str[count]!='/'){
        if (count>= URL_ARG_STRING_INCR_RATE * (resized+1)){
            new_str = realloc(new_str,URL_ARG_STRING_INCR_RATE * (resized+2) );
            resized++;
        }
        new_str[count] = str[count];
        count++;
    }
    arg->value_string = new_str;
    return count;
}
int consume_float(const char *str,UrlVariable *arg){
    float value = 0.0f;
    int count = 0 ;
    int found_period = 0;
    int after_period=1;
    while((str[count]>='0' && str[count]<='9') || str[count]=='.'){
        if(str[count]=='.'){
            // if period was already found then exit early
            if(found_period) return 0;
            found_period=1;
            count++;
            continue;
        }
        if(!found_period){
            value = value * 10 + str[count]-'0';
        }else{
            value += (str[count]-'0')/(10.0f*after_period);
            after_period*=10;
        }
        count++;
    }
    arg->value_float = value;
    arg->type = UAT_FLOAT;
    return count;
}



int consume_double(const char *str,UrlVariable *arg){
    double value = 0.0f;
    int count = 0 ;
    int found_period = 0;
    int after_period=1;
    while((str[count]>='0' && str[count]<='9') || str[count]=='.'){
        if(str[count]=='.'){
            // if period was already found then exit early
            if(found_period) return 0;
            found_period=1;
            count++;
            continue;
        }
        if(!found_period){
            value = value * 10 + str[count]-'0';
        }else{
            after_period*=10;
            value += (str[count]-'0')/(10.0f*after_period);
        }
        count++;
    }
    arg->value_double = value;
    arg->type = UAT_DOUBLE;
    return count;
    return 0;
}

int consume_arg(char type,const char *str,UrlVariable *arg){
    log_info("Consume Type [%c] ",type);
    switch (type) {
        case 'd':return consume_digit(str,arg);
        case 's':return consume_str(str,arg);
        case 'c':return consume_char(str,arg);
        case 'f':return consume_float(str,arg);
        case 'l':return consume_double(str,arg);
        default: return 0;
    }
}

int count_fmt_args(const char *fmt_url){
    char *ref = (char *)fmt_url;
    int count = 0;
    while(ref[0]){
        switch (ref[0]) {
        case '%':
            ref++;
            if(ref[0]!='%')count++;
        }
        ref++;
    }
    return  count;
}

UrlVariables find_if_match(const char* route_url,const char *url){
    
    //route copy init
    int route_len = strlen(route_url);
    char *fmt_url = malloc(route_len);
    memcpy(fmt_url,route_url,sizeof(char)*route_len);
    int url_ptr=0;

    // args intialization
    UrlVariables args = INIT_URL_ARGS();
    int total_args = count_fmt_args(fmt_url);
    
    if(total_args==0)
        return  args;

    args.args = malloc(sizeof(UrlVariable)*total_args);
    args.length = total_args;
    int cur_arg = 0;



    // fill args from url using route_structure
    while(fmt_url[0]){
        if(fmt_url[0]=='%' && fmt_url[1]!='%'){
            url_ptr += consume_arg(fmt_url[1],(url+url_ptr),&args.args[cur_arg]);
            fmt_url++;
            fmt_url++;
            cur_arg++;
        }else if(fmt_url[0]==(url+url_ptr)[0]){
            fmt_url++;
            url_ptr++;
        }else{
            args.length=0;
            if(args.args != NULL)
                free(args.args);
            return  args;
        }
    }
    return args;
}


void free_url_args(UrlVariables args){
    for (int i = 0; i < args.length; ++i){
        if(args.args[i].type ==  UAT_STRING && args.args[i].value_string != NULL){
            free(args.args[i].value_string);
        }
    }
    free(args.args);
}

static int begin_request_handler(struct mg_connection *conn){
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    log_debug("New connection at %s",request_info->uri);
    // normal url search
    for (size_t i = 0; i < ROUTE_TABLE.count; ++i){
        if(!strcmp(request_info->uri,ROUTE_TABLE.routes[i].url)){
            ROUTE_TABLE.routes[i].callback(conn);
            return 1;
        }
    }
    // variabled url search
    for (size_t i = 0; i < VAR_ROUTE_TABLE.count; ++i){
        UrlVariables args = find_if_match(VAR_ROUTE_TABLE.routes[i].url,request_info->uri);
        if(args.length > 0){
            VAR_ROUTE_TABLE.routes[i].callbackargs(conn,args);
            free_url_args(args);
            return 1;
        }
    }

    if (!serve_file(conn,(request_info->uri+1))) {
        return 1;
    }

    return !serve_file(conn,"htmls/404.html")?1:0;
}

void render_html(Request request,const char* file_name){
    if(!serve_file(request,file_name)){
        return;
    }
    log_error("Failed To Render %s",file_name);
}

void render_text(Request request,const char * text){
    mg_printf(request,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %lu\r\n"
              "\r\n"
              "%s",
              strlen(text), text);
}


int apply_template(char *from,char *to,TemplateVars templ_vars){
    memcpy(to,from,strlen(from));
    return strlen(from);
}

void render_template(Request request,const char* file_name,TemplateVars templ_vars){
    FILE *tmp_fp;

    FILE *fp = fopen(file_name,"r");
    if( fp == NULL){
        log_error("Failed to open [%s] ",file_name);
        return;
    }

    // convert the unique memory address to unique file name
    size_t tmp_file_id = *(size_t  *)request;
    // max int has 10 digits for 32bit, and 20 digits for 64bits
    // so to  cover it 35 will be enough
    char tmp_file_name[35];
    sprintf(tmp_file_name,"pb_tmp_%lu.html",tmp_file_id);

    tmp_fp = fopen(tmp_file_name,"w");
    if(tmp_fp == NULL){
        log_error("Failed creating tmp file [%s] ",tmp_file_name);
    }

    char read_chunk[MAX_READ_CHUNK];
    char write_chunk[MAX_READ_CHUNK];

    int ptr = 0,continue_from=0,read_buff_offset=0;
    while(!feof(fp)){
        // try to read whole chunk like a single object
        int read_count = fread((read_chunk+read_buff_offset),1,MAX_READ_CHUNK-read_buff_offset-5,fp);
        read_chunk[read_count]=0;

        continue_from = apply_template(read_chunk,write_chunk,templ_vars);
        log_debug("Inside File Chunker [%d]",MAX_READ_CHUNK-continue_from);
        fwrite(write_chunk,1,continue_from,tmp_fp);
        read_buff_offset = MAX_READ_CHUNK-continue_from-5;
        memcpy(read_chunk,(read_chunk+continue_from),read_buff_offset);
    }
    fclose(fp);
    fclose(tmp_fp);
    log_debug("Serving Template file... [%s]",tmp_file_name);
    render_html(request,tmp_file_name);
    // remove(tmp_file_name);
}




const char * get_method(Request request){
    const struct mg_request_info *request_info = mg_get_request_info(request);
    return request_info->request_method;
};

int server_run(char *port){
    struct mg_context *ctx;
    struct mg_callbacks callbacks;
    const char *options[] = {"listening_ports", port, NULL};
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.begin_request = begin_request_handler;
    ctx = mg_start(&callbacks, NULL, options);
    log_info("Server Running at localhost:%s ...",port);
    getchar();
    log_info("Server Stoped ");
    log_info("Exiting ... ");
    mg_stop(ctx);
    return 0;
}
