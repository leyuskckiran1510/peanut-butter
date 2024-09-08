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
typedef struct {
    int count;
    char **col_names;
    char **col_values;
}Column ;

typedef struct {
   Column (*next_row)(void);
   Column (*all_row)(void);
   void (*stop)();
} sqlite_object; 

typedef int(*DatabaseCallback)(void* passthrough_data,int col_count,char **col_value,char **col_name);

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


typedef char* __database_uri_port_or_file_name;
typedef char* __database_username;
typedef char* __database_password;

typedef struct DatabaseStruct DatabaseStruct;
typedef DatabaseStruct* Database ;

struct DatabaseStruct {
    sqlite3 *__connection;
    int __is_initialized;
    int (*init)(Database db,__database_uri_port_or_file_name uri_with_port,__database_username username,__database_password password);
    int (*init_v2)(Database db,__database_uri_port_or_file_name uri_with_port,__database_username username,__database_password password,int flags);
    int (*execute_single)(Database db,char * sql_query,DatabaseCallback callback,void* passthrough_data);
    int (*execute_multi)(Database db,char * sql_queries,DatabaseCallback callback,void* passthrough_data);
    int (*close_connection)(Database db);
    int (*logger)(const char* log_message);
};

Database database_select(DatabaseType db_type);
#endif