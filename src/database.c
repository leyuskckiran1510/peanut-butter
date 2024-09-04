#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define throw(x)   __assert_fail((x),__FILE__,__LINE__,__ASSERT_FUNCTION);

char sql_syntax_error[] = " [ <= SYNTAX ERROR HERE ] ";

int callback(void* a1,int a2,char **a3,char**a4){
    (void)a1;
     for (int i = 0; i < a2; i++) {
        printf("%s = %s\n", a4[i], a3[i] ? a3[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int sqlite_init(Database db,char *uri_with_port,char* username,char* password){
    // sqlite doesnot requires username and password
    (void)username;
    (void)password;

    int status = sqlite3_open(uri_with_port,&db.__connection);
     if(status!=SQLITE_OK){
        if(db.logger){
            db.logger("failed to initialize sqlite with error");
            db.logger(strerror(errno));
        }
        return  DB_FAILED_INIT;
    }

    db.__is_initialized=1;
    return DB_SUCESS_INIT;
}
int sqlite_init_v2(Database db,char *uri_with_port,char* username,char* password,int flags){
    (void)username;
    (void)password;
    sqlite3 *connection;
    int status = sqlite3_open_v2(uri_with_port,&connection,flags | SQLITE_OPEN_CREATE,"unix-dotfile");
    db.__connection = connection;
     if(status!=SQLITE_OK){
        if(db.logger){
            db.logger("failed to initialize sqlite with error");
            db.logger(strerror(errno));
        }
        return DB_FAILED_INIT;
    }
    return DB_SUCESS_INIT;
   
}
void sqlite_exec_error_log(Database db,char* sql_queries){
    db.logger("query execution failed with following exception");
    db.logger(db.__error_buffer_pointer);
    db.logger(sqlite3_errmsg((sqlite3 *)db.__connection));
    db.logger(sql_queries);
    int error_position = sqlite3_error_offset((sqlite3 *)db.__connection);
    if (error_position>=0){
        char *trackback = calloc(1,strlen(sql_queries) + error_position);
        int sql_index = 0,tb_index=0;
        while(sql_queries[sql_index]){
            trackback[tb_index]=sql_queries[sql_index];
            if(sql_index==error_position){
                int error_index = 0;
                while(sql_syntax_error[error_index]){
                    trackback[++tb_index] = sql_syntax_error[error_index];
                }
            }
            if(sql_queries[sql_index]==';'){
                trackback[++tb_index]='\n';
            }
            sql_index++;
            tb_index++;
        }
        db.logger(trackback);
        free(trackback);
        trackback=NULL;
    }

}

int sqlite_execute_single(Database db,char * sql_query,DatabaseCallback callback,void* passthrough_data){
    if(db.__error_buffer_pointer == NULL){
            db.__error_buffer_pointer = calloc(1,MAX_ERROR_MSG_LEN);
        }
    int status = sqlite3_exec(db.__connection,sql_query,callback,passthrough_data,&db.__error_buffer_pointer);
    if(status==SQLITE_ABORT){
        if(db.logger){
            sqlite_exec_error_log(db, sql_query);
        }
        return DB_FAILED_INIT;
    }
    return DB_SUCESS_EXEC;
}

int sqlite_execute_multi(Database db,char * sql_queries,DatabaseCallback callback,void* passthrough_data){
    if(db.__error_buffer_pointer == NULL){
        db.__error_buffer_pointer = calloc(1,MAX_ERROR_MSG_LEN);
    }
    int status = sqlite3_exec(db.__connection,sql_queries,callback,passthrough_data,&db.__error_buffer_pointer);
    if(status==SQLITE_ABORT){
        if(db.logger){
            sqlite_exec_error_log(db, sql_queries);
        }
        return DB_FAILED_INIT;
    }
    return DB_SUCESS_EXEC;
}

int sqlite_close_connection(Database db){
    int status = sqlite3_close((sqlite3 *)(db.__connection));
    if(status)
        return  DB_FAILED_CLOSE;
    return DB_SUCESS_CLOSE;
}

Database select_database(DatabaseType db_type){
    Database db;
    switch (db_type) {
       case DB_SQLITE3:{
        db.init = sqlite_init;
        db.init_v2 = sqlite_init_v2;
        db.execute_single = sqlite_execute_single;
        db.execute_multi = sqlite_execute_multi;
        db.close_connection = sqlite_close_connection;
        db.logger=NULL;
        db.__error_buffer_pointer=NULL;
        break;
       }
    default:
        throw("Database must be one of these {DB_SQLITE3}");
    }
    return db;
}
int  logger(const char *msg){
    printf("[logger] %s",msg);
    return 0;
}

int func(sqlite3 *connection){
    char query[] ="CREATE TABLE IF NOT EXISTS account(id INTEGER PRIMARY KEY autoincrement,name varchar(250),age int,phone varchar(10));\
                    INSERT INTO account(name,age,phone) VALUES('kiran',12,98000); \
                    SELECT * from account;"; 
   return sqlite3_exec(connection,query,callback,NULL,NULL);
}


int main(void){
    Database db = select_database(DB_SQLITE3);
    db.logger = logger;
    // int status = db.init(db,"database.db",NULL,NULL);
    int status = db.init_v2(db,"database.db",NULL,NULL,SQLITE_OPEN_READWRITE);
    sqlite3 *connection = malloc(sizeof(sqlite3*));
    status = sqlite3_open_v2("database.db",&connection,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,"unix-dotfile");; 
    func(connection);
    
    // db.__connection = connection;
    // db.execute_multi(db,query,callback,NULL);
    // db.close_connection(db);
    // printf("was db intialized %d %d \n",db.__is_initialized,status);
    return 0;
}