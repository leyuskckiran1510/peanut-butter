#include <string.h>
#include "peanut_butter.h"


void home(const char *method,Request request){
    if(!strcmp("GET",method)){
        return render_html(request,"htmls/index.html");
    }
    return render_html(request,"htmls/method_not_allowed.html");
}

void about(const char *method,Request request){
    return render_html(request,"htmls/about.html");
}


int main(void){
    URL("/",home);
    URL("/about",about);
    return server_run("8080");
}
