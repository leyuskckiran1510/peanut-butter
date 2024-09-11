#include "database.h"
#define LOG_LEVEL 4
#define SECURITY 1

#include <stdio.h>
#include <string.h>
#include "peanut_butter.h"

// make db global such that it is easier to access through out functions
Database db;

ROUTED(home){
    if(is_method("GET")){
        return render_html("/htmls/index.html");
    }

    return render_404("/htmls/method_not_allowed.html");
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
        log_debug("2. { filename, %s} ", FORM_GET_FILE("resume"));
    }
    return redirect("/forms",302);
}

ROUTED(text_response){
    return render_text("<h1>Hello From PB<h1>");
}

ROUTED(raw_text_response){
    return render_raw_text("<h1>Hello From PB<h1>");
}

void login_callback(void *passthrough,Row row){
    *(int *)passthrough = 1;
}

ROUTED(login){
    if(is_method("GET")){
        return render_html("htmls/login.html");
    }
    else if(is_method("POST")){
        FORM_INIT(false);
        char *username = FORM_GET("username",NULL);
        char *password = FORM_GET("password",NULL);
        char *query_fmt = "SELECT username from account where username == '%s' and password == '%s';";
        int found = 0;
        database_query_fmt(query_fmt,username,password);
        database_execute(db,query_fmt,login_callback,&found);
        if (found){
            return render_text("You are logged in");
        }
        return render_text("Credentials wrong!! ");
    }
}

ROUTED(signup){
    if(is_method("GET")){
        return render_html("htmls/signup.html");
    }
    else if(is_method("POST")){
        FORM_INIT(false);
        char *username = FORM_GET("username",NULL);
        char *password = FORM_GET("password",NULL);
        char *query_fmt = "INSERT INTO account(username,password) VALUES ('%s','%s');";
        database_query_fmt(query_fmt,username,password);
        int status = database_execute(db,query_fmt,login_callback,NULL);
        if (DB_SUCESS_EXEC){
            return redirect("/login",303);
        }
        return redirect("/signup",303);
    }
}

int logger(const char * msg){
    log_debug("%s",msg);
    return 0;
}

void callback(void * passthroug_data,Row row){
    for(int i=0;i<row.cols;i++){
        log_debug("%s -> %s",row.col_names[i],row.col_values[i]);
    }
}

int server(){
    db = database_select(DB_SQLITE3);
    if (DB_FAILED_INIT ==  database_init(db,"database.db",NULL,NULL,SQLITE_OPEN_READWRITE)){
        logger("Failed to init databse");
        return 0;
    };
    database_set_logger(db,logger);
    database_execute(db,"CREATE TABLE IF NOT EXISTS account(id INTEGER PRIMARY KEY autoincrement,username varchar(250), password varchar(10));",callback,NULL);
    URL("/",home);
    URL("/about",about);
    URL("/forms",forms);
    URL("/txt1",text_response);
    URL("/login",login);
    URL("/signup",signup);
    URL("/txt2",raw_text_response);
    VAR_URL("/%d/home",user_home);
    VAR_URL("/%s/contact",contact);
    
    return server_run("1234");
}

int main(void){
    return server();
}
