#include <string.h>
#include "peanut_butter.h"


void home(Request request){
    const char *method = get_method(request);
    if(!strcmp("GET",method)){
        return render_html(request,"htmls/index.html");
    }
    return render_html(request,"htmls/method_not_allowed.html");
}

void about(Request request){
    return render_html(request,"htmls/about.html");
}

void user_home(Request request,UrlArgs urlags){
    char secrete = 'k';
    // this is bad way to validate admins, 
    // this is just a fun example
    if(urlags.args[0].value==0 && urlags.args[1].value == secrete){
        log_info("Admin name is %s",urlags.args[2].ua_char_ptr);
        return render_html(request,"htmls/admin.html");
    }
    return render_html(request,"htmls/index.html");
}

int server(){
    URL("/",home);
    URL("/about",about);
    VAR_URL("/%d/%c/%s/home",user_home);
    return server_run("8080");
}

int main(void){
    return server();
}
