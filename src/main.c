// #include <stdio.h>
#include <string.h>

#include "peanut_butter.h"


void home(Request request){
    const char *method = get_method(request);
    if(!strcmp("GET",method)){
        return render_html(request,"/htmls/index.html");
    }
    return render_html(request,"/htmls/method_not_allowed.html");
}

void about(Request request){
    return render_html(request,"htmls/about.html");
}

void user_home(Request request,UrlVariables urlags){
    if(urlags.args[0].value==123){
        return render_html(request,"htmls/admin.html");
    }
    return render_html(request,"htmls/index.html");
}

void template(Request request){
    TemplateVars vars = {
        .length=0,
        .templ=NULL,
    };
    return render_template(request,"htmls/template.html",vars);
}

int server(){
    URL("/",home);
    URL("/temp",template);
    URL("/about",about);
    VAR_URL("/%d/%f/%s/home",user_home);
    return server_run("8080");
}

int main(void){
    return server();
}
