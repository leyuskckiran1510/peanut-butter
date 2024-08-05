#ifndef __PEANUT_BUTTER__
    #define __PEANUT_BUTTER__

#include <stdint.h>
#define MAX_ROUTES 1024
#define PB(x)  _pb_##x
#include "../include/civetweb.h"

#define ROUTE_COUNT __PB_ROUTES_COUNT

#define free(x)  {free(x);x=NULL;}

#define log(type,fmt,... ) printf( "[%5s] %s:%d | " fmt"\n",type,__FILE__,__LINE__, ##__VA_ARGS__ );
#define log_info(fmt,...)  log("INFO",fmt,##__VA_ARGS__);
#define log_debug(fmt,...)  log("DEBUG",fmt,##__VA_ARGS__);
#define log_error(fmt,...)  log("ERROR",fmt,##__VA_ARGS__);
#define log_warn(fmt,...)  log("WARN",fmt,##__VA_ARGS__);

#define URL(route,callback)  PB(add_route(route,&callback));
#define VAR_URL(var_route,callback)  PB(add_var_route(var_route,&callback));


#define URLARG_TYPE(x,to)  switch (x){\
                        case 'd':to=UAT_INTEGER;break;\
                        case 'c':to=UAT_CHARACTER;break;\
                        case 's':to=UAT_STRING;break;\
                        case 'f':to=UAT_FLOAT;break;\
                        case 'l':to= UAT_DOUBLE;break;\
                        default:to=UAT_UNKNOWN;break;\
                        }\

typedef enum {
    UAT_INTEGER=0,
    UAT_CHARACTER,
    UAT_STRING,
    UAT_FLOAT,
    UAT_DOUBLE,
    UAT_UNKNOWN,
}UrlArgType;

typedef struct{
    union {
        int value;
        char ua_char;
        char* ua_char_ptr;
        float ua_float;
        double ua_double;
    };
    UrlArgType type;

} UrlArg;

typedef struct{
    UrlArg *args;
    uint8_t length;
}UrlArgs;

#define INIT_URL_ARGS()    {.args=NULL,.length=0}


typedef struct mg_connection *Request;
typedef void (*ViewCallback) (Request request);
typedef void (*ViewCallbackArgs) (Request request,UrlArgs urlagrs);


typedef struct{
    char *url;
    union{
        ViewCallback callback;
        ViewCallbackArgs callbackargs;
    };
    int callback_type;
}Route;

typedef struct {
    Route routes[MAX_ROUTES];
    unsigned int count;
}Routes;

static Routes ROUTE_TABLE;
static Routes VAR_ROUTE_TABLE;

void PB(add_route(char* route,ViewCallback callback));
void PB(add_var_route(char*  var_route,ViewCallbackArgs callback));

int server_run(char *port);
void render_html(Request request,const char* file_name);
const char * get_method(Request request);
int count_fmt_args(const char *fmt_url);
UrlArgs find_if_match(const char* og_url,const char *url);
#endif