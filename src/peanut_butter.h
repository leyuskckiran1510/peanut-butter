#ifndef __PEANUT_BUTTER__
    #define __PEANUT_BUTTER__

#define MAX_ROUTES 1024
#define PB(x)  _pb_##x
#include "../include/civetweb.h"

#define ROUTE_COUNT __PB_ROUTES_COUNT

#define URL(route,callback)  PB(add_route(route,&callback));

typedef struct mg_connection *Request;

typedef void (*ViewCallback) (const char* method, Request request);

typedef struct{
    char *url;
    ViewCallback callback;
}Route;

typedef struct {
    Route routes[MAX_ROUTES];
    unsigned int count;
}Routes;

static Routes ROUTE_TABLE;

void PB(add_route(char*,ViewCallback));
int server_run(char *port);
void render_html(Request request,const char* file_name);
#endif