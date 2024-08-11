#ifndef __PEANUT_BUTTER__

#define __PEANUT_BUTTER__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/civetweb.h"
#include "logger.h"

#define MAX_ROUTES 1024
#define MAX_QUERIES 50
#define MAX_TEMPL_VAR_NAME 30
#define TEMPL_CHUNK 100

#define PB(x)  _pb_##x
#define free(x)  {free(x);x=NULL;}


#define FUNC_TEMPLATE_VAR_NAME template_variable
#define FUNC_REQ_PARAM_NAME request
#define FUNC_UVAR_PARAM_NAME urlvariable

#define URL(route,callback)          PB(add_route(route,&callback));
#define VAR_URL(var_route,callback)  PB(add_var_route(var_route,&callback));

#define ROUTED(func_name)   void func_name(Request FUNC_REQ_PARAM_NAME)
#define URL_VAR_ROUTED(func_name)   void func_name(Request FUNC_REQ_PARAM_NAME, UrlVariables FUNC_UVAR_PARAM_NAME)



#define TEMPLATE_INIT() TemplateVars FUNC_TEMPLATE_VAR_NAME  = {.length=0,.templ=malloc(sizeof(TemplateVar)*TEMPL_CHUNK),};
#define TEMPLATE_ASSIGN(_name,_avalue,_type) { \
                                FUNC_TEMPLATE_VAR_NAME .templ[FUNC_TEMPLATE_VAR_NAME .length].name = _name;\
                                FUNC_TEMPLATE_VAR_NAME .templ[FUNC_TEMPLATE_VAR_NAME .length].value._type##_value = _avalue;\
                                FUNC_TEMPLATE_VAR_NAME .templ[FUNC_TEMPLATE_VAR_NAME .length].value.type =UAT_##_type;\
                                FUNC_TEMPLATE_VAR_NAME .length++; \
                                 int mutiplier = (FUNC_TEMPLATE_VAR_NAME .length/TEMPL_CHUNK)+1; \
                                 if(FUNC_TEMPLATE_VAR_NAME .length >= TEMPL_CHUNK*mutiplier){\
                                       FUNC_TEMPLATE_VAR_NAME .templ =realloc(FUNC_TEMPLATE_VAR_NAME .templ,sizeof(TemplateVar)*TEMPL_CHUNK*mutiplier);\
                                 }\
                                }\

#define TEMPLATE_VAR()   FUNC_TEMPLATE_VAR_NAME 



#define QUERY_INIT()  UrlQueries __url_queries = parse_query( (FUNC_REQ_PARAM_NAME) );\
                                query_track((FUNC_REQ_PARAM_NAME),&__url_queries);

#define QUERY_LENGTH() __url_queries.length
#define QUERY_INDEX(index) __url_queries.queries[index]
#define QUERY_VAR() __url_queries
#define QUERY_GET(_name,default_value) query_search(QUERY_VAR(),_name,default_value);

#define URL_VAR_INDEX(index,__type) FUNC_UVAR_PARAM_NAME.args[index].__type##_value


#define UrlVariable_TYPE(x,to)  switch (x){\
                        case 'd':to=UAT_INTEGER;break;\
                        case 'c':to=UAT_CHARACTER;break;\
                        case 's':to=UAT_STRING;break;\
                        case 'f':to=UAT_FLOAT;break;\
                        case 'l':to= UAT_DOUBLE;break;\
                        default:to=UAT_UNKNOWN;break;\
                        }\

#define INIT_URL_ARGS()    {.args=NULL,.length=0}
                        
#define get_method() _get_method(FUNC_REQ_PARAM_NAME)
#define is_method(method) (!strcmp(method,get_method()))
#define render_html(file_name) _render_html(FUNC_REQ_PARAM_NAME,(file_name))
#define render_text(text) _render_html(FUNC_REQ_PARAM_NAME,(text))
#define render_template(file_name) _render_template(FUNC_REQ_PARAM_NAME,(file_name),FUNC_TEMPLATE_VAR_NAME)
#define redirect(file_name,redir_code) _redirect(FUNC_REQ_PARAM_NAME,(file_name),(redir_code))


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



typedef struct mg_connection *Request;
typedef void (*ViewCallback) (Request request);
typedef void (*ViewCallbackArgs) (Request request,UrlVariables url_variables);
typedef void (*Callback) (void *);

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


typedef struct{
    Callback *callback_pointers;
    void*    *data_pointer;
    uint16_t count;
}UserData;

static Routes ROUTE_TABLE;
static Routes VAR_ROUTE_TABLE;


const char * _get_method(Request request);

void PB(add_route(char* route,ViewCallback callback));

void PB(add_var_route(char*  var_route,ViewCallbackArgs callback));


int server_run(char *port);

void _render_html(Request request,const char* file_name);

void _render_text(Request request,const char * text);

void _render_template(Request request,const char* file_name,TemplateVars templ_vars);

void _redirect(Request request,char *to_url,uint16_t redirect_code);

/*
🟥🟥 Don't forget to call `free_url_query` after you have completed using the
query
*/
UrlQueries parse_query(Request request);

char * query_search(UrlQueries url_queries,char *name,char *default_value);

void query_track(Request request,UrlQueries *queries);

#endif