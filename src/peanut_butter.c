#define __PB_DOT_C__
#define LOG_LEVEL 4
#include "logger.h"
// standard headers
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// included headers
#include "../include/civetweb.h"

// costume headers
#include "mime_map.h"
#include "peanut_butter.h"

#define MAX_SIZE 1024
#define MAX_DIR_DEPTH 10
#define URL_ARG_STRING_INCR_RATE 10
#define MAX_READ_CHUNK 1024 * 3  // 3kb
#define MAX_URL_QUERY_DATA 100
#define USER_DATA_CALLBACKS_AT_ONCE 10

#define IS_SEP(x) (((x) == '/') || ((x) == '\\'))

#define and &&
#define or ||
#define xor ^

char *__pb_sec_headers;

void PB(add_route(char *url, ViewCallback callback)) {
  log_debug("Added Route '%s' ", url);
  Route route = {.url = url, .callback = callback};
  ROUTE_TABLE.routes[ROUTE_TABLE.count] = route;
  ROUTE_TABLE.count++;
}

void PB(add_var_route(char *var_route, ViewCallbackArgs callback)) {
  log_debug("Added Route '%s' ", var_route);
  Route route = {.url = var_route, .callbackargs = callback};
  VAR_ROUTE_TABLE.routes[VAR_ROUTE_TABLE.count] = route;
  VAR_ROUTE_TABLE.count++;
}

void free_url_query(void *v_quires) {
  UrlQueries *quires = v_quires;
  if (quires == NULL) {
    return;
  };

  for (int i = 0; i < quires->length; ++i) {
    // queries starting with '_' are private pointers
    // so who ever defines frees it, don't try to GC
    if (quires->queries[i].name[0] == '_') continue;

    if (quires->queries[i].name != NULL) free(quires->queries[i].name);

    if (quires->queries[i].value != NULL) free(quires->queries[i].value);
  }
  free(quires);
}

UserData *get_user_data(Request request) {
  UserData *udt = mg_get_user_connection_data(request);
  if (udt == NULL) {
    log_debug("Allocating new UserData for request") UserData *data =
        calloc(sizeof(UserData), 1);
    data->callback_pointers =
        calloc(sizeof(void *), USER_DATA_CALLBACKS_AT_ONCE);
    data->data_pointer = calloc(sizeof(void *), USER_DATA_CALLBACKS_AT_ONCE);
    data->count = 0;
    udt = data;
  } else {
    // if userdata is already allocated, now
    // increase the size if needed
    int mutiplier = ((udt->count + 1) / USER_DATA_CALLBACKS_AT_ONCE) + 1;
    if (udt->count >= USER_DATA_CALLBACKS_AT_ONCE * mutiplier) {
      log_debug("Re-allocating additional UserData for request")
          udt->callback_pointers =
          realloc(udt->callback_pointers,
                  sizeof(void *) * USER_DATA_CALLBACKS_AT_ONCE * mutiplier);
      udt->data_pointer =
          realloc(udt->data_pointer,
                  sizeof(void *) * USER_DATA_CALLBACKS_AT_ONCE * mutiplier);
    }
  }
  log_debug("Sucess.") return udt;
}

size_t sizeof_file(const char *file_name){
  FILE *fp = fopen(file_name,"r");
  if(fp==NULL) return -1;
  fseek(fp,0,SEEK_SET);
  size_t cur = ftell(fp);
  fseek(fp,0,SEEK_END);
  size_t size = ftell(fp)-cur;
  fclose(fp);
  return size;
}

void query_track(Request request, UrlQueries *queries) {
  UserData *udt = get_user_data(request);
  udt->data_pointer[udt->count] = queries;
  udt->callback_pointers[udt->count++] = &free_url_query;
  mg_set_user_connection_data(request, udt);
}

void remove_temp_folder(conRequest request) {
  char foldername[MAX_TEMP_FOLDER_FILE_SIZE];
  sprintf(foldername, TEMP_FOLDER_FMT(request));
  if (remove(foldername)) {
    log_error("failed to remove temp folder (%s)\n\t=>%s", foldername,
              strerror(errno));
  }
}

