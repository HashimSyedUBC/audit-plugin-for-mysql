#ifndef THR_TIMER_H
#define THR_TIMER_H

#include <pthread.h>
#include <stdbool.h>
#include <my_thread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*thr_timer_callback)(void);

typedef struct {
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    thr_timer_callback callback;
    unsigned long delay_us;
    bool running;
    bool expired;
} thr_timer_t;

int thr_timer_init(thr_timer_t* timer, thr_timer_callback callback);
int thr_timer_settime(thr_timer_t* timer, unsigned long microseconds);
int thr_timer_end(thr_timer_t* timer);
bool thr_timer_is_expired(thr_timer_t* timer);

#ifdef __cplusplus
}
#endif

#endif // THR_TIMER_H