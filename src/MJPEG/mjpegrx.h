/*=============================================================================
 *File Name: mjpegrx.h
 *Description: MJPEG HTTP stream decoder
 *Author: FRC Team 3512, Spartatroniks
 *=============================================================================*/

#ifndef _MJPEGRX_H
#define _MJPEGRX_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#endif

struct keyvalue_t {
    char *key;
    char *value;
    struct keyvalue_t *next;
};

struct mjpeg_threadargs_t {
    char *host;
    char *reqpath;
    int port;

    struct mjpeg_callbacks_t *callbacks;
    struct mjpeg_inst_t *inst;
};

struct mjpeg_callbacks_t {
    void (*donecallback)(void *optarg);
    void (*readcallback)(char *buf, int bufsize, void *optarg);
    void *optarg;
};

#ifdef WIN32
struct mjpeg_inst_t {
    volatile int threadrunning;
    HANDLE thread;
    unsigned int threadId;
    int sd;
};
#else
struct mjpeg_inst_t {
    volatile int threadrunning;
    pthread_t thread;
    int sd;
};
#endif

struct mjpeg_inst_t *
mjpeg_launchthread(
        char *host,
        int port,
        char *reqpath,
        struct mjpeg_callbacks_t *callbacks
        );

void mjpeg_stopthread(struct mjpeg_inst_t *inst);

/* Win32 requires different function pointer for _beginthreadex */
#ifdef WIN32
unsigned int __stdcall mjpeg_threadmain(void *optarg);
#else
void * mjpeg_threadmain(void *optarg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MJPEGRX_H */