void free_per_request(conRequest request) {
  UserData *udt = mg_get_user_connection_data(request);
  if (udt == NULL) return;
  for (int i = 0; i < udt->count; ++i) {
    udt->callback_pointers[i](udt->data_pointer[i]);
  }

  free(udt->callback_pointers);
  free(udt->data_pointer);
  free(udt);
  remove_temp_folder(request);
}

char *virtual_path_traverse(const char *file_path) {
  // to prevent file_traversal attacks
  char *clean_path = malloc(strlen(file_path) + 1);
  uint16_t seprators[MAX_DIR_DEPTH] = {0};
  uint16_t ptr = 0, sep_count = 1, cpy_pointer = 0;
  while (file_path[ptr]) {
    // unix like systemfilepath    , windows file system
    if (IS_SEP(file_path[ptr]) && file_path[ptr + 1] != '.') {
      // keep track of seprators for easier traverse back
      seprators[sep_count++] = cpy_pointer;
      // consume unnecessay '/' or '\'
      while (IS_SEP(file_path[ptr + 1])) {
        ptr++;
      }

    } else if (file_path[ptr] == '.' && file_path[ptr + 1] == '.') {
      sep_count = sep_count > 1 ? sep_count - 1 : 0;
      cpy_pointer = seprators[sep_count];
      ptr += 2;  // conmsue '../'
      while (IS_SEP(file_path[ptr + 1])) {
        ptr++;
      }

    }
    // consume single dots also
    else if (file_path[ptr] == '.' && IS_SEP(file_path[ptr + 1])) {
      ptr += 1;  // consume './'
      while (IS_SEP(file_path[ptr + 1])) {
        ptr++;
      }

      ptr++;  // increment pointer to skill '/'
      continue;
    }

    if (!cpy_pointer && (file_path[ptr] == '/' || file_path[ptr + 1] == '\\')) {
      ptr++;
      continue;
    }
    clean_path[cpy_pointer] = file_path[ptr];
    cpy_pointer++;
    clean_path[cpy_pointer] = 0;
    ptr++;
  }
  return clean_path;
}

const char *get_mime_type(const char *file_path) {
  char file_ext[10];
  uint16_t path_ptr = 0, ext_ptr = 0;
  while (file_path[path_ptr] != '.' && file_path[path_ptr]) path_ptr++;
  path_ptr++;  // consume '.'
  while (file_path[path_ptr] && ext_ptr < 10)
    file_ext[ext_ptr++] = file_path[path_ptr++];
  file_ext[ext_ptr] = 0;
  MIME_TYPE_MATCH(file_ext);
}

static int serve_file(Request request, const char *file_path) {
  char *path = virtual_path_traverse(file_path);
  FILE *fp = fopen(path, "r");
  if (fp == NULL) {
    return 1;
  }
  fclose(fp);

  const char *mime_type = get_mime_type(path);
  log_debug("Serving File... %s [%s]", path, mime_type);
  mg_send_mime_file(request, path, mime_type);
  free(path);
  return 0;
}

int consume_digit(const char *str, UrlVariable *arg) {
  int count = 0;
  arg->value = 0;
  arg->type = UAT_INTEGER;
  while (str[count] >= '0' && str[count] <= '9') {
    arg->value = arg->value * 10 + str[count] - '0';
    count++;
  }
  return count;
}
int consume_char(const char *str, UrlVariable *arg) {
  arg->c_value = str[0];
  arg->type = UAT_CHARACTER;
  return 1;
}

int print_able_range(char letter) {
  // printable character range
  // SPACE to ~
  return 32 <= letter && letter <= 126;
}

int consume_str(const char *str, UrlVariable *arg) {
  char *new_str = calloc(URL_ARG_STRING_INCR_RATE, 1);
  int count = 0, resized = 0;

  arg->type = UAT_STRING;

  while (print_able_range(str[count]) && str[count] != '/') {
    if (count >= URL_ARG_STRING_INCR_RATE * (resized + 1)) {
      new_str = realloc(new_str, URL_ARG_STRING_INCR_RATE * (resized + 2));
      resized++;
    }
    new_str[count] = str[count];
    count++;
  }
  arg->s_value = new_str;
  return count;
}
int consume_float(const char *str, UrlVariable *arg) {
  float value = 0.0f;
  int count = 0;
  int found_period = 0;
  int after_period = 1;
  while ((str[count] >= '0' && str[count] <= '9') || str[count] == '.') {
    if (str[count] == '.') {
      // if period was already found then exit early
      if (found_period) return 0;
      found_period = 1;
      count++;
      continue;
    }
    if (!found_period) {
      value = value * 10 + str[count] - '0';
    } else {
      value += (str[count] - '0') / (10.0f * after_period);
      after_period *= 10;
    }
    count++;
  }
  arg->f_value = value;
  arg->type = UAT_FLOAT;
  return count;
}

