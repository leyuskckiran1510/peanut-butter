#ifndef __PEANUT_BUTTER__
    #define __PEANUT_BUTTER__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_ROUTES 1024
#define MAX_CONNECTION_AT_A_TIME 500
#define MAX_QUERIES 50
#define PB(x)  _pb_##x
#include "../include/civetweb.h"

#define ROUTE_COUNT __PB_ROUTES_COUNT

#define free(x)  {free(x);x=NULL;}


#define DEBUG 1


#if DEBUG == 1
    #define log(type,fmt,... ) printf( "[%5s] %s:%6d | " fmt"\n",type,__FILE__,__LINE__, ##__VA_ARGS__ );
#else
    #define log(type,fmt,... )  ;
#endif


#define log_info(fmt,...)  log("INFO",fmt,##__VA_ARGS__);
#define log_debug(fmt,...)  log("DEBUG",fmt,##__VA_ARGS__);
#define log_error(fmt,...)  log("ERROR",fmt,##__VA_ARGS__);
#define log_warn(fmt,...)  log("WARN",fmt,##__VA_ARGS__);


#define URL(route,callback)  PB(add_route(route,&callback));
#define VAR_URL(var_route,callback)  PB(add_var_route(var_route,&callback));

#define MAX_TEMPL_VAR_NAME 30
#define TEMPL_CHUNK 100
#define TEMP_INIT() TemplateVars _tmpl_var = {.length=0,.templ=malloc(sizeof(TemplateVar)*TEMPL_CHUNK),};
#define TEMP_VAL(_name,_avalue,_type) { \
                                _tmpl_var.templ[_tmpl_var.length].name = _name;\
                                _tmpl_var.templ[_tmpl_var.length].value._type##_value = _avalue;\
                                _tmpl_var.templ[_tmpl_var.length].value.type =UAT_##_type;\
                                _tmpl_var.length++; \
                                 int mutiplier = (_tmpl_var.length/TEMPL_CHUNK)+1; \
                                 if(_tmpl_var.length >= TEMPL_CHUNK*mutiplier){\
                                       _tmpl_var.templ =realloc(_tmpl_var.templ,sizeof(TemplateVar)*TEMPL_CHUNK*mutiplier);\
                                 }\
                                }\

#define TEMP_VAR()   _tmpl_var

#define UrlVariable_TYPE(x,to)  switch (x){\
                        case 'd':to=UAT_INTEGER;break;\
                        case 'c':to=UAT_CHARACTER;break;\
                        case 's':to=UAT_STRING;break;\
                        case 'f':to=UAT_FLOAT;break;\
                        case 'l':to= UAT_DOUBLE;break;\
                        default:to=UAT_UNKNOWN;break;\
                        }\

typedef enum {
    UAT_INTEGER=0,UAT_i=0,
    UAT_CHARACTER=1,UAT_c=1,
    UAT_STRING=2,UAT_s=2,
    UAT_FLOAT=3,UAT_f=3,
    UAT_DOUBLE=4,UAT_d=4,
    UAT_UNKNOWN=5,UAT_u=5,
}UrlVariableType;


typedef struct{
    union {
        int value;
        int i_value;
        char c_value;
        char* s_value;
        float f_value;
        double d_value;
    };
    UrlVariableType type;

} UrlVariable;


typedef struct{
    char *name;
    UrlVariable value;
}TemplateVar;

typedef struct{
    TemplateVar *templ;
    uint8_t length;
} TemplateVars;

typedef struct{
    UrlVariable *args;
    uint8_t length;
}UrlVariables;

#define INIT_URL_ARGS()    {.args=NULL,.length=0}


typedef struct mg_connection *Request;
typedef void (*ViewCallback) (Request request);
typedef void (*ViewCallbackArgs) (Request request,UrlVariables url_variables);


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

typedef struct{
    char *name;
    char *value;
}UrlQuery;

typedef struct{
    UrlQuery queries[MAX_QUERIES];
    uint16_t length;
}UrlQueries;

static Routes ROUTE_TABLE;
static Routes VAR_ROUTE_TABLE;


const char * get_method(Request request);
void PB(add_route(char* route,ViewCallback callback));
void PB(add_var_route(char*  var_route,ViewCallbackArgs callback));


int server_run(char *port);
void render_html(Request request,const char* file_name);
void render_template(Request request,const char* file_name,TemplateVars templ_vars);
void redirect(Request request,char *to_url,uint16_t redirect_code);
/*
🟥🟥 Don't forget to call `free_url_query` after you have completed using the
query
*/
UrlQueries parse_query(Request request);
void free_url_query(UrlQueries quires);
#endif