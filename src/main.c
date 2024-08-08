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
    UrlQueries quires = parse_query(request);
    if(urlags.args[0].value==123){
        TEMP_VAL("name",urlags.args[2].s_value,s);
        TEMP_VAL("age",urlags.args[1].i_value,i);
        free_url_query(quires);
        // log_debug("Freeing [%d]",quires.length);
        return render_template(request,"htmls/template.html",TEMP_VAR());
    }
    if(urlags.args[0].value<123){
        free_url_query(quires);
        return render_html(request,"htmls/index.html");
    }

    free_url_query(quires);
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
