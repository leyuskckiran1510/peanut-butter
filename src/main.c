#include <string.h>
#define LOG_LEVEL 4
#include "peanut_butter.h"

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


int server(){
    URL("/",home);
    URL("/about",about);
    VAR_URL("/%d/home",user_home);
    VAR_URL("/%s/contact",contact);
    return server_run("8080");
}

int main(void){
    return server();
}
