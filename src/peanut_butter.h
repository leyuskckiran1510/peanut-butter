#ifndef __PEANUT_BUTTER__

#define __PEANUT_BUTTER__

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/civetweb.h"
#include "logger.h"
#include "database.h"

#define MAX_ROUTES 1024
#define MAX_QUERIES 50
#define MAX_TEMPL_VAR_NAME 30
#define TEMPL_CHUNK 100

#ifdef _WIN32
  #define OS_PATH_SEP "\\" 
#else
  #define OS_PATH_SEP "/"
#endif

#define TEMP_FOLDER_PREFIX "user_upload_"
#define TEMP_FOLDER_FMT(request)  TEMP_FOLDER_PREFIX"%p",request
#define TEMP_TEMPL_FILE_PREFIX "pb_tmp_"
#define MAX_TEMP_FOLDER_FILE_SIZE 35

#define PB(x)  _pb_##x
#define free(x)  {free(x);x=NULL;}


#define FUNC_TEMPLATE_VAR_NAME __template_variable
#define FUNC_REQ_PARAM_NAME __request_param
#define FUNC_UVAR_PARAM_NAME __urlvariable_param
#define FUNC_QUERY_VAR_NAME __urlquery_var

#define URL(route,callback)          PB(add_route(route,&callback));
#define VAR_URL(var_route,callback)  PB(add_var_route(var_route,&callback));

#define ROUTED(func_name)   void func_name(Request FUNC_REQ_PARAM_NAME)
#define URL_VAR_ROUTED(func_name)   void func_name(Request FUNC_REQ_PARAM_NAME, UrlVariables FUNC_UVAR_PARAM_NAME)



#define TEMPLATE_INIT() TemplateVars FUNC_TEMPLATE_VAR_NAME  = {.length=0,.templ=malloc(sizeof(TemplateVar)*TEMPL_CHUNK),};
#define TEMPLATE_VAR()   FUNC_TEMPLATE_VAR_NAME 
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




#define QUERY_INIT()  UrlQueries* FUNC_QUERY_VAR_NAME = parse_query( (FUNC_REQ_PARAM_NAME) );\
                                query_track((FUNC_REQ_PARAM_NAME),FUNC_QUERY_VAR_NAME);

#define QUERY_VAR() FUNC_QUERY_VAR_NAME
#define QUERY_LENGTH() FUNC_QUERY_VAR_NAME->length
#define QUERY_INDEX(index) FUNC_QUERY_VAR_NAME->queries[index]
#define QUERY_GET(name,default_value) query_search(QUERY_VAR(),name,default_value)

/*must have initialized form with FORM_INIT to use form*/ 
#define FORM_INIT(save_file_bool) FormDatas*  FUNC_QUERY_VAR_NAME = parse_form( (FUNC_REQ_PARAM_NAME),(save_file_bool));\
                                    query_track((FUNC_REQ_PARAM_NAME),FUNC_QUERY_VAR_NAME);
#define FORM_VAR() QUERY_VAR()
#define FORM_LENGTH()  QUERY_LENGTH()
#define FORM_INDEX(index) QUERY_INDEX(index)
#define FORM_GET(name,default_value) QUERY_GET(name,default_value)
#define FORM_GET_FILE(name)  query_file_search(QUERY_VAR(),name)


#define INIT_URL_ARGS()    {.args=NULL,.length=0}
#define URL_VAR_INDEX(index,__type) FUNC_UVAR_PARAM_NAME.args[index].__type##_value
#define UrlVariable_TYPE(x,to)  switch (x){\
                        case 'd':to=UAT_INTEGER;break;\
                        case 'c':to=UAT_CHARACTER;break;\
                        case 's':to=UAT_STRING;break;\
                        case 'f':to=UAT_FLOAT;break;\
                        case 'l':to= UAT_DOUBLE;break;\
                        default:to=UAT_UNKNOWN;break;\
                        }\

                
#define get_method() _get_method(FUNC_REQ_PARAM_NAME)
#define is_method(method) (!strcmp(method,get_method()))
#define render_html(file_name) _render_html(FUNC_REQ_PARAM_NAME,(file_name))
#define render_404(file_name) _render_404(FUNC_REQ_PARAM_NAME,(file_name))
#define render_text(text) _render_text(FUNC_REQ_PARAM_NAME,(text))
#define render_raw_text(text) _render_raw_text(FUNC_REQ_PARAM_NAME,(text))
#define render_template(file_name) _render_template(FUNC_REQ_PARAM_NAME,(file_name),FUNC_TEMPLATE_VAR_NAME)
#define redirect(route,redir_code) _redirect(FUNC_REQ_PARAM_NAME,(route),(redir_code))
#define server_run(port) _server_run((port), SECURITY_HEADERS)

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
typedef const struct mg_connection *conRequest;
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
}UrlQuery,FormData;

typedef struct{
    FormData queries[MAX_QUERIES];
    uint16_t length;
}UrlQueries,FormDatas;


typedef struct{
    Callback *callback_pointers;
    void*    *data_pointer;
    uint16_t count;
}UserData;

#ifdef __PB_DOT_C__
    static Routes ROUTE_TABLE;
    static Routes VAR_ROUTE_TABLE;
    #define SECURITY 0
#endif

#ifndef SECURITY
    #define SECURITY 0
    #pragma message "\n=========================\n \
\t Please Add `#define SECURITY value` at top of you backend file \n \
\t You must choose one of the following \n \
\t value = 0; no security {allow image,media and script to be loaded from anywhere} \n \
\t value > 0; max security {only allow image,media and script from same domain} \n \
\t Used `0` [ max-security from XSS ] by default,\n \
========================= \n \n "

#endif


#if SECURITY > 0
    #define SEC_IMG_SRC  "'self'"
    #define SEC_MEDIA_SRC "'self'"
    #define SEC_SCRIPT_SRC "'self'"
#elif defined (SECURITY)
    #define SEC_IMG_SRC  "*"
    #define SEC_MEDIA_SRC "*"
    #define SEC_SCRIPT_SRC "*"
#endif
#ifndef __PB_DOT_C__
static char SECURITY_HEADERS[] = "Content-Security-Policy: default-src 'self';"
                                                           "img-src "    SEC_IMG_SRC ";"
                                                           "media-src "  SEC_MEDIA_SRC ";"
                                                           "script-src " SEC_SCRIPT_SRC  ";"
                                                           "\r\n"
                                  "X-Frame-Options: SAMEORIGIN \r\n"
                                  "X-Content-Type-Options: nosniff \r\n"

;
#endif
const char * _get_method(Request request);

void PB(add_route(char* route,ViewCallback callback));

void PB(add_var_route(char*  var_route,ViewCallbackArgs callback));


/*
start server at port: 
*/
int _server_run(char *port,char *security_headers);
// these are inner funcitons with macro wrappers
void _render_html(Request request,const char* file_name);
void _render_404(Request request,const char* file_name);
void _render_text(Request request,const char * text);
void _render_raw_text(Request request,const char *text);
void _render_template(Request request,const char* file_name,TemplateVars templ_vars);
void _redirect(Request request,char *to_url,uint16_t redirect_code);

/*
🟥🟥 Don't forget to call `free_url_query` after you have completed using the
query
*/
UrlQueries* parse_query(Request request);
FormDatas* parse_form(Request request,bool save_file_bool);

char * query_search(UrlQueries *url_queries,char *name,char *default_value);
char * query_file_search(UrlQueries *url_queries,char *name);

void query_track(Request request,UrlQueries *queries);

#endif