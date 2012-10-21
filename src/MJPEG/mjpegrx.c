/* mjpeg HTTP stream decoder */

#ifdef WIN32

#include <winsock2.h>
#include <signal.h>
#define socklen_t int
#define MSG_WAITALL 0x8 // do not complete until packet is completely filled

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#else

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include <pthread.h>
#include <assert.h>

#endif

#include "mjpegrx.h"

#ifdef WIN32
void
_sck_wsainit(){
    WORD vs;
    WSADATA wsadata;

    vs = MAKEWORD(2, 2);
    WSAStartup(vs, &wsadata);

    return;
}
#endif

int
mjpeg_rxbyte(char **buf, int *bufpos, int *bufsize, int sd){
    int bytesread;

    bytesread = recv(sd, (*buf)+*bufpos, 1, MSG_WAITALL);
    assert(bytesread == 1);
    if(bytesread < 1) return 1;
    (*bufpos)++;

    if(*bufpos == 1023){
        *bufsize += 1024;
        *buf = realloc(*buf, *bufsize);
    }

    return 0;
}

int
mjpeg_rxheaders(char **buf_out, int *bufsize_out, int sd){
    int allocsize;
    int bufpos;
    char *buf;

    bufpos = 0;
    allocsize = 1024;
    buf = malloc(allocsize);

    while(1){
        if(mjpeg_rxbyte(&buf, &bufpos, &allocsize, sd) != 0) return -1;
        if(bufpos >= 4 && buf[bufpos-4] == '\r' && buf[bufpos-3] == '\n' && buf[bufpos-2] == '\r' && buf[bufpos-1] == '\n'){
            break;
        }
    }
    buf[bufpos] = '\0';

    *buf_out = buf;
    *bufsize_out = bufpos;

    return 0;
}

int
mjpeg_sck_connect(char *host, int port)
{
    int sd;
    int error;
    struct hostent *hp;
    struct sockaddr_in pin;

    #ifdef WIN32
    _sck_wsainit();
    #endif

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0) return -1;

    /* get the host */
    hp = gethostbyname(host);
    if(hp == NULL) return -1;

    memset(&pin, 0, sizeof(struct sockaddr_in));
    pin.sin_family = AF_INET;
    pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
    pin.sin_port = htons(port);

    /* connect */
    error = connect(
        sd,
        (struct sockaddr *)&pin,
        sizeof(struct sockaddr_in));
    if(error != 0) return -1;

    /* everything worked, return the socket descriptor */
    return sd;
}

char *
strchrs(char *s, char *c)
{
    char *t;
    char *ct;

    for(t = s; *t != '\0'; t++){
        for(ct = c; *ct != '\0'; ct++){
            if(*t == *ct) return t;
        }
    }

    return NULL;
}

char *
strtok_r_n(char *str, char *sep, char **last, char *used)
{
    char *strsep;
    char *ret;

    if(str != NULL) *last = str;

    strsep = strchrs(*last, sep);
    if(strsep == NULL) return NULL;
    if(used != NULL) *used = *strsep;
    *strsep = '\0';

    ret = *last;
    *last = strsep+1;

    return ret;
}

/* Processes the HTTP response headers, seperating them into key-value
   pairs. These are then stored in a linked list. The "header" argument
   should point to a block of HTTP response headers in the standard ':'
   and '\n' seperated format. The key is the text on a line before the
   ':', the value is the text after the ':', but before the '\n'. Any
   line without a ':' is ignored. a pointer to the first element in a
   linked list is returned. */
struct keyvalue_t *
mjpeg_process_header(char *header)
{
    char *strtoksave;
    struct keyvalue_t *list = NULL;
    struct keyvalue_t *start = NULL;
    char *key;
    char *value;
    char used;

    header = strdup(header);
    if(header == NULL) return NULL;

    key = strtok_r_n(header, ":\n", &strtoksave, &used);
    if(key == NULL){
        return NULL;
    }

    while(1){ /* we break out inside */
        /* if no ':' exists on the line, ignore it */
        if(used == '\n'){
            key = strtok_r_n(
                NULL,
                ":\n",
                &strtoksave,
                &used);
            if(key == NULL) break;
            continue;
        }

        /* create a linked list element */
        if(list == NULL){
            list = malloc(sizeof(struct keyvalue_t));
            start = list;
        }else{
            list->next = malloc(sizeof(struct keyvalue_t));
            list = list->next;
        }
        list->next = NULL;

        /* save off the key */
        list->key = strdup(key);

        /* get the value */
        value = strtok_r_n(NULL, "\n", &strtoksave, NULL);
        if(value == NULL) break;
        value++;
        if(value[strlen(value)-1] == '\r'){
            value[strlen(value)-1] = '\0';
        }
        list->value = strdup(value);

        /* get the key for next loop */
        key = strtok_r_n(NULL, ":\n", &strtoksave, &used);
        if(key == NULL) break;
    }

    free(header);

    return start;
}

