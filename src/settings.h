#ifndef __PB_SETINGS__
#define __PB_SETINGS__

// make db global such that it is easier to access through out functions
#include "database.h"

// name of database connection variable
#define DATABASE_NAME db;
/*
// logging level,
0   -> ERROR
1   -> ERROR,WARN
2   -> ERROR,WARN,DEBUG
>=3 -> ERROR,WARN,DEBUG,INFO 
*/
#define LOG_LEVEL 4
/*
// XSS-security for response headers
0   ->  no security {allow image,media and script to be loaded from anywhere}
>0  ->  max security {only allow image,media and script from same domain}

Note:- Used `1` [ max-security from XSS ] by default,
*/
#define SECURITY 1

// MAXIMUM ALLOWED ROUTES
#define MAX_ROUTES 1024
// MAXIMUM NUMBERS OF URL QUERYIES ALLOWED
/*
for example: example.com/?name=a&b=c
here total queries is 2
*/
#define MAX_QUERIES 50
// MAXIMUM LENGTH OF TEMPLATE_VARIABLE ALLOWED
// {name} , herer templ_var_name length is 4
#define MAX_TEMPL_VAR_NAME 30
// MAXIMUM NUMBER OF CHARS TO PROCESS AT ONCE WHILE PROCESSING TEMPLATE
// HTML FILES
#define TEMPL_CHUNK 100


// users upload folder prefix
#define TEMP_FOLDER_PREFIX "user_upload_"

// prefix for html file, generated while redering template variables
// this is intermediate file and will be automatically removed
#define TEMP_TEMPL_FILE_PREFIX "pb_tmp_"


// DO NOT EDIT FROM BELOW HERE
static Database DATABASE_NAME;

#endif