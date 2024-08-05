#ifndef __PB_MIME_MATCH__
    #define __PB_MIME_MATCH__

#include <string.h>


#define CASE(M,X,Y)       if(!strcmp((M),(X)))return (Y);

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
#define MIME_TYPE_MATCH(file_ext) \
            CASE(file_ext,"aac","audio/aac");\
            CASE(file_ext,"abw","application/x-abiword");\
            CASE(file_ext,"apng","image/apng");\
            CASE(file_ext,"arc","application/x-freearc");\
            CASE(file_ext,"avif","image/avif");\
            CASE(file_ext,"avi","video/x-msvideo");\
            CASE(file_ext,"azw","application/vnd.amazon.ebook");\
            CASE(file_ext,"bin","application/octet-stream");\
            CASE(file_ext,"bmp","image/bmp");\
            CASE(file_ext,"bz","application/x-bzip");\
            CASE(file_ext,"bz2","application/x-bzip2");\
            CASE(file_ext,"cda","application/x-cdf");\
            CASE(file_ext,"csh","application/x-csh");\
            CASE(file_ext,"css","text/css");\
            CASE(file_ext,"csv","text/csv");\
            CASE(file_ext,"doc","application/msword");\
            CASE(file_ext,"docx","application/vnd.openxmlformats-officedocument.wordprocessingml.document");\
            CASE(file_ext,"eot","application/vnd.ms-fontobject");\
            CASE(file_ext,"epub","application/epub+zip");\
            CASE(file_ext,"gz","application/gzip");\
            CASE(file_ext,"gif","image/gif");\
            CASE(file_ext,"htm","text/html");\
            CASE(file_ext,"html","text/html");\
            CASE(file_ext,"ico","image/vnd.microsoft.icon");\
            CASE(file_ext,"ics","text/calendar");\
            CASE(file_ext,"jar","application/java-archive");\
            CASE(file_ext,"jpeg","image/jpeg");\
            CASE(file_ext,"jpg","image/jpeg");\
            CASE(file_ext,"js","text/javascript");\
            CASE(file_ext,"json","application/json");\
            CASE(file_ext,"jsonld","application/ld+json");\
            CASE(file_ext,"mid","audio/midi");\
            CASE(file_ext,"midi","audio/midi");\
            CASE(file_ext,"mjs","text/javascript");\
            CASE(file_ext,"mp3","audio/mpeg");\
            CASE(file_ext,"mp4","video/mp4");\
            CASE(file_ext,"mpeg","video/mpeg");\
            CASE(file_ext,"mpkg","application/vnd.apple.installer+xml");\
            CASE(file_ext,"odp","application/vnd.oasis.opendocument.presentation");\
            CASE(file_ext,"ods","application/vnd.oasis.opendocument.spreadsheet");\
            CASE(file_ext,"odt","application/vnd.oasis.opendocument.text");\
            CASE(file_ext,"oga","audio/ogg");\
            CASE(file_ext,"ogv","video/ogg");\
            CASE(file_ext,"ogx","application/ogg");\
            CASE(file_ext,"opus","audio/ogg");\
            CASE(file_ext,"otf","font/otf");\
            CASE(file_ext,"png","image/png");\
            CASE(file_ext,"jpg","image/jpeg");\
            CASE(file_ext,"pdf","application/pdf");\
            CASE(file_ext,"php","application/x-httpd-php");\
            CASE(file_ext,"ppt","application/vnd.ms-powerpoint");\
            CASE(file_ext,"pptx","application/vnd.openxmlformats-officedocument.presentationml.presentation");\
            CASE(file_ext,"rar","application/vnd.rar");\
            CASE(file_ext,"rtf","application/rtf");\
            CASE(file_ext,"sh","application/x-sh");\
            CASE(file_ext,"svg","image/svg+xml");\
            CASE(file_ext,"tar","application/x-tar");\
            CASE(file_ext,"tif","image/tiff");\
            CASE(file_ext,"tiff","image/tiff");\
            CASE(file_ext,"ts","video/mp2t");\
            CASE(file_ext,"ttf","font/ttf");\
            CASE(file_ext,"txt","text/plain");\
            CASE(file_ext,"vsd","application/vnd.visio");\
            CASE(file_ext,"wav","audio/wav");\
            CASE(file_ext,"weba","audio/webm");\
            CASE(file_ext,"webm","video/webm");\
            CASE(file_ext,"webp","image/webp");\
            CASE(file_ext,"woff","font/woff");\
            CASE(file_ext,"woff2","font/woff2");\
            CASE(file_ext,"xhtml","application/xhtml+xml");\
            CASE(file_ext,"xls","application/vnd.ms-excel");\
            CASE(file_ext,"xlsx","application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");\
            CASE(file_ext,"xml","application/xml");\
            CASE(file_ext,"xul","application/vnd.mozilla.xul+xml");\
            CASE(file_ext,"zip","application/zip");\
            CASE(file_ext,"3gp","video/3gpp");\
            CASE(file_ext,"3g2","video/3gpp2");\
            CASE(file_ext,"7z","application/x-7z-compressed");\
            return "text/plain";  \


#endif