void
mjpeg_freelist(struct keyvalue_t *list){
    struct keyvalue_t *tmp = NULL;
    struct keyvalue_t *c;

    for(c = list; c != NULL; c = c->next){
        if(tmp != NULL) free(tmp);
        tmp = c;
        free(c->key);
        free(c->value);
    }
    if(tmp != NULL) free(tmp);

    return;
}

char *
mjpeg_getvalue(struct keyvalue_t *list, char *key)
{
    struct keyvalue_t *c;
    for(c = list; c != NULL; c = c->next){
        if(strcmp(c->key, key) == 0) return c->value;
    }

    return NULL;
}

struct mjpeg_inst_t *
mjpeg_launchthread(
        char *host,
        int port,
        struct mjpeg_callbacks_t *callbacks
        )
{
    struct mjpeg_threadargs_t *args;
    struct mjpeg_inst_t *inst;

    inst = malloc(sizeof(struct mjpeg_inst_t));

    args = malloc(sizeof(struct mjpeg_threadargs_t));
    args->host = strdup(host);
    args->port = port;
    args->inst = inst;

    args->callbacks = malloc(sizeof(struct mjpeg_callbacks_t));
    memcpy(
        args->callbacks,
        callbacks,
        sizeof(struct mjpeg_callbacks_t)
    );

    inst->threadrunning = 1;

    pthread_create(&args->thread, NULL, mjpeg_threadmain, args);
    inst->thread = &args->thread;

    return inst;
}

void
mjpeg_stopthread(struct mjpeg_inst_t *inst)
{
    inst->threadrunning = 0;
    pthread_join(*inst->thread, NULL);

    free(inst);

#ifdef WIN32
    WSACleanup();
#endif

    return;
}

void *
mjpeg_threadmain(void *optarg)
{
    struct mjpeg_threadargs_t* args = optarg;

    int sd;

    char *asciisize;
    int datasize;
    char *buf; 

    char *headerbuf;
    int headerbufsize;
    struct keyvalue_t *headerlist;

    int bytesread;

    /* connect */
    sd = mjpeg_sck_connect(args->host, args->port);
    if(sd < 0){

        /* call the thread finished callback */
        if(args->callbacks->donecallback != NULL){
            args->callbacks->donecallback(
                args->callbacks->optarg);
        }

        free(args->host);
        free(args->callbacks);

        pthread_exit(NULL);
        return NULL;
    }

    /* sends some bogus data so that we'll get a response */
    send(sd, "U\r\n", 3, 0);

    while(args->inst->threadrunning > 0){
        /* Read and parse incoming HTTP response headers */
        mjpeg_rxheaders(&headerbuf, &headerbufsize, sd);
#ifdef DEBUG
        printf("%s\n", headerbuf);
#endif
        headerlist = mjpeg_process_header(headerbuf);
	free(headerbuf);
        if(headerlist == NULL) break;

        /* Read the Content-Length header to determine the
           length of data to read */
        asciisize = mjpeg_getvalue(
            headerlist,
            "Content-Length");
        if(asciisize == NULL) break;

        datasize = atoi(asciisize);

        /* read the data */
        buf = malloc(datasize);
        bytesread = recv(sd, buf, datasize, MSG_WAITALL);
	assert(bytesread == datasize);

        if(args->callbacks->readcallback != NULL){
            args->callbacks->readcallback(
                buf,
                datasize,
                args->callbacks->optarg);
        }
        free(buf);

        mjpeg_freelist(headerlist);
    }
    #ifdef WIN32
    closesocket(sd);
    #else
    close(sd);
    #endif

    if(args->callbacks->donecallback != NULL){
        args->callbacks->donecallback(args->callbacks->optarg);
    }

    free(args->host);
    free(args->callbacks);
    free(args);

    pthread_exit(NULL);
    return NULL;
}