int consume_double(const char *str, UrlVariable *arg) {
  double value = 0.0f;
  int count = 0;
  int found_period = 0;
  int after_period = 1;
  while ((str[count] >= '0' && str[count] <= '9') || str[count] == '.') {
    if (str[count] == '.') {
      // if period was already found then exit early
      if (found_period) return 0;
      found_period = 1;
      count++;
      continue;
    }
    if (!found_period) {
      value = value * 10 + str[count] - '0';
    } else {
      after_period *= 10;
      value += (str[count] - '0') / (10.0f * after_period);
    }
    count++;
  }
  arg->d_value = value;
  arg->type = UAT_DOUBLE;
  return count;
  return 0;
}

int consume_arg(char type, const char *str, UrlVariable *arg) {
  switch (type) {
    case 'd':
      return consume_digit(str, arg);
    case 's':
      return consume_str(str, arg);
    case 'c':
      return consume_char(str, arg);
    case 'f':
      return consume_float(str, arg);
    case 'l':
      return consume_double(str, arg);
    default:
      return 0;
  }
}

int count_fmt_args(const char *fmt_url) {
  char *ref = (char *)fmt_url;
  int count = 0;
  while (ref[0]) {
    switch (ref[0]) {
      case '%':
        ref++;
        if (ref[0] != '%') count++;
    }
    ref++;
  }
  return count;
}

UrlVariables find_if_match(const char *route_url, const char *url) {
  // route copy init
  int route_len = strlen(route_url);
  char *fmt_url = malloc(route_len);
  memcpy(fmt_url, route_url, sizeof(char) * route_len);
  int url_ptr = 0;

  // args intialization
  UrlVariables args = INIT_URL_ARGS();
  int total_args = count_fmt_args(fmt_url);

  if (total_args == 0) return args;

  args.args = malloc(sizeof(UrlVariable) * total_args);
  args.length = total_args;
  int cur_arg = 0;

  // fill args from url using route_structure
  while (fmt_url[0]) {
    if (fmt_url[0] == '%' && fmt_url[1] != '%') {
      url_ptr += consume_arg(fmt_url[1], (url + url_ptr), &args.args[cur_arg]);
      fmt_url++;
      fmt_url++;
      cur_arg++;
    } else if (fmt_url[0] == (url + url_ptr)[0]) {
      fmt_url++;
      url_ptr++;
    } else {
      args.length = 0;
      if (args.args != NULL) free(args.args);
      return args;
    }
  }
  return args;
}

void free_url_args(UrlVariables args) {
  for (int i = 0; i < args.length; ++i) {
    if (args.args[i].type == UAT_STRING && args.args[i].s_value != NULL) {
      free(args.args[i].s_value);
    }
  }
  free(args.args);
}

void free_template_var(TemplateVars templ_vars) {
  if (templ_vars.templ != NULL) {
    free(templ_vars.templ);
  }
}

static int begin_request_handler(struct mg_connection *conn) {
  const struct mg_request_info *request_info = mg_get_request_info(conn);
  mg_response_header_start(conn,200);
  int header_status = mg_response_header_add_lines(conn, __pb_sec_headers);
  header_status = mg_response_header_add(conn,"hello","value",5);
  log_debug("header_status {%d}",header_status);
  
  log_debug("New connection at %s", request_info->local_uri);

  // normal url search
  for (size_t i = 0; i < ROUTE_TABLE.count; ++i) {
    if (!strcmp(request_info->local_uri, ROUTE_TABLE.routes[i].url)) {
      ROUTE_TABLE.routes[i].callback(conn);
      return 1;
    }
  }
  // variabled url search
  for (size_t i = 0; i < VAR_ROUTE_TABLE.count; ++i) {
    UrlVariables args =
        find_if_match(VAR_ROUTE_TABLE.routes[i].url, request_info->local_uri);
    if (args.length > 0) {
      VAR_ROUTE_TABLE.routes[i].callbackargs(conn, args);
      free_url_args(args);
      return 1;
    }
  }

  if (!serve_file(conn, (request_info->local_uri + 1))) {
    return 1;
  }
  _render_404(conn, "htmls/404.html");
  return 1;
}

