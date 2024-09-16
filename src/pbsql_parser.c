#include "database.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TABLES_AT_ONCE 10
#define MAX_ATTRIBUTES 50
#define MAX_PROPERTY 10
#define MAX_NAME_SIZE  30
#define __SOME_OFFSET  60
#define MAX_TABLE_PREP_LENGTH MAX_NAME_SIZE*(MAX_PROPERTY)*(MAX_ATTRIBUTES) + __SOME_OFFSET

#ifndef free 
#define free(x)  {if(x!=NULL){free(x);x=NULL;}}
#endif

#define static 

#define PBSQL_IMPLEMENTATION
#ifdef PBSQL_IMPLEMENTATION

static char *TRANSLATION_MAP_KEYS[] = {
    "pk",
    "int",
    "string",
};
static char *TRANSLATION_MAP_VALUES[] = {
    "PRIMARY KEY AUTOINCREMENT",
    "INTEGER",
    "varchar",
};
static int TRANSLATION_MAP_COUNT = 3;

static char CREATE_TABLE_QUERY_HEAD[] = "CREATE TABLE IF NOT EXISTS";

typedef struct {
    char *name;
    char **property;
    int property_count;
}Attributes;

typedef struct {
    char *name;
    Attributes *attributes;
    int attribute_count;
    char *init_query;
} Table;

static Table *tables = NULL;

static void to_lower_case(char *word,int len){
    if(len<0) len = strlen(word);
    while(len>=0){
        word[len] = tolower(word[len]);
        len--;
    };
}

static char *translate_if_necessary(char *buffer){
    int len = strlen(buffer);
    to_lower_case(buffer,len);
    for(int i=0;i<TRANSLATION_MAP_COUNT;i++){
        // printf("%s -> %s  [%d]\n ",TRANSLATION_MAP_KEYS[i],buffer,strncmp(TRANSLATION_MAP_KEYS[i],buffer,len));
        if(!strncmp(TRANSLATION_MAP_KEYS[i],buffer,strlen(TRANSLATION_MAP_KEYS[i]))){
            char temp_buffer[MAX_NAME_SIZE];
            int index=0;
            while(buffer[index] && buffer[++index]!='(');
            if(buffer[index]=='(')
                memcpy(temp_buffer,(buffer+index),len-index);
            memcpy(buffer,TRANSLATION_MAP_VALUES[i],strlen(TRANSLATION_MAP_VALUES[i]));
            memcpy((buffer+strlen(TRANSLATION_MAP_VALUES[i])),temp_buffer,len-index);
        }
    }
    return buffer;
}


static void print_table(Table table){
    printf("Name:- %s {\n",table.name);
    for (int i = 0; i < table.attribute_count; ++i){
        printf("\t%s ",table.attributes[i].name);
        for (int j = 0; j <= table.attributes[i].property_count; ++j){
            printf("%s ",table.attributes[i].property[j]);
        }
        printf(",\n");
    }
    printf("}\n");

}

static void print_tables(Table *tables,int count){
    for (int i = 0; i < count; ++i){
        print_table(tables[i]);
    }
}

static void prep_table_query(Table *table){
    // CREATE TABLE IF NOT EXISTS %s ( ...... );
    table->init_query = calloc(MAX_TABLE_PREP_LENGTH,sizeof(char));
    int index=0;
    index = sprintf(table->init_query,"%s %s ( ",CREATE_TABLE_QUERY_HEAD,table->name);
    for (int i = 0; i < table->attribute_count; ++i){
        index += sprintf((table->init_query+index)," %s ",table->attributes[i].name);
        for (int j = 0; j <= table->attributes[i].property_count; ++j){
            index += sprintf((table->init_query+index)," %s ",translate_if_necessary(table->attributes[i].property[j]));
        }
        if(i<table->attribute_count-1)
            index += sprintf((table->init_query+index),",");

    }
    index += sprintf((table->init_query+index),");");
    printf(" %s \n",table->init_query);
}

static void prep_tables_query(Table *tables,int count){
    for (int i = 0; i < count; ++i){
        prep_table_query(&tables[i]);
    }
}

static void free_table(Table table){
    free(table.name);
    for (int i = 0; i < table.attribute_count; ++i){
        free(table.attributes[i].name);
        for (int j = 0; j <= table.attributes[i].property_count; ++j){
            free(table.attributes[i].property[j]);
        }
        free(table.attributes[i].property);
     }
    free(table.attributes);

}
static void free_tables(Table *tables,int count){
    for (int i = 0; i < count; ++i){
        free_table(tables[i]);
    }
}

