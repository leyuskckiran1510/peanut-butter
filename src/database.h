#ifndef __PB_DATABASE__
    #define __PB_DATABASE__
#include "../include/sqlite3.h"
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_ERROR_MSG_LEN 1024
#define SQLITE_PRIVATE  
#define database_query_fmt(x,...)  sprintf(QUERY_FMT_BUFFER,(x),##__VA_ARGS__);x=QUERY_FMT_BUFFER;


typedef struct {
    int cols;
    char **col_names;
    char **col_values;
}Row;

typedef int(*DatabaseCallback)(void* passthrough_data,int col_count,char **col_value,char **col_name);
typedef void(*RowCallback)(void *passthrough_data,Row row);

typedef struct{
    void *passthrough_data;
    RowCallback callback;
}CallbackWrapper;

typedef enum{
    // sucess
    DB_SUCESS_SELECT=0,
    DB_SUCESS_INIT=1,
    DB_SUCESS_EXEC=2,
    DB_SUCESS_CLOSE=3,
    // failuer
    DB_FAILED_SELECT=4,
    DB_FAILED_INIT=5,
    DB_FAILED_EXEC=6,
    DB_FAILED_CLOSE=7,
}DatabaseReturnCodes;

typedef enum{
    DB_SQLITE3 = 0,
}DatabaseType;


typedef const char* __database_uri_port_or_file_name;
typedef const char* __database_username;
typedef const char* __database_password;

typedef struct DatabaseStruct DatabaseStruct;
typedef DatabaseStruct* Database ;

struct DatabaseStruct {
    sqlite3 *__connection;
    int __is_initialized;
    int (*init)(Database db,const char * uri_with_port,const char* username,const char* password,int flags);
    int (*execute_single)(Database db,char * sql_query,DatabaseCallback callback,void* passthrough_data);
    int (*execute_multi)(Database db,char * sql_queries,DatabaseCallback callback,void* passthrough_data);
    int (*close_connection)(Database db);
    int (*logger)(const char* log_message);
};

static char QUERY_FMT_BUFFER[1024];

Database database_select(DatabaseType db_type);
int database_init(Database db,__database_uri_port_or_file_name uri,__database_username username,__database_password password,int flags);
int database_execute(Database db,char * sql_queries,RowCallback callback,void* passthrough_data);
int database_close(Database db);
void database_set_logger(Database db,int (*logger)(const char *msg));
#endif