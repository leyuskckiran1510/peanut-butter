
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TABLES_AT_ONCE 10
#define MAX_ATTRIBUTES 50
#define MAX_PROPERTY 10
#define MAX_NAME_SIZE  30

char *TRANSLATION_MAP[] = {
    "pk","PRIMARY KEY",
    "int","INTEGER",
    "string","varchar",
};
char table_head[] = "CREATE TABLE IF NOT EXISTS";

typedef struct {
    char *name;
    char **property;
    int property_count;
}Attributes;

typedef struct {
    char *name;
    Attributes *attributes;
} Table;
/*

*wadawdawdwad*/
// CREATE TABLE IF NOT EXISTS account(id INTEGER PRIMARY KEY autoincrement,username varchar(250), password varchar(10));
// Account{
//     id PK int,
//     username string(250),
//     password string(250),
// }
void  consume_comments(FILE *fp){
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

int is_alpha(const char c ){
    return ('A'<=c && 'Z'>=c ) || ('a'<=c && 'z'>=c) || (c=='_');
}

int is_alpha_numeric(const char c){
    return is_alpha(c) || ('0'<=c && '9'>=c);
}

int is_spaces(const char c){
    return c=='\n' || c==' ' || c == '\t' || c == '\v' || c == '\f';
}

char consume_word(FILE *fp,char *buffer){
    int buffer_idx= 0;
    fseek(fp,-1,SEEK_CUR);
    char cur_char = fgetc(fp);
    while(cur_char){
        cur_char = fgetc(fp);
        if(is_alpha(cur_char) || (buffer_idx > 0 && is_alpha_numeric(cur_char))){
                buffer[buffer_idx++] = cur_char;
        }else if(is_spaces(cur_char) && buffer_idx ==0 )
            continue;
        else
            break;
    }
    return cur_char;
}

char consume_property(FILE *fp,char *buffer){
    int buffer_idx= 0;
    fseek(fp,-1,SEEK_CUR);
    char cur_char = fgetc(fp);
    while(cur_char){
        cur_char = fgetc(fp);
        if(is_spaces(cur_char) && buffer_idx ==0 )
            continue;
        if(is_spaces(cur_char) || cur_char==',' || cur_char=='}' )
            break;
        buffer[buffer_idx++] = cur_char;
        
    }
    return cur_char;
}


void * malloc_2d(int dim,int member,size_t size){
    void **array = calloc(dim,sizeof(void *));
    for(int i=0;i<dim;i++){
        array[i] = calloc(member,size);
    }
    return array;
}

char consume_attribute(FILE *fp,Attributes *out_attribute){
    Attributes attribute = {
        .name = calloc(MAX_NAME_SIZE,sizeof(char)),
        .property = malloc_2d(MAX_PROPERTY,MAX_NAME_SIZE,sizeof(char)),
        .property_count = -1,
    };
    int name=0;
    fseek(fp,-1,SEEK_CUR);
    char cur_char = fgetc(fp);
    while(cur_char!=',' && cur_char!='}'){
        if(!name){
            cur_char = consume_word(fp,attribute.name);
            name=1;
        }
        if(name){
            cur_char = consume_property(fp,attribute.property[++attribute.property_count]);
            printf("%s -] %s\n",attribute.name,attribute.property[attribute.property_count]);
            if(cur_char==',')
                cur_char=fgetc(fp);
        }
    }
    memcpy(out_attribute,&attribute,sizeof(Attributes));
    return cur_char;

}

Table parse_table(FILE *fp){
    Table table = {
        .name = calloc(MAX_NAME_SIZE,sizeof(char)),
        .attributes = calloc(MAX_ATTRIBUTES,sizeof(Attributes)),
    };
    int table_name = 0;
    int column=0;
    fseek(fp,-1,SEEK_CUR);
    char cur_char = fgetc(fp);
    while(cur_char > 0 && cur_char!='}' ){
        if(cur_char=='/'){
            consume_comments(fp);
        }
        if(!table_name){
            cur_char = consume_word(fp,table.name);
            printf("==Table name== %s === \n",table.name);
            table_name = 1;
        }
        if(table_name){
           cur_char = consume_attribute(fp,&table.attributes[column++]);
        }
    }
    return table;
}

Table * parse_tables(FILE *fp,int *out_table_count){
    Table *tables = calloc(MAX_TABLES_AT_ONCE,sizeof(Table));
    int table_count=0;
    while(!feof(fp)){
        char cur_char = fgetc(fp);
        if(cur_char=='/'){
            consume_comments(fp);
        }
        else{
            tables[table_count++] = parse_table(fp);
            int multiplier = (table_count/MAX_TABLES_AT_ONCE)+1;
            if(table_count>=MAX_TABLES_AT_ONCE*multiplier){
                Table *temp_tables = realloc(tables,sizeof(Table)*MAX_TABLES_AT_ONCE*(multiplier+1));
                if(temp_tables==NULL)
                    __assert_fail("Failed To allocated memory, Buy some RAM!!!",__FILE__,__LINE__,__ASSERT_FUNCTION);
                tables = temp_tables;
            }
        }
    }
    *out_table_count = table_count-1;
    return tables;
}

void print_tables(Table *tables,int count){
    for (int i = 0; i < count; ++i){
        printf("%s\n",tables[i].name);
    }
}


int main(){
    FILE *fp = fopen("src/migration.pbsql","r");
    int table_count = 0;
    Table *tables = parse_tables(fp,&table_count);
    print_tables(tables,table_count);
    fclose(fp);

}