#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define throw(x) __assert_fail((x), __FILE__, __LINE__, __ASSERT_FUNCTION);
#define PRIVATE static

char SQL_SYNTAX_ERROR[] = "^~~~~~ ";

PRIVATE int db_callback(void *wrapper_pointer, int column_count,
                        char **column_values, char **column_names) {
  CallbackWrapper wrapper = *(CallbackWrapper *)wrapper_pointer;
  Row row = {
      .cols = column_count,
      .col_names = column_names,
      .col_values = column_values,
  };
  wrapper.callback(wrapper.passthrough_data, row);
  return 0;
}

PRIVATE int sqlite_init(Database db, const char *uri_with_port,
                        const char *username, const char *password, int flags) {
  (void)username;
  (void)password;
  int status = sqlite3_open_v2(uri_with_port, &db->__connection,
                               flags | SQLITE_OPEN_CREATE, "unix-dotfile");
  if (status != SQLITE_OK) {
    if (db->logger) {
      db->logger("failed to initialize sqlite with error");
      db->logger(strerror(errno));
    }
    return DB_FAILED_INIT;
  }
  db->__is_initialized = 1;
  return DB_SUCESS_INIT;
}
PRIVATE void sqlite_exec_error_log(Database db, char *sql_queries) {
  db->logger("query execution failed:  with following exception");
  db->logger(sqlite3_errmsg((sqlite3 *)db->__connection));
  db->logger(sql_queries);

  int error_position = sqlite3_error_offset((sqlite3 *)db->__connection);
  if (error_position >= 0) {
    char *trackback = calloc(1, error_position + strlen(SQL_SYNTAX_ERROR) + 1);
    int tb_index = 0;
    while (tb_index <= error_position) {
      trackback[tb_index] = ' ';
      if (tb_index == error_position) {
        int error_index = 0;
        while (SQL_SYNTAX_ERROR[error_index]) {
          trackback[tb_index] = SQL_SYNTAX_ERROR[error_index];
          error_index++;
          tb_index++;
        }
      }
      tb_index++;
    }
    trackback[tb_index - 1] = '\n';
    db->logger(trackback);
    free(trackback);
    trackback = NULL;
  }
}

PRIVATE int sqlite_execute_single(Database db, char *sql_query,
                                  DatabaseCallback callback,
                                  void *passthrough_data) {
  char *error_buffer;
  int status = sqlite3_exec(db->__connection, sql_query, callback,
                            passthrough_data, &error_buffer);
  if (status && error_buffer) {
    if (db->logger) {
      sqlite_exec_error_log(db, sql_query);
    }
    sqlite3_free(error_buffer);
    return DB_FAILED_INIT;
  }
  return DB_SUCESS_EXEC;
}

PRIVATE int sqlite_execute_multi(Database db, char *sql_queries,
                                 DatabaseCallback callback,
                                 void *passthrough_data) {
  size_t till = 0, from = 0;
  while (sql_queries[from]) {
    char holder = 0;
    till = 0;
    while ((from + sql_queries + till) && *(from + sql_queries + till) != ';') {
      till++;
    }
    if (*(from + sql_queries + till) == ';') till++;
    if(*(from + sql_queries + till)){
      holder = *(from + sql_queries + till);
      *(from + sql_queries + till) = 0;
    }
    int status = sqlite_execute_single(db, (sql_queries + from), callback,
                                       passthrough_data);
    if(holder)
      *(from + sql_queries + till) = holder;
    if (status != DB_SUCESS_EXEC) {
      return status;
    }
    from += till;
  }
  return DB_SUCESS_EXEC;
}

PRIVATE int sqlite_close_connection(Database db) {
  int status = sqlite3_close((sqlite3 *)(db->__connection));
  if (status) return DB_FAILED_CLOSE;
  return DB_SUCESS_CLOSE;
}

Database database_select(DatabaseType db_type) {
  // to remove warning
  (void)QUERY_FMT_BUFFER;

  Database db = calloc(sizeof(DatabaseStruct), 1);
  switch (db_type) {
    case DB_SQLITE3: {
      db->init = sqlite_init;
      db->execute_single = sqlite_execute_single;
      db->execute_multi = sqlite_execute_multi;
      db->close_connection = sqlite_close_connection;
      db->logger = NULL;
      break;
    }
    default:
      throw("Database must be one of these {DB_SQLITE3}");
  }
  return db;
}

// PRIVATE int db_logger(const char *msg) {
//   printf("[logger] %s\n", msg);
//   return 0;
// }

int database_init(Database db, const char *uri, const char *username,
                  const char *password, int flags) {
  return db->init(db, uri, username, password, flags);
}

int database_execute(Database db, char *sql_queries, RowCallback callback,
                     void *passthrough_data) {
  if (!db->__is_initialized) throw("Datbase not initialized");

  CallbackWrapper wrapper = {.passthrough_data = passthrough_data,
                             .callback = callback};
  return db->execute_multi(db, sql_queries, db_callback, &wrapper);
}

int database_close(Database db) {
  if (!db->__is_initialized) throw("Datbase not initialized");
  return db->close_connection(db);
}

void database_set_logger(Database db, int (*logger)(const char *msg)) {
  if(! db->__is_initialized)
    throw("Datbase not initialized");
  db->logger = logger;
}
