#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define throw(x)   __assert_fail((x),__FILE__,__LINE__,__ASSERT_FUNCTION);

char SQL_SYNTAX_ERROR[] = "^~~~~~ SYNTAX ERROR HERE";

int callback(void* a1,int a2,char **a3,char**a4){
    (void)a1;
     for (int i = 0; i < a2; i++) {
        printf("%s = %s\n", a4[i], a3[i] ? a3[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int sqlite_init(Database db,const char *uri_with_port,const char* username,const char* password,int flags){
    (void)username;
    (void)password;
    int status = sqlite3_open_v2(uri_with_port,&db->__connection,flags | SQLITE_OPEN_CREATE,"unix-dotfile");
     if(status!=SQLITE_OK){
        if(db->logger){
            db->logger("failed to initialize sqlite with error");
            db->logger(strerror(errno));
        }
        return DB_FAILED_INIT;
    }
    return DB_SUCESS_INIT;
   
}
void sqlite_exec_error_log(Database db,char* sql_queries){
    db->logger("query execution failed:  with following exception");
    db->logger(sqlite3_errmsg((sqlite3 *)db->__connection));
    db->logger(sql_queries);
 
    int error_position = sqlite3_error_offset((sqlite3 *)db->__connection);
    if (error_position>=0){
        char *trackback = calloc(1, error_position+strlen(SQL_SYNTAX_ERROR)+1);
        int tb_index=0;
        while(tb_index<=error_position){
            trackback[tb_index]=' ';
            if(tb_index==error_position){
                int error_index = 0;
                while(SQL_SYNTAX_ERROR[error_index]){
                    trackback[tb_index] = SQL_SYNTAX_ERROR[error_index];
                    error_index++;
                    tb_index++;
                }
            }
            tb_index++;
        }
        trackback[tb_index-1]='\n';
        db->logger(trackback);
        free(trackback);
        trackback=NULL;
    }
}

int sqlite_execute_single(Database db,char * sql_query,DatabaseCallback callback,void* passthrough_data){
    char *error_buffer;
    int status = sqlite3_exec(db->__connection,sql_query,callback,passthrough_data,&error_buffer);
    if(status && error_buffer){
        if(db->logger){
            sqlite_exec_error_log(db, sql_query);
        }
        sqlite3_free(error_buffer);
        return DB_FAILED_INIT;
    }
    return DB_SUCESS_EXEC;
}

int sqlite_execute_multi(Database db,char * sql_queries,DatabaseCallback callback,void* passthrough_data){
    char holder=0;
    size_t till=0,from=0;
    while(sql_queries[from]){
        till=0;
        while((from + sql_queries + till) && *(from + sql_queries + till)!=';' ){
            till++;
        }
        if(*(from + sql_queries + till)==';') till++;
        holder = *(from + sql_queries + till) ;
        *(from + sql_queries + till) = 0;
        int status = sqlite_execute_single(db,(sql_queries+from),callback,passthrough_data);
        *(from + sql_queries + till) = holder;
        if(status!=DB_SUCESS_EXEC){
            return status;
        }
        from += till;
    }
    return DB_SUCESS_EXEC;
}


int sqlite_close_connection(Database db){
    int status = sqlite3_close((sqlite3 *)(db->__connection));
    if(status)
        return  DB_FAILED_CLOSE;
    return DB_SUCESS_CLOSE;
}

Database database_select(DatabaseType db_type){
    Database db = calloc(sizeof(DatabaseStruct),1);
    switch (db_type) {
       case DB_SQLITE3:{
        db->init = sqlite_init;
        db->execute_single = sqlite_execute_single;
        db->execute_multi = sqlite_execute_multi;
        db->close_connection = sqlite_close_connection;
        db->logger=NULL;
        break;
       }
    default:
        throw("Database must be one of these {DB_SQLITE3}");
    }
    return db;
}

int logger(const char *msg){
    printf("[logger] %s\n",msg);
    return 0;
}

int database_init(Database db,const char * uri,const char *username,const char*password,int flags){
    return db->init(db,uri,username,password,flags);
}

int database_execute(Database db,char * sql_queries,DatabaseCallback callback,void* passthrough_data){
    return db->execute_multi(db,sql_queries,callback,passthrough_data);
}

int database_close(Database db){
    return  db->close_connection(db);
}

void database_set_logger(Database db,int (*logger)(const char *msg)){
    db->logger = logger;
}

int main(void){
    Database db = database_select(DB_SQLITE3);
    database_set_logger(db,logger);
    database_init(db,"database.db",NULL,NULL,SQLITE_OPEN_READWRITE);
    char query[] ="CREATE TABLE IF NOT EXISTS account(id INTEGER PRIMARY KEY autoincrement,name varchar(250),age int,phone varchar(10));\
                    INSERT INTO account(name,age,phone) VALUES('kiran',12,98000); \
                    SELECT id from account;";
    database_execute(db,query,callback,NULL);
    database_close(db);
    return 0;
}