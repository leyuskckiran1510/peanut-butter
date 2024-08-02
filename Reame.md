# Simple miniframework for web-backend 
Aim of this project to build a miniframework to get started with the 
web with speed out of the box.


# Recent Status
1. Handles static HTML serve at http not https

# uses [civetweb](https://github.com/civetweb/civetweb)
This project uses very old version of civetweb i.e civetweb 1.8.



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


void home(const char *method,Request request){
    if(!strcmp("GET",method)){
        return render_html(request,"index.html");
    }
    return render_html(request,"method_not_allowed.html");

}

void about(const char *method,Request request){
    return render_html(request,"about.html");
}


int main(void){
    URL("/",home);
    URL("/about",about);
    return server_run("8080");
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