void _render_html(Request request, const char *file_name) {
  if (!serve_file(request, file_name)) {
    return;
  }
  log_error("Failed To Render %s in (%s)", file_name, __func__);
  return;
}

void _render_404(Request request, const char *file_name) {
  mg_printf(request,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html;\r\n"
            "Connection: close\r\n"
            "Content-Length: %lu\r\n"
            "\r\n",sizeof_file(file_name));
  mg_send_file_body(request, file_name);
  return;
}

void _render_text(Request request, const char *text) {
  mg_printf(request,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %lu\r\n"
            "\r\n"
            "%s",
            strlen(text), text);
  return;
}

void _render_raw_text(Request request, const char *text) {
  mg_printf(request,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/text\r\n"
            "Content-Length: %lu\r\n"
            "\r\n"
            "%s",
            strlen(text), text);
  return;
}

int not_white_space(char c) {
  // ! to ~
  return c >= 33 and c <= 126;
}

int stamp_var(char *dest, size_t pos, UrlVariable var) {
  char *value_string = calloc(50, 1);
  char *temp = value_string;
  switch (var.type) {
    case UAT_STRING:
      free(value_string);
      value_string = var.s_value;
      temp = value_string;
      break;
    case UAT_i:
      sprintf(value_string, "%d", var.i_value);
      break;
    case UAT_f:
      sprintf(value_string, "%f", var.f_value);
      break;
    case UAT_d:
      sprintf(value_string, "%lf", var.d_value);
      break;
    case UAT_c:
      sprintf(value_string, "%c", var.c_value);
      break;
    case UAT_u:
      value_string[0] = 0;
      log_error("In stamp_var got unknown urlvaribale type ");
      break;
  }
  while (pos < MAX_READ_CHUNK and temp[0]) {
    dest[pos] = temp[0];
    pos++;
    temp++;
  }
  if (var.type != UAT_STRING) {
    free(value_string);
  }
  return pos;
}

int apply_template(char *from, char *to, int *write_count,
                   TemplateVars templ_vars) {
  uint16_t from_ptr = 0, to_ptr = 0, from_ptr_copy = 0;
  char name[MAX_TEMPL_VAR_NAME] = {0};

  while (from[from_ptr] and to_ptr < (MAX_READ_CHUNK - 5)) {
    // found two '{'
    from_ptr_copy = from_ptr;
    if (from[from_ptr] == '{' and from[from_ptr + 1] == '{') {
      uint8_t name_ptr = 0;
      from_ptr += 2;
      while (from[from_ptr] and from[from_ptr] != '}') {
        if (not_white_space(from[from_ptr])) {
          name[name_ptr++] = from[from_ptr];
        }
        from_ptr++;
      }

      // the read buffer ended before the template
      // could have been rendered/parsed
      if (!from[from_ptr]) {
        to[to_ptr] = 0;
        return from_ptr_copy;
      }
      name[name_ptr] = 0;
      from_ptr += 2;
      if (name_ptr > 0) {
        for (int i = 0; i < templ_vars.length; ++i) {
          if (!strcmp(templ_vars.templ[i].name, name)) {
            to_ptr = stamp_var(to, to_ptr, templ_vars.templ[i].value);
          }
        }
      }
    }
    to[to_ptr++] = from[from_ptr++];
  }
  to[to_ptr] = 0;
  *write_count = to_ptr;
  return from_ptr;
}

