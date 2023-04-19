#ifndef THREADPOOLWORKER_H
#define THREADPOOLWORKER_H

#include "threadlock.h"
#include "threadpool.h"
#include <stdbool.h>
#include <stddef.h>

struct ThreadPoolWorker;

struct ThreadPoolWorker *threadpoolworker_new(struct ThreadPoolThreadAPI *, struct ThreadLock *, bool *should_stop, int wait_timeout_in_milliseconds);
void threadpoolworker_release(struct ThreadPoolWorker *);
bool threadpoolworker_is_running(struct ThreadPoolWorker *);
bool threadpoolworker_assign(struct ThreadPoolWorker *, void (*fn)(void *), void *args);

#endif