static void  consume_comments(FILE *fp){
    char cur_char = fgetc(fp);
    // single line comment
    if(cur_char=='/')
        while(!feof(fp) && fgetc(fp)!='\n');
    // multiline comment
    if(cur_char=='*'){
        while(!feof(fp)){
            cur_char = fgetc(fp);
            if(cur_char=='*'){
                while(!feof(fp)&&cur_char=='*'){
                    cur_char=fgetc(fp);
                }
                if(cur_char=='/'){
                    return;
                }
            }
        }
    }
}

static int is_alpha(const char c ){
    return ('A'<=c && 'Z'>=c ) || ('a'<=c && 'z'>=c) || (c=='_');
}

static int is_alpha_numeric(const char c){
    return is_alpha(c) || ('0'<=c && '9'>=c);
}

static int is_spaces(const char c){
    return c=='\n' || c==' ' || c == '\t' || c == '\v' || c == '\f';
}

static char consume_word(FILE *fp,char *buffer){
    int buffer_idx= 0;
    fseek(fp,-1,SEEK_CUR);
    char cur_char = fgetc(fp);
    while(cur_char&& !feof(fp)){
        if(is_alpha(cur_char) || (buffer_idx > 0 && is_alpha_numeric(cur_char))){
                buffer[buffer_idx++] = cur_char;
                cur_char = fgetc(fp);
            continue;
        }else if(is_spaces(cur_char) && buffer_idx ==0 ){
            cur_char = fgetc(fp);
            continue;
        }
        break;
    }
    return cur_char;
}

static char consume_property(FILE *fp,char *buffer){
    int buffer_idx= 0;
    fseek(fp,-1,SEEK_CUR);
    char cur_char = fgetc(fp);
    while(cur_char&& !feof(fp)){
        cur_char = fgetc(fp);
        if(is_spaces(cur_char) && buffer_idx ==0 )
            continue;
        if(is_spaces(cur_char) || cur_char==',' || cur_char=='}' )
            break;
        buffer[buffer_idx++] = cur_char;
        
    }
    return cur_char;
}


static char consume_spaces(FILE *fp){
    char cur_char = fgetc(fp);
    while(is_spaces(cur_char)&& !feof(fp))cur_char=fgetc(fp);
    fseek(fp,-1,SEEK_CUR);
    return cur_char;
};

static void consume_opening_braces(FILE *fp){
    fseek(fp,-1,SEEK_CUR);
    while(fgetc(fp)!='{' && !feof(fp));
    fgetc(fp);
}

static void * malloc_2d(int dim,int member,size_t size){
    void **array = calloc(dim,sizeof(void *));
    for(int i=0;i<dim;i++){
        array[i] = calloc(member,size);
    }
    return array;
}

static char consume_attribute(FILE *fp,Attributes *out_attribute){
    Attributes attribute = {
        .name = calloc(MAX_NAME_SIZE,sizeof(char)),
        .property = malloc_2d(MAX_PROPERTY,MAX_NAME_SIZE,sizeof(char)),
        .property_count = -1,
    };
    int name=0;
    fseek(fp,-1,SEEK_CUR);
    char cur_char = fgetc(fp);
    while(cur_char!=',' && cur_char!='}' && !feof(fp)){
        if(!name){
            cur_char = consume_word(fp,attribute.name);
            name=1;
        }
        if(name){
            cur_char = consume_property(fp,attribute.property[++attribute.property_count]);
        }
    }
    // consume final , or } 
    cur_char = fgetc(fp);
    memcpy(out_attribute,&attribute,sizeof(Attributes));
    return cur_char;

}

static Table parse_table(FILE *fp){
    Table table = {
        .name = calloc(MAX_NAME_SIZE,sizeof(char)),
        .attributes = calloc(MAX_ATTRIBUTES,sizeof(Attributes)),
        .attribute_count = 0,
    };
    int table_name = 0;
    fseek(fp,-1,SEEK_CUR);
    char cur_char = fgetc(fp);
    while(cur_char > 0 && cur_char!='}' ){
        if(cur_char=='/'){
            consume_comments(fp);
        }
        if(!table_name){
            cur_char = consume_word(fp,table.name);
            if(feof(fp))
                break;
            if(is_spaces(cur_char))
                cur_char = consume_spaces(fp);
            consume_opening_braces(fp);
            table_name = 1;
        }
        if(table_name){
           cur_char = consume_attribute(fp,&table.attributes[table.attribute_count++]);
           if(is_spaces(cur_char))
                cur_char = consume_spaces(fp);
        }
    }
    if(!strlen(table.name)){
        free(table.name);
        free(table.attributes);
    }
    return table;

}

