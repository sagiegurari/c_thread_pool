#ifndef THREADPOOLLOCK_H
#define THREADPOOLLOCK_H

#include <stdbool.h>
#include <stddef.h>

struct ThreadPoolLock;

struct ThreadPoolLock *threadpoollock_new(int wait_timeout_in_milliseconds);
void threadpoollock_release(struct ThreadPoolLock *);
bool threadpoollock_lock(struct ThreadPoolLock *);
bool threadpoollock_unlock(struct ThreadPoolLock *);
bool threadpoollock_signal(struct ThreadPoolLock *, bool wrap_with_lock);
bool threadpoollock_wait(struct ThreadPoolLock *, bool wrap_with_lock);
bool threadpoollock_wait_with_timeout(struct ThreadPoolLock *, bool wrap_with_lock, int wait_timeout_in_milliseconds);

#endif

