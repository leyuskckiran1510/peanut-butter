#include "../include/sqlite3.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ERROR_MSG_LEN 10000

typedef struct {
    int count;
    char **keys;
    char **values;
}Column ;

typedef struct {
   Column (*next_row)(void);
   Column (*all_row)(void);
   void (*stop)();
} sqlite_object; 

int callback(void* a1,int a2,char **a3,char**a4){
     for (int i = 0; i < a2; i++) {
        printf("%s = %s\n", a4[i], a3[i] ? a3[i] : "NULL");
    }
    printf("\n");
    return 0;
}


int main(void){
    sqlite3 *connection;
    sqlite3_stmt *res;
    char *error_buffer = calloc(1,MAX_ERROR_MSG_LEN);
    int status = sqlite3_open_v2("database.db",&connection,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,"unix-dotfile");
    if(status!=SQLITE_OK){
        printf("Error opeing sqlitedb [%s]",strerror(errno));
    }
    char query[] ="CREATE TABLE IF NOT EXISTS account(id INTEGER PRIMARY KEY autoincrement,name varchar(250),age int,phone varchar(10));\
                    INSERT INTO account(name,age,phone) VALUES('kiran',12,98000); \
                    SELECT * from account;"; 
    sqlite3_exec(connection,query,callback,NULL,&error_buffer);
    printf("Error if any [%s]\n",error_buffer);
    sqlite3_close(connection);
    return 0;
}