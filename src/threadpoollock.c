#include "threadpoollock.h"
#include <pthread.h>
#include <stdlib.h>

struct ThreadPoolLock
{
  int             wait_timeout_in_milliseconds;
  pthread_mutex_t lock;
  pthread_cond_t  signal;
};

struct ThreadPoolLock *threadpoollock_new(int wait_timeout_in_milliseconds)
{
  struct ThreadPoolLock *lock = malloc(sizeof(struct ThreadPoolLock));

  lock->wait_timeout_in_milliseconds = wait_timeout_in_milliseconds;

  if (pthread_mutex_init(&lock->lock, NULL))
  {
    free(lock);
    return(NULL);
  }
  if (pthread_cond_init(&lock->signal, NULL))
  {
    pthread_mutex_destroy(&lock->lock);
    free(lock);
    return(NULL);
  }

  return(lock);
}


void threadpoollock_release(struct ThreadPoolLock *lock)
{
  if (lock == NULL)
  {
    return;
  }

  pthread_mutex_destroy(&lock->lock);
  pthread_cond_destroy(&lock->signal);
  free(lock);
}


bool threadpoollock_lock(struct ThreadPoolLock *lock)
{
  if (lock == NULL)
  {
    return(false);
  }

  pthread_mutex_lock(&lock->lock);
  return(true);
}


bool threadpoollock_unlock(struct ThreadPoolLock *lock)
{
  if (lock == NULL)
  {
    return(false);
  }

  pthread_mutex_unlock(&lock->lock);
  return(true);
}


bool threadpoollock_signal(struct ThreadPoolLock *lock, bool wrap_with_lock)
{
  if (lock == NULL)
  {
    return(false);
  }

  if (wrap_with_lock)
  {
    if (!threadpoollock_lock(lock))
    {
      return(false);
    }
  }
  pthread_cond_signal(&lock->signal);
  if (wrap_with_lock)
  {
    threadpoollock_unlock(lock);
  }

  return(true);
}


bool threadpoollock_wait(struct ThreadPoolLock *lock, bool wrap_with_lock)
{
  if (lock == NULL)
  {
    return(false);
  }

  return(threadpoollock_wait_with_timeout(lock, wrap_with_lock, lock->wait_timeout_in_milliseconds));
}


bool threadpoollock_wait_with_timeout(struct ThreadPoolLock *lock, bool wrap_with_lock, int wait_timeout_in_milliseconds)
{
  if (lock == NULL)
  {
    return(false);
  }

  if (wrap_with_lock)
  {
    if (!threadpoollock_lock(lock))
    {
      return(false);
    }
  }

  if (wait_timeout_in_milliseconds <= 0)
  {
    pthread_cond_wait(&lock->signal, &lock->lock);
  }
  else
  {
    struct timespec ts;
    ts.tv_sec  = time(NULL);
    ts.tv_nsec = wait_timeout_in_milliseconds * 1000000;

    pthread_cond_timedwait(&lock->signal, &lock->lock, &ts);
  }

  if (wrap_with_lock)
  {
    threadpoollock_unlock(lock);
  }

  return(true);
}

