# Simple miniframework for web-backend 
Aim of this project to build a miniframework to get started with the 
web with speed out of the box.


# Features
1. Static HTML server with costume route [completed]
2. Server static files [2024-Aug-04] [completed]
3. Extended route features
    1. In route  variables [2024-Aug-05] [almost completed]
    1. Regex route   [2024-Aug-05] [cancled for now]
4. Redirect, Html templating


# Technology and Software in Use
1. [civetweb](https://github.com/civetweb/civetweb)
    This project uses very old version of civetweb i.e civetweb 1.8.
    https://civetweb.github.io/civetweb/UserManual.html


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

```


4. now compile and run
```bash
make
```
5. And You are good to go.




# FAQ

1. Why does it use old version of civetweb when newer versions like 1.16 have been released?
-> The project started with a motive to implement https logics with [httplib](https://github.com/lammertb/libhttp),
    which was a fork og civetweb, but was not mainted and caused troubles so, used the civetweb directly.
2. Will civetweb version be updated?
-> Yes I will grdually increase to latest version and mainten accordingly.