void _render_template(Request request, const char *file_name,
                      TemplateVars templ_vars) {
  FILE *tmp_fp;

  FILE *fp = fopen(file_name, "r");
  if (fp == NULL) {
    log_error("Failed to open [%s] ", file_name);
    return;
  }

  // convert the unique memory address to unique file name
  size_t tmp_file_id = *(size_t *)request;
  // max int has 10 digits for 32bit, and 20 digits for 64bits
  // so to  cover it MAX_TEMP_FOLDER_FILE_SIZE will be enough
  char tmp_file_name[MAX_TEMP_FOLDER_FILE_SIZE];
  sprintf(tmp_file_name, TEMP_TEMPL_FILE_PREFIX "%lu.html", tmp_file_id);

  tmp_fp = fopen(tmp_file_name, "w");
  if (tmp_fp == NULL) {
    log_error("Failed creating tmp file [%s] ", tmp_file_name);
  }

  char read_chunk[MAX_READ_CHUNK];
  char write_chunk[MAX_READ_CHUNK];
  int write_count = 0;

  int continue_from = 0, read_buff_offset = 0;
  while (!feof(fp)) {
    // try to read whole chunk like a single object
    int read_count = fread((read_chunk + read_buff_offset), 1,
                           MAX_READ_CHUNK - read_buff_offset - 1, fp);
    read_chunk[read_count] = 0;

    continue_from =
        apply_template(read_chunk, write_chunk, &write_count, templ_vars);
    fwrite(write_chunk, 1, write_count, tmp_fp);
    read_buff_offset = MAX_READ_CHUNK - continue_from - 1;
    memcpy(read_chunk, (read_chunk + continue_from), read_buff_offset);
  }
  fclose(fp);
  fclose(tmp_fp);
  log_debug("Serving Template file... [%s]", tmp_file_name);
  _render_html(request, tmp_file_name);
  remove(tmp_file_name);
  free_template_var(templ_vars);
  return;
}

UrlQueries *parse_query(Request request) {
  UrlQueries *urlqueries = calloc(1, sizeof(UrlQueries));
  const struct mg_request_info *request_info = mg_get_request_info(request);
  const char *query = request_info->query_string;
  if (query == NULL) {
    return urlqueries;
  }
  int query_len = strlen(query) + 1;
  char *decoded_query = calloc(1, query_len);
  mg_url_decode(query, query_len, decoded_query, query_len, 1);
  decoded_query[query_len - 1] = 0;
  int copy_ptr = 0;

  while (decoded_query[0]) {
    char *name = calloc(1, MAX_URL_QUERY_DATA);
    char *value = calloc(1, MAX_URL_QUERY_DATA);
    while (decoded_query[0] and decoded_query[0] != '=') {
      name[copy_ptr++] = decoded_query[0];
      decoded_query++;
    }
    if (decoded_query[0]) decoded_query++;  // consume '='
    copy_ptr = 0;
    while (decoded_query[0] and decoded_query[0] != '&') {
      value[copy_ptr++] = decoded_query[0];
      decoded_query++;
    }
    if (decoded_query[0]) decoded_query++;  // consume '&'
    copy_ptr = 0;
    urlqueries->queries[urlqueries->length].name = name;
    urlqueries->queries[urlqueries->length].value = value;
    urlqueries->length++;
  }
  return urlqueries;
}

char *query_file_search(UrlQueries *url_queries, char *name) {
  uint16_t name_len = strlen(name);
  for (uint16_t i = 0; i < url_queries->length; ++i) {
    // compare with user input name, as file_name ends with "\n"
    if (!strncmp(name, url_queries->queries[i].name, name_len) and
        url_queries->queries[i].name[name_len] ==
            10) {  // it needs to end with \n
      return url_queries->queries[i].value;
    }
  }
  return NULL;
}

char *query_search(UrlQueries *url_queries, char *name, char *default_value) {
  for (uint16_t i = 0; i < url_queries->length; ++i) {
    if (!strcmp(name, url_queries->queries[i].name)) {
      return url_queries->queries[i].value;
    }
  }
  return default_value;
}

void _redirect(Request request, char *to_url, uint16_t redirect_code) {
  if (mg_send_http_redirect(request, to_url, redirect_code) < 0) {
    log_error("Failed Redirecting to url [%s](%d)", to_url, redirect_code);
  }
  return;
}

const char *_get_method(Request request) {
  const struct mg_request_info *request_info = mg_get_request_info(request);
  return request_info->request_method;
};

