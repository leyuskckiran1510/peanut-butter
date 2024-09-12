#include "database.h"
#include <assert.h>
#include <ctype.h>
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
#define free(x)  if(x!=NULL){free(x);x=NULL;}
#endif

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

static char table_head[] = "CREATE TABLE IF NOT EXISTS";

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
    index = sprintf(table->init_query,"%s %s ( ",table_head,table->name);
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

void do_migration(Database db,int debug){
    FILE *fp = fopen("src/migration.pbsql","r");
    int table_count = 0;
    Table *tables = parse_tables(fp,&table_count);
    prep_tables_query(tables,table_count);
    db->logger("Migrating tables...");
    for(int i=0;i<table_count;i++){
        db->logger(tables[i].name);
        db->logger(tables[i].init_query);
        database_execute(db,tables[i].init_query,lcl_callback,NULL);
    }
    if(debug){
        print_tables(tables,table_count);
    }
    fclose(fp);
    free_tables(tables,table_count);
}


// int main(){
//     Database db = database_select(DB_SQLITE3);
//     database_init(db,"database1.db",NULL,NULL,SQLITE_OPEN_READWRITE);
//     database_set_logger(db,ll_loger);
//     do_migration(db,1);
//     database_close(db);
// }