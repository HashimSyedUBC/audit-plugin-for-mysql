#ifdef __cplusplus
extern "C" {
#endif

#include "thr_timer.h"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>


static void* timer_thread(void* arg) {
    thr_timer_t* timer = (thr_timer_t*)arg;
    struct timespec ts;

    while (timer->running) {
        pthread_mutex_lock(&timer->mutex);
        
        while (timer->expired && timer->running) {
            pthread_cond_wait(&timer->cond, &timer->mutex);
        }

        if (!timer->running) {
            pthread_mutex_unlock(&timer->mutex);
            break;
        }

        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timer->delay_us / 1000000;
        ts.tv_nsec += (timer->delay_us % 1000000) * 1000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000;
        }

        int rc = pthread_cond_timedwait(&timer->cond, &timer->mutex, &ts);
        
        if (rc == ETIMEDOUT && timer->running && !timer->expired) {
            timer->expired = true;
            pthread_mutex_unlock(&timer->mutex);
            timer->callback();
        } else {
            pthread_mutex_unlock(&timer->mutex);
        }
    }

    return NULL;
}

int thr_timer_init(thr_timer_t* timer, thr_timer_callback callback) {
    timer->callback = callback;
    timer->running = true;
    timer->expired = false;
    timer->delay_us = 0;
    pthread_mutex_init(&timer->mutex, NULL);
    pthread_cond_init(&timer->cond, NULL);

    return pthread_create(&timer->thread, NULL, timer_thread, timer);
}

int thr_timer_settime(thr_timer_t* timer, unsigned long microseconds) {
    pthread_mutex_lock(&timer->mutex);
    timer->delay_us = microseconds;
    timer->expired = false;
    pthread_mutex_unlock(&timer->mutex);
    pthread_cond_signal(&timer->cond);
    return 0;
}

int thr_timer_end(thr_timer_t* timer) {
    pthread_mutex_lock(&timer->mutex);
    timer->running = false;
    pthread_mutex_unlock(&timer->mutex);
    pthread_cond_signal(&timer->cond);
    pthread_join(timer->thread, NULL);
    pthread_mutex_destroy(&timer->mutex);
    pthread_cond_destroy(&timer->cond);
    return 0;
}

bool thr_timer_is_expired(thr_timer_t* timer) {
    bool expired;
    pthread_mutex_lock(&timer->mutex);
    expired = timer->expired;
    pthread_mutex_unlock(&timer->mutex);
    return expired;
}

#ifdef __cplusplus
}
#endif