#include <assert.h>
#include <stdint.h>
#include <string.h>


#define and &&
#define or ||

uint8_t hex1_to_int(char hex_char){
    if(hex_char>='0' and hex_char <= '9'){
        return hex_char-'0';
    }else if(hex_char>='A' and hex_char<='F'){
        return  hex_char-'A' + 10;
    }else if(hex_char>='a' and hex_char<='f'){
        return  hex_char-'a' + 10;
    }
    return  -1;
}

uint32_t str_to_host(const char* str_host){
    uint32_t to_int = 0;
    if(strcmp(str_host,"localhost")==0){
        // return 127.0.0.0
        to_int = 127<<3*8;
        return  to_int;
    }
    int shifts=2;
    int tmp=0;
    while(str_host[0]){
    if(str_host[0]=='.'){
        str_host++;
        // when their is single digit between chars
        // just fill with zeros
        tmp = to_int&0xf;
        to_int &= 0xfffffff0;
        to_int<<=4*shifts;
        to_int |= tmp&0xf;
        shifts=2;
    }
    //  mask the last four bits from hex_to_n
    // and or with all 0's four bit from shift
    to_int<<=4;
    shifts--;
    to_int |= hex1_to_int(str_host[0])&0xf;
    str_host++;
    }
    return to_int;
}