void store_in_form_data(FormDatas *form_data, const char *key,
                        const char *value, int valuelen, bool is_file) {
  size_t keylen = strlen(key);

  char *_name = calloc(1, MAX_URL_QUERY_DATA);
  char *_value = calloc(1, valuelen + 1);

  memcpy(_name, key, keylen);

  if (is_file)
    *(_name + keylen) = 10;  // newline as we cannot put \n in key value

  memcpy(_value, value, valuelen);

  form_data->queries[form_data->length].name = _name;
  form_data->queries[form_data->length].value = _value;
  form_data->length++;
}

int field_found(const char *key, const char *filename, char *path,
                size_t pathlen, void *user_data) {
  if (filename[0]) {
    FormDatas *fd = user_data;
    char folder_path[MAX_TEMP_FOLDER_FILE_SIZE];
    sprintf(folder_path, TEMP_FOLDER_FMT(fd->queries[0].value));
    mkdir(folder_path, 0777);
    sprintf(path, "%s" OS_PATH_SEP "%s", folder_path, filename);
    store_in_form_data(user_data, key, path, pathlen, true);
    return MG_FORM_FIELD_STORAGE_STORE;
  }

  (void)pathlen;
  (void)key;
  (void)user_data;
  return MG_FORM_FIELD_STORAGE_GET;
}

int field_get(const char *key, const char *value, size_t valuelen,
              void *user_data) {
  FormDatas *form_data = user_data;
  store_in_form_data(form_data, key, value, valuelen, false);
  (void)user_data;
  return MG_FORM_FIELD_HANDLE_NEXT;
}
int field_store(const char *path, long long file_size, void *user_data) {
  (void)user_data;
  (void)file_size;
  return MG_FORM_FIELD_HANDLE_NEXT;
}

void remove_folder(void *blankpointer) {
  FormDatas *fd = blankpointer;
  for (int i = 0; i < fd->length; ++i) {
    char *name = fd->queries[i].name;
    uint16_t len = strlen(name);
    if (name[len - 1] == 10) {  // last element of name has '\n'[10 in ascii]
      if (remove(fd->queries[i].value)) {
        if (strcmp("No such file or directory", strerror(errno))) {
          log_error("could not remove %s file from temp folder",
                    fd->queries[i].value);
          log_error("%s", strerror(errno));
        }
      }
    }
  }
}

FormDatas *parse_form(Request request, bool save_file_bool) {
  (void)save_file_bool;
  FormDatas *form_data = calloc(1, sizeof(FormDatas));
  form_data->queries[0].name = "__request_pointer";
  form_data->queries[0].value = (char *)request;
  form_data->length = 1;
  struct mg_form_data_handler fdh = {
      .user_data = form_data,
      .field_found = &field_found,
      .field_get = &field_get,
      .field_store = &field_store,
  };

  if (!save_file_bool) {
    UserData *udt = get_user_data(request);
    udt->data_pointer[udt->count] = form_data;
    udt->callback_pointers[udt->count++] = &remove_folder;
    mg_set_user_connection_data(request, udt);
  }

  int total = mg_handle_form_request(request, &fdh);
  (void)total;
  if (form_data->length > 1) {
    // bring last element to first to override __Request_pointer
    form_data->queries[0] = form_data->queries[form_data->length - 1];
    form_data->length--;
    if (total != form_data->length) {
      log_error(
          "parsed form data is not equal to request form data ([%d != %d ])",
          form_data->length, total);
    }
    return form_data;
  }
  log_info("form was empty.");
  return NULL;
}

int log_message(conRequest req, const char *log) {
  // to aovid unused varibale warning
  (void)req;
  log_debug("%s", log);
  return 1;
}

int _server_run(char *port,char *sec_headers) {
  __pb_sec_headers = sec_headers;
  struct mg_context *ctx;
  struct mg_callbacks callbacks;
  // https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md#options-from-civetwebc
  const char *options[] = {"listening_ports", port, NULL};
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.begin_request = begin_request_handler;
  callbacks.connection_close = free_per_request;
  callbacks.log_message = log_message;
  ctx = mg_start(&callbacks, NULL, options);
  if (ctx != NULL) {
    log_info("Server Running at http://localhost:%s ...", port);
    getchar();
  }
  log_info("Server Stoped ");
  log_info("Exiting ... ");
  mg_stop(ctx);
  return 0;
}
