#include <assert.h>
#include<stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>
#include <netinet/in.h>
#include "utils.h"
// #include "threader.h"



#define PORT 1234
#define HOST "0.0.0.0"
#define BUFFER_SIZE 1024


typedef struct{
    char *data;
    size_t length;
} HTML_FILE;


HTML_FILE read_html(const char * file_name,const char * header,int header_size){
    HTML_FILE html_file;
    FILE *fp = fopen(file_name,"r");
    if(fp==NULL){
        assert(1!=1);
    }
    long cur = ftell(fp);
    fseek(fp,0,SEEK_END);
    long file_size = ftell(fp)-cur;
    char *buffer = malloc(file_size+1+header_size);
    char *buffer_init;
    memcpy(buffer,header,header_size);
    printf("\n====BUFFER===%s\n=========\n\n",buffer);
    buffer += header_size;
    fread(buffer, file_size,1,fp);
    printf("\n====BUFFER===%s\n=========\n\n",buffer_init);
    html_file.data = buffer_init;
    html_file.length = file_size;
    return html_file;
}

void* handel_req(void* arg){
    char *header = "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                     ;
    int header_len = strlen(header);
    char buffer[BUFFER_SIZE];
    int new_socket = *(int *)arg;
    printf("Got New Connection At %d\n", new_socket);
    ssize_t bytesRead = read(new_socket, buffer, BUFFER_SIZE - 1);
    if (bytesRead >= 0) {
        buffer[bytesRead] = '\0';
        puts(buffer);
    } else {
        perror("Error reading buffer...\n");
    }
    
    HTML_FILE html_file = read_html("index.html",header, header_len);
    printf("Sending.. %s",header);
    write(new_socket, html_file.data,html_file.length);
    close(new_socket);
    return arg;
}


void listen_for_connection(int sockfd,struct sockaddr_in sock_addr){
    thread_obj_setup(0xff,NULL);
    while (1) {
        int addrlen = sizeof(sock_addr);
        int new_socket = accept(sockfd, (struct sockaddr*)&sock_addr, (socklen_t*)&addrlen);
        // int thread_index = THREAD_OBJECT.create(handel_req,&new_socket);
        // THREAD_OBJECT.join(thread_index);        
        handel_req(&new_socket);
    }
}



int main(){
    // int sockfd;
    // struct sockaddr_in server_addr;
    // sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // if (sockfd == -1) {
    //     perror("Error creating socket");
    // }

    // memset(&server_addr, 0, sizeof(server_addr));
    // server_addr.sin_family = AF_INET;
    // server_addr.sin_addr.s_addr = str_to_host(HOST);
    // server_addr.sin_port = htons(PORT);

    // if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
    //     -1) {
    //     perror("Error binding socket");
    // }
    // listen(sockfd,10);
    // listen_for_connection(sockfd,server_addr);
    HTML_FILE htm = read_html("index.html","hello",6);
    printf("%s",htm.data);
    return  0;
}

