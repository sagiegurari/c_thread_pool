#include "threadpoollock.h"
#include "threadpoolworker.h"
#include <stdlib.h>

struct ThreadPoolWorker
{
  void                       *thread;
  void                       (*fn)(void *);
  void                       *args;
  struct ThreadPoolLock      *worker_lock;
  struct ThreadPoolLock      *notify_lock;
  struct ThreadPoolThreadAPI *thread_api;
  bool                       *should_stop;
};

static void *_threadpoolworker_run(void *);

struct ThreadPoolWorker *threadpoolworker_new(struct ThreadPoolThreadAPI *thread_api, struct ThreadPoolLock *notify_lock, bool *should_stop, int wait_timeout_in_seconds)
{
  if (thread_api == NULL || notify_lock == NULL || *should_stop)
  {
    return(NULL);
  }

  struct ThreadPoolLock *worker_lock = threadpoollock_new(wait_timeout_in_seconds);
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
    threadpoollock_release(worker->worker_lock);
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

  threadpoollock_lock(worker->worker_lock);
  worker->fn   = NULL;
  worker->args = NULL;
  threadpoollock_signal(worker->worker_lock, false);
  threadpoollock_unlock(worker->worker_lock);
  worker->thread_api->stop(worker->thread_api, worker->thread);

  threadpoollock_release(worker->worker_lock);
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

  threadpoollock_lock(worker->worker_lock);
  worker->args = args;
  worker->fn   = fn;
  threadpoollock_signal(worker->worker_lock, false);
  threadpoollock_unlock(worker->worker_lock);

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
    threadpoollock_lock(worker->worker_lock);
    void (*fn)(void *) = worker->fn;
    void *args = worker->args;
    if (fn != NULL)
    {
      fn(args);

      worker->args = NULL;
      worker->fn   = NULL;

      threadpoollock_unlock(worker->worker_lock);
      threadpoollock_signal(worker->notify_lock, true);
    }
    else if (!*worker->should_stop)
    {
      threadpoollock_signal(worker->notify_lock, true);
      threadpoollock_wait(worker->worker_lock, false);
      threadpoollock_unlock(worker->worker_lock);
    }

    if (*worker->should_stop && worker->fn == NULL)
    {
      break;
    }
  } while (true);

  return(NULL);
} /* _threadpool_worker_loop */
