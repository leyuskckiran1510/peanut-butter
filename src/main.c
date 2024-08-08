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
    TEMP_INIT();
    if(urlags.args[0].value==123){
        UrlQueries quires = parse_query(request);
        for (int i = 0; i < quires.length; ++i){
            TEMP_VAL(quires.queries[i].name,quires.queries[i].value,s);
        }
        render_template(request,"htmls/template.html",TEMP_VAR());
        return  free_url_query(quires);
    }
    if(urlags.args[0].value<123){
        return render_html(request,"htmls/index.html");
    }
    return redirect(request,"/",302);;
}


int server(){
    URL("/",home);
    URL("/about",about);
    VAR_URL("/%d/%d/%s/home",user_home);
    return server_run("8080");
}

int main(void){
    return server();
}
