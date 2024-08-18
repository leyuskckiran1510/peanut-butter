#define LOG_LEVEL 4
#include <string.h>
#include "peanut_butter.h"
#include "logger.h"

ROUTED(home){
    if(is_method("GET")){
        return render_html("/htmls/index.html");
    }
    return render_html("/htmls/method_not_allowed.html");
}

URL_VAR_ROUTED(user_home){    
    TEMPLATE_INIT();
    if(URL_VAR_INDEX(0,i)==123){
        QUERY_INIT();
        for (int i = 0; i < QUERY_LENGTH(); ++i){
            TEMPLATE_ASSIGN(QUERY_INDEX(i).name,QUERY_INDEX(i).value,s);
        }
        return render_template("htmls/template.html");;
    }
    if(URL_VAR_INDEX(0,i) < 123){
        return render_html("htmls/non_admin.html");
    }
    return redirect("/",302);
}


ROUTED(about){
    render_html("htmls/about.html");
}

URL_VAR_ROUTED(contact){
    TEMPLATE_INIT();
    if(!strcmp(URL_VAR_INDEX(0,s),"admin")){
        TEMPLATE_ASSIGN("name","Admin",s);
        TEMPLATE_ASSIGN("phone","+977-98000000000",s);
        TEMPLATE_ASSIGN("address","somewhere on earth",s);

    }else if(!strcmp(URL_VAR_INDEX(0,s),"manager")){
            TEMPLATE_ASSIGN("name","Manager",s);
            TEMPLATE_ASSIGN("phone","+977-98000000000",s);
            TEMPLATE_ASSIGN("address","somewhere on hell",s);
    }else{
        TEMPLATE_ASSIGN("name",URL_VAR_INDEX(0,s),s);
        TEMPLATE_ASSIGN("phone","---",s);
        TEMPLATE_ASSIGN("address","unknown user",s);
    }
    return render_template("htmls/contact.html");
}



ROUTED(forms){
    if(is_method("GET")){
        return render_html("htmls/forms.html");
    }
    else if (is_method("POST")){
        FORM_INIT(false);
        log_debug("1. { text, %s} ", FORM_GET("text-input1","default_value1"));
        log_debug("2. { filename, %s} ", FORM_GET("resume",NULL));
    }
    return redirect("/forms",302);
}

int server(){
    URL("/",home);
    URL("/about",about);
    URL("/forms",forms);
    VAR_URL("/%d/home",user_home);
    VAR_URL("/%s/contact",contact);
    return server_run("1234");
}

int main(void){
    return server();
}
