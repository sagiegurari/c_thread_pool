#include "threadlock.h"
#include "threadpool.h"
#include "threadpoolthreadapiimpl.h"
#include "threadpoolworker.h"
#include <stdlib.h>

struct ThreadPool
{
  bool                        in_release;
  bool                        blocked;
  size_t                      max_size;
  bool                        wait_for_available_thread;
  int                         wait_timeout_in_milliseconds;
  struct  ThreadPoolThreadAPI *thread_api;
  bool                        release_thread_api;
  struct ThreadPoolWorker     **workers;
  struct ThreadLock           *state_lock;
  struct ThreadLock           *notification_lock;
};

static struct ThreadPoolWorker *_threadpool_start_worker(struct ThreadPool *);
struct ThreadPoolWorker        *_threadpool_get_available_worker(struct ThreadPool *, void (*fn)(void *), void *);

struct ThreadPoolOptions       threadpool_new_default_options(void)
{
  struct ThreadPoolOptions options =
  {
    .max_size                     = THREADPOOL_DEFAULT_MAX_SIZE,
    .wait_for_available_thread    = true,
    .wait_timeout_in_milliseconds = THREADPOOL_DEFAULT_WAIT_TIMEOUT_MILLISECONDS,
  };

  return(options);
}

struct ThreadPool *threadpool_new(void)
{
  struct ThreadPoolOptions options = threadpool_new_default_options();

  return(threadpool_new_with_options(options, NULL));
}

struct ThreadPool *threadpool_new_with_options(struct ThreadPoolOptions options, struct ThreadPoolThreadAPI *thread_api)
{
  if (!options.max_size)
  {
    return(NULL);
  }

  struct ThreadLockOptions lock_options = threadlock_new_default_options();
  lock_options.wait_timeout_in_milliseconds = options.wait_timeout_in_milliseconds;
  struct ThreadLock        *state_lock = threadlock_new_with_options(lock_options);
  if (state_lock == NULL)
  {
    return(NULL);
  }
  struct ThreadLock *notification_lock = threadlock_new_with_options(lock_options);
  if (notification_lock == NULL)
  {
    threadlock_release(state_lock);
    return(NULL);
  }

  struct ThreadPool *pool = malloc(sizeof(struct ThreadPool));
  pool->state_lock                   = state_lock;
  pool->notification_lock            = notification_lock;
  pool->in_release                   = false;
  pool->blocked                      = false;
  pool->max_size                     = options.max_size;
  pool->wait_for_available_thread    = options.wait_for_available_thread;
  pool->wait_timeout_in_milliseconds = options.wait_timeout_in_milliseconds;
  pool->thread_api                   = thread_api;
  pool->release_thread_api           = thread_api == NULL;
  pool->workers                      = malloc(sizeof(struct ThreadPoolWorker *) * pool->max_size);
  for (size_t index = 0; index < pool->max_size; index++)
  {
    pool->workers[index] = NULL;
  }

  if (pool->thread_api == NULL)
  {
    pool->thread_api = threadpoolthreadapiimpl_new();
  }

  return(pool);
} /* threadpool_new_with_options */


void threadpool_release(struct ThreadPool *pool)
{
  if (pool == NULL || pool->in_release)
  {
    return;
  }

  pool->in_release = true;
  threadpool_block(pool);

  for (size_t index = 0; index < pool->max_size; index++)
  {
    struct ThreadPoolWorker *worker = pool->workers[index];
    if (worker == NULL)
    {
      break;
    }

    threadpoolworker_release(worker);
  }

  threadlock_release(pool->state_lock);
  threadlock_release(pool->notification_lock);
  if (pool->release_thread_api)
  {
    threadpoolthreadapiimpl_release(pool->thread_api);
  }
  free(pool->workers);
  free(pool);
}


bool threadpool_invoke(struct ThreadPool *pool, void (*fn)(void *), void *args)
{
  if (pool == NULL || fn == NULL || pool->in_release || pool->blocked)
  {
    return(false);
  }

  struct ThreadPoolWorker *worker = _threadpool_get_available_worker(pool, fn, args);
  while (worker == NULL && pool->wait_for_available_thread && !pool->in_release && !pool->blocked)
  {
    // wait for worker to be available
    threadlock_wait_with_timeout(pool->notification_lock, true, 1000);

    worker = _threadpool_get_available_worker(pool, fn, args);
  }

  return(worker != NULL);
}


void threadpool_block(struct ThreadPool *pool)
{
  if (pool == NULL)
  {
    return;
  }

  threadlock_lock(pool->state_lock);
  pool->blocked = true;

  for (size_t index = 0; index < pool->max_size; index++)
  {
    struct ThreadPoolWorker *worker = pool->workers[index];
    if (worker == NULL)
    {
      break;
    }

    while (threadpoolworker_is_running(worker))
    {
      // wait for worker to notify its done
      threadlock_wait(pool->notification_lock, true);
    }
  }

  pool->blocked = false;
  threadlock_unlock(pool->state_lock);
}

static struct ThreadPoolWorker *_threadpool_start_worker(struct ThreadPool *pool)
{
  if (pool == NULL)
  {
    return(NULL);
  }

  struct ThreadPoolWorker *worker = threadpoolworker_new(pool->thread_api, pool->notification_lock, &pool->in_release, pool->wait_timeout_in_milliseconds);
  if (worker == NULL)
  {
    return(NULL);
  }

  return(worker);
}

struct ThreadPoolWorker *_threadpool_get_available_worker(struct ThreadPool *pool, void (*fn)(void *), void *args)
{
  struct ThreadPoolWorker *worker = NULL;
  bool                    done    = false;

  threadlock_lock(pool->state_lock);
  for (size_t index = 0; index < pool->max_size; index++)
  {
    worker = pool->workers[index];
    if (worker == NULL)
    {
      worker = _threadpool_start_worker(pool);
      if (worker != NULL)
      {
        pool->workers[index] = worker;
        threadpoolworker_assign(worker, fn, args);
        done = true;
      }
      break;
    }
    else if (threadpoolworker_assign(worker, fn, args))
    {
      done = true;
      break;
    }
  }

  threadlock_unlock(pool->state_lock);

  if (!done)
  {
    return(NULL);
  }

  return(worker);
}

