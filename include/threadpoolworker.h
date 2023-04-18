#ifndef THREADPOOLWORKER_H
#define THREADPOOLWORKER_H

#include "threadpool.h"
#include "threadpoollock.h"
#include <stdbool.h>
#include <stddef.h>

struct ThreadPoolWorker;

struct ThreadPoolWorker *threadpoolworker_new(struct ThreadPoolThreadAPI *, struct ThreadPoolLock *, bool *should_stop, int wait_timeout_in_seconds);
void threadpoolworker_release(struct ThreadPoolWorker *);
bool threadpoolworker_is_running(struct ThreadPoolWorker *);
bool threadpoolworker_assign(struct ThreadPoolWorker *, void (*fn)(void *), void *args);

#endif

