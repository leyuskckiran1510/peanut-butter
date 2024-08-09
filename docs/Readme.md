# peanut-butter docs

this is docs to be filled
## code example
```c
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
```

## code breakdown

1. URL([route_string](#string),[view_callback](#view-callback)):- <br>
    `URL` is a macro that added the route to global RouteTable 

2. VAR_URL([route_string](#string),[view_callback_args](#view-callback-args)):- <br>
    It is same as `URL` with a catch, you can add string formator as a place holder
    ; allowing us to make a dynamic url,
    taking the above example
    route = `"/%d/%c/%s/home"`
    it matches following case scenarios
        1. `/1/a/a string/home` 
        2. `/2/b/b string/home` 
        ... 
    - in place of `%d` you can have any integer
    - in place of `%c` you can have any character
    - in place of `%s` you can have any string as long as it doesnot contains
      `/`, if you want `/` you can urlencode it
    - you can also combine them as
        `/%d%c%d%s/home`
        and many more
    - !! NOTE !!, `%s` is greedy, meaning, `%s` should be used at last or solo
        as it consume everything until `/`, so these will not work as expected
        - `/%s%d/`
        - `/%s%s/`
            ...

3. server_run([port_string](#string)):- take port number as string,
    and intializes the server, `server_run` should be last funciton to be called
    from main funciton, as it will start the server loop and anything 
    after it will only execute after server is closed.<br>
    ⚠️⚠️ NOTE ⚠️⚠️, `URL(x,y)` should be before `server_run`, otherwise
    routes wont be added to server RouteTable, when server runs. 

4. render_html([request](#request),[html_file_path](#string)):- As name suggests<br>
    this function returns html file to be displayed.
5. render_template([request](#request),[html_file_path](#string),[TEMP_VAR()](#template)):- <br>
    this function is same as `render_html` with a additional argument, [`TemplateVars`](#template),
    this is usefull when you want to use template html file with place holders that
    can be dynamically replaced .
    html_file_example
    ```html
    <!-- template.html--->
    <div>
        Hello {{name}}, How Are You.
        Your Role is {{role}}.
    </div>

    ```
    Here , `{{template_name}}` is a place holder, that can be replaced dynamically.
    example_pb_code for above html
    ```c
    void user_home(Request request){
        TEMP_INIT(); //template should be initialized before using it
        TEMP_VAL("name","Leyuskc",s);
        TEMP_VAL("role","admin",s);
        return  render_template(request,"template.html",TEMP_VAR());
    }
    ```
# String
<div id="string">
character array or string constant,
    example
```c
const char *url = "/home";
char url[100];
memcpy(url,"/xyz",6);
```
</div>

# VIEW_CALLBACK
<div id="view-callback">

> ViewCallback = func(Request [request](#request));

```c
// ViewCallback are functions that take Request as first parameter
void home(Request req); // ✅ valid
void about(Request req,int abc); // ❌ invalid
// ViewCallback must only take requests as parameter
```
</div>

# VIEW_CALLBACK_ARGS
<div id="view-callback-args">

> ViewCallbackArgs =  func(Request [request](#request),UrlVariables [url_variables](#url-variables));


```c
// ViewCallbackArgs are functions that take 
// 1. Request as first parameter
// 2. UrlVaribales as second arguments

void user_home(Request request,UrlVariables urlags); // ✅ valid
void about(Request req,UrlVariables urlags,int abc); // ❌ invalid
// ViewCallbackArgs must only take requests and UrlVaribales as parameter
```
</div>

# URL_VARIABLE

> UrlVariables = { <br>
>    union value; <br>
>    uvar_type type; <br>
> };
<div id="url-variable">

```c

UrlVariable urlvar;
urlvar.value ;  // value direct is the base to access any value their is
                // to be specific you can add prefix accordingly
/*
urlvar.i_value -> for integer
urlvar.s_value -> for char* or string
urlvar.c_value -> for char
urlvar.f_value -> for float
urlvar.d_value -> for double
*/
urlvar.type; // type determines what the value holds, it can be
            // char,char*,int,float,double
```
</div>

# URL_VARIABLES

>UrlVariables = {<br>
>    [UrlVariable](#url-variable) *args; <br>
>    uint8_t length; <br>
>};
<div id="url-variables">

```c

UrlVariables urlvar;
urlvar.args ;  // args is the list of UrlVariable
urlvar.length; // length is how many elements are their in args
```
</div>

# TEMPLATE
>TemplateVar = {
>    char *name;
>    UrlVariable value;
>}

>TemplateVars = {
>    TemplateVar *templ;
>    uint8_t length;
>} ;
<div  id="template" >

```c
typedef struct{
    char *name; // tempalte name
    UrlVariable value; //template value, read UrlVaribale for more info
}TemplateVar;

typedef struct{
    TemplateVar *templ; // list of TemplateVar, read above
    uint8_t length; // total template count
} TemplateVars;

```
- it has three main `macros`
1. `TEMP_INIT()`:- this must be called before using templates
2. `TEMP_VAL(template_name,template_value,template_type)`:-
    1. [template_name](#string): a string
    2. template_value : it can be any of these  data type,
                        int,char,char*,float,double
    3. template_type:- its a single character defining type of template_value
            `s` -> `string/char*`
            `i` -> `int`
            `f` -> `float`
            `d` -> `double`
            `c` -> `char`

            !! Note !! this is compile time constant, so 
            it is not character/string, its a direct symbol
            code example
```c
TEMP_VAL("name","Leyuskc",s);  // ✅ valid
TEMP_VAL("name","Leyuskc",'s');  // ❌ invalid
TEMP_VAL("name","Leyuskc","s");  // ❌ invalid
```
look at the `s` its like a expression it self not a value, so 
template type should be determined during the compile time

3. `TEMP_VAR()` :- invoke this macro where ever it calls for TemplateVars in your 
                code, but remember to invoke `TEMP_INIT()` first.


Any error will be detected during compile time and it will break. So don't worry about
it slipping in production. The errors will look wired for now

</div>

# REQUESTS
<div  id="request" >
</div>