static Table * parse_tables(FILE *fp,int *out_table_count){
    Table *tables = calloc(MAX_TABLES_AT_ONCE,sizeof(Table));
    int table_count=0;
    while(!feof(fp)){
        char cur_char = fgetc(fp);
        if(cur_char=='/'){
            consume_comments(fp);
        }
        else{
            Table temp_table = parse_table(fp);
            if(temp_table.name==NULL) continue;
            tables[table_count++]=temp_table;
            int multiplier = (table_count/MAX_TABLES_AT_ONCE)+1;
            if(table_count>=MAX_TABLES_AT_ONCE*multiplier){
                Table *temp_tables = realloc(tables,sizeof(Table)*MAX_TABLES_AT_ONCE*(multiplier+1));
                if(temp_tables==NULL)
                    __assert_fail("Failed To allocated memory, Buy some RAM!!!",__FILE__,__LINE__,__ASSERT_FUNCTION);
                tables = temp_tables;
            }
        }
    }
    *out_table_count = table_count;
    return tables;
}

static void lcl_callback(void *a,Row row){
    (void)row;
    (void)a;
}

static int ll_loger(const char *msg){
    return printf("[log] %s\n",msg);

}

void do_migration(Database db,int debug,int *table_count){
    FILE *fp = fopen("src/migration.pbsql","r");
    tables = parse_tables(fp,table_count);
    prep_tables_query(tables,(*table_count));
    if(db->logger)
        db->logger("Migrating tables...");
    for(int i=0;i< (*table_count);i++){
        if(db->logger){
            db->logger(tables[i].name);
            db->logger(tables[i].init_query);
        }
        database_execute(db,tables[i].init_query,lcl_callback,NULL);
    }
    if(debug){
        print_tables(tables,(*table_count));
    }
    fclose(fp);
}

#define     NOT   " NOT "
#define     AND   " AND " 
#define     OR    " OR " 
#define     XOR   " XOR " 

typedef enum{
    CRUD_READ_SINGLE=0,
    CRUD_READ_ALL,
    CRUD_INSERT,
    CRUD_UPDATE,
    CRUD_DELETE,
    CRUD_DELETE_SAFE,
}CRUD_TYPE;

#define SEPRATE_VALUES_BY_COMMA "\x01" //   Start of Heading character
#define WRAP_VALUES_SINGLE_QUOTE "\x02" // Start of Text character
#define FLAG_RST  "\x03" // End of Text Character
#define WHERE(...) " WHERE ",##__VA_ARGS__ 
#define IS(x)   " == " ,"'", x,"'"
#define LIKE(x)  " LIKE ","'", x,"'"
#define FOR(...) "(", SEPRATE_VALUES_BY_COMMA," " , ##__VA_ARGS__ , SEPRATE_VALUES_BY_COMMA ,") ",FLAG_RST
#define VALUES(...) "VALUES "," ( ",WRAP_VALUES_SINGLE_QUOTE," ", ##__VA_ARGS__,WRAP_VALUES_SINGLE_QUOTE,") ",FLAG_RST
#define SET(...) " SET ",##__VA_ARGS__
#define MAX_LONG_DB_QUERY 1024

Table  check_and_find_table(const char *table_name){
    int tbl_idx = 0;
    while(1){
        Table table = tables[tbl_idx++];
        assert(table.name!=NULL && "no such table ");
        if(!strcmp(table_name,table.name)) return table;
    }

}

int costume_copy(char*s1,char*s2){
    if(s2==NULL) return 0;
    int count = 0;
    while(s2[count]){
        s1[count]=s2[count];
        count++;
    }
    return count;
}
void put_char(char *buffer,int *offset,char __char){
    (buffer+(*offset))[0] =  __char;
    (*offset)++;
}


#define COMMA_FLAG        0b000000010
#define SINGLE_QUOTE_FLAG 0b000000001

char char_from_flag(int flag){
    if (flag & COMMA_FLAG)
        return ',';
    if (flag & SINGLE_QUOTE_FLAG)
        return '\'';
    return ' ';
}

