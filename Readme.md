# Simple miniframework for web-backend 
Aim of this project to build a miniframework to get started with the 
web with speed out of the box.


# Features
1. Static HTML server with costume route [completed]
2. Server static files [2024-Aug-04] [completed]
3. Extended route features
    1. In route  variables [2024-Aug-06] [completed]
    1. Regex route   [2024-Aug-05] [cancled for now]
4. HTML templating   [2024-Aug-06] [started-working] 
    - [2024-Aug-06], template copying works, now just swaping
        the template varibale is left
    - [completed][2024-Aug-07:23-11-33 PM]
5. Redirect, [completed] [2024-Aug-07:23-11-17 PM]
6. urlencoded_parse [to work on]


# Features To add
1. Middlewares
2. Authentications
3. Persestent datas like Cookies and Sessions,
4. RateLimiting
5. HTTPS/test/support
6. Parse Post forms/urls
 

# Technology and Software in Use
1. [civetweb](https://github.com/civetweb/civetweb)


# Got starting point from 
1. https://medium.com/@justup1080/tutorial-creating-a-minimalist-http-server-in-c-2303d140c725
2. 


# How To use
1. Clone the repo
2. Open `src/main.c` 
3. Write your backend logic, example shown below

```c
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

void user_home(Request request,UrlVariables urlags){
    char secrete = 'k';
    // this is bad way to validate admins, 
    // this is just a fun example
    if(urlags.args[0].value==0 && urlags.args[1].value == secrete){
        log_info("Admin name is %s",urlags.args[2].value_string);
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

```


4. now compile and run
```bash
make
```
5. And You are good to go.




# FAQ
