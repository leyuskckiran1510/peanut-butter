#ifndef __PB_LOGGER__
#define __PB_LOGGER__

#ifndef LOG_LEVEL
    #define LOG_LEVEL 0
#endif

#define log(type,fmt,... ) printf( "[%5s] %s:%6d | " fmt"\n",type,__FILE__,__LINE__, ##__VA_ARGS__ );
#define log_info(fmt,...)  ;
#define log_debug(fmt,...) ;
#define log_warn(fmt,...)  ;
#define log_error(fmt,...) ;

#if LOG_LEVEL >= 0
    #undef log_error
    #define log_error(fmt,...)  log("ERROR",fmt,##__VA_ARGS__);
#endif
#if LOG_LEVEL >= 1
    #undef log_warn
    #define log_warn(fmt,...)  log("WARN",fmt,##__VA_ARGS__);
#endif
#if LOG_LEVEL >= 2
    #undef log_debug
    #define log_debug(fmt,...)  log("DEBUG",fmt,##__VA_ARGS__);
#endif
#if LOG_LEVEL >= 3
    #undef log_info
    #define log_info(fmt,...)  log("INFO",fmt,##__VA_ARGS__);

#endif


#endif