void god_tier_query_builder(CRUD_TYPE crud_type ,char *table_name,...){
    (void) crud_type;
    (void) table_name;
    va_list vaargs;
    char *query_buffer = calloc(MAX_LONG_DB_QUERY,sizeof(char));
    int buffer_offset = 0;
    char *cur = NULL;
    int started = 0;
    Table table = check_and_find_table(table_name);
    CallbackWrapper callback;
    switch(crud_type){
    case CRUD_READ_ALL:
    case CRUD_READ_SINGLE:{
        char *query_head = "SELECT * FROM ";
        buffer_offset = costume_copy(query_buffer,query_head);
        break;
    }
    case CRUD_DELETE:
    case CRUD_DELETE_SAFE:{
        char *query_head = "DELETE FROM ";
        buffer_offset = costume_copy(query_buffer,query_head);
        break;
    }
    case CRUD_INSERT:{
        char *query_head = "INSERT INTO ";
        buffer_offset = costume_copy(query_buffer,query_head);
        break;
    }
    case CRUD_UPDATE:{
        char *query_head = "UPDATE "; 
        buffer_offset = costume_copy(query_buffer,query_head);
        break;
    }
    }

    va_start(vaargs,table_name);
    
    if(crud_type == CRUD_READ_ALL){
        callback = va_arg(vaargs,CallbackWrapper);
    }

    buffer_offset += costume_copy((query_buffer+buffer_offset),table_name);
    put_char(query_buffer,&buffer_offset,' ');
    int char_to_append_flag = 0;
    while(cur || (!cur && !started++)){
        cur = va_arg (vaargs,void *);
        if ( cur && cur[0] == FLAG_RST[0]){
            char_to_append_flag = 0;
            continue;
        }
        if(cur && cur[0] == WRAP_VALUES_SINGLE_QUOTE[0]){
            char_to_append_flag ^= SINGLE_QUOTE_FLAG;
            continue;
        }
        if(cur && cur[0] == SEPRATE_VALUES_BY_COMMA[0]){
            char_to_append_flag ^= COMMA_FLAG;
            continue;
        }
        if(char_to_append_flag){
            int temp = char_to_append_flag;
            int count = 0;
                printf("%b\n",temp);
            while(temp){
                put_char(query_buffer,&buffer_offset,char_from_flag(temp));
                temp>>=count;
                temp<<=count++;
            }
        }
        buffer_offset += costume_copy((query_buffer+buffer_offset),cur);
        // if(char_to_append_flag){
        //     int temp = char_to_append_flag;
        //     int count = 0;
        //     while(temp){
        //         temp>>=count;
        //         temp<<=count++;
        //         put_char(query_buffer,&buffer_offset,char_from_flag(temp));
        //     }
        // }
    }
    printf(" Final Query [ %s ]\n",query_buffer);
    put_char(query_buffer,&buffer_offset,';');
    (query_buffer+buffer_offset)[0] = '\0';
    va_end(vaargs);
    free(query_buffer);
}

#define database_find_in(table_name,...) god_tier_query_builder(CRUD_READ_SINGLE,(table_name),##__VA_ARGS__,NULL)
#define database_findall_in(table_name,...) god_tier_query_builder(CRUD_READ_ALL,(table_name),##__VA_ARGS__,NULL)
#define database_add_in(table_name,...) god_tier_query_builder(CRUD_INSERT,(table_name),##__VA_ARGS__,NULL)
#define database_update_in(table_name,...) god_tier_query_builder(CRUD_UPDATE,(table_name),##__VA_ARGS__,NULL)
#define database_remove_in(table_name,...) god_tier_query_builder(CRUD_DELETE_SAFE,(table_name),##__VA_ARGS__,NULL)
#define database_purge_in(table_name,...) god_tier_query_builder(CRUD_DELETE,(table_name),##__VA_ARGS__,NULL)

int main(){
    Database db = database_select(DB_SQLITE3);
    int table_count = 0;
    database_init(db,"database1.db",NULL,NULL,SQLITE_OPEN_READWRITE);
    database_set_logger(db,ll_loger);
    do_migration(db,0,&table_count);

    char username[] = "tester";
    char password[] = "password";
    CallbackWrapper callback;
    database_find_in("Account",WHERE("username",IS(username),AND,"password",IS(password)));
    // database_findall_in("Account",callback,WHERE(username,IS("tester"),AND,password,IS("password")));
    database_add_in("Account",FOR("username","password"),VALUES(username,password));
    // database_update_in("Account",WHERE(username,LIKE("a%")),SET(FOR(username,"unknown"),FOR(role,"admin")));
    database_close(db);
    free_tables(tables,table_count);
}

#endif