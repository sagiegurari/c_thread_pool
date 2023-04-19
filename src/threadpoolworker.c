#include "threadlock.h"
#include "threadpoolworker.h"
#include <stdlib.h>

struct ThreadPoolWorker
{
  void                       *thread;
  void                       (*fn)(void *);
  void                       *args;
  struct ThreadLock          *worker_lock;
  struct ThreadLock          *notify_lock;
  struct ThreadPoolThreadAPI *thread_api;
  bool                       *should_stop;
};

static void *_threadpoolworker_run(void *);

struct ThreadPoolWorker *threadpoolworker_new(struct ThreadPoolThreadAPI *thread_api, struct ThreadLock *notify_lock, bool *should_stop, int wait_timeout_in_milliseconds)
{
  if (thread_api == NULL || notify_lock == NULL || *should_stop)
  {
    return(NULL);
  }

  struct ThreadLockOptions lock_options = threadlock_new_default_options();
  lock_options.wait_timeout_in_milliseconds = wait_timeout_in_milliseconds;
  struct ThreadLock        *worker_lock = threadlock_new_with_options(lock_options);
  if (worker_lock == NULL)
  {
    return(NULL);
  }

  struct ThreadPoolWorker *worker = malloc(sizeof(struct ThreadPoolWorker));
  worker->worker_lock = worker_lock;
  worker->notify_lock = notify_lock;
  worker->thread_api  = thread_api;
  worker->should_stop = should_stop;
  worker->fn          = NULL;
  worker->args        = NULL;

  worker->thread = thread_api->start(thread_api, _threadpoolworker_run, worker);
  if (worker->thread == NULL)
  {
    threadlock_release(worker->worker_lock);
    free(worker);
    return(NULL);
  }

  return(worker);
}


void threadpoolworker_release(struct ThreadPoolWorker *worker)
{
  if (worker == NULL)
  {
    return;
  }

  threadlock_lock(worker->worker_lock);
  worker->fn   = NULL;
  worker->args = NULL;
  threadlock_signal(worker->worker_lock, false);
  threadlock_unlock(worker->worker_lock);
  worker->thread_api->stop(worker->thread_api, worker->thread);

  threadlock_release(worker->worker_lock);
  free(worker);
}


bool threadpoolworker_is_running(struct ThreadPoolWorker *worker)
{
  if (worker == NULL)
  {
    return(false);
  }

  return(worker->fn != NULL);
}


bool threadpoolworker_assign(struct ThreadPoolWorker *worker, void (*fn)(void *), void *args)
{
  if (worker == NULL || fn == NULL || threadpoolworker_is_running(worker))
  {
    return(false);
  }

  threadlock_lock(worker->worker_lock);
  worker->args = args;
  worker->fn   = fn;
  threadlock_signal(worker->worker_lock, false);
  threadlock_unlock(worker->worker_lock);

  return(true);
}


static void *_threadpoolworker_run(void *worker_ptr)
{
  if (worker_ptr == NULL)
  {
    return(NULL);
  }

  struct ThreadPoolWorker *worker = (struct ThreadPoolWorker *)worker_ptr;
  do
  {
    threadlock_lock(worker->worker_lock);
    void (*fn)(void *) = worker->fn;
    void *args = worker->args;
    if (fn != NULL)
    {
      fn(args);

      worker->args = NULL;
      worker->fn   = NULL;

      threadlock_unlock(worker->worker_lock);
      threadlock_signal(worker->notify_lock, true);
    }
    else if (!*worker->should_stop)
    {
      threadlock_signal(worker->notify_lock, true);
      threadlock_wait(worker->worker_lock, false);
      threadlock_unlock(worker->worker_lock);
    }

    if (*worker->should_stop && worker->fn == NULL)
    {
      break;
    }
  } while (true);

  return(NULL);
} /* _threadpool_worker_loop */
