#include "peanut_butter.h"

ROUTED(home){
    if(is_method("GET")){
        return render_html("/htmls/index.html");
    }
    return render_html("/htmls/method_not_allowed.html");
}


ROUTED(about){
    return render_html("htmls/about.html");
}


URL_VAR_ROUTED(user_home){    
    TEMPLATE_INIT();
    if(URL_VAR_INDEX(0,d)==123){
        QUERY_INIT();
        for (int i = 0; i < QUERY_LENGTH(); ++i){
            TEMPLATE_ASSIGN(QUERY_INDEX(i).name,QUERY_INDEX(i).value,s);
        }
        return render_template("htmls/template.html");;
    }
    if(URL_VAR_INDEX(0,d) < 123){
        return render_html("htmls/index.html");
    }
    return redirect("/",302);
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
