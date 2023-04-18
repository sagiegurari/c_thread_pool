#include "threadpoolthreadapiimpl.h"
#include <pthread.h>
#include <stdlib.h>

static void *_threadpoolthreadapiimpl_start(struct ThreadPoolThreadAPI *, void *(*fn)(void *), void *);
static bool _threadpoolthreadapiimpl_stop(struct ThreadPoolThreadAPI *, void *);

struct ThreadPoolAPIThread
{
  pthread_t thread;
};

struct ThreadPoolThreadAPI *threadpoolthreadapiimpl_new()
{
  struct ThreadPoolThreadAPI *thread_api = malloc(sizeof(struct ThreadPoolThreadAPI));

  thread_api->start   = _threadpoolthreadapiimpl_start;
  thread_api->stop    = _threadpoolthreadapiimpl_stop;
  thread_api->context = NULL;

  return(thread_api);
}


void threadpoolthreadapiimpl_release(struct ThreadPoolThreadAPI *thread_api)
{
  if (thread_api == NULL)
  {
    return;
  }

  free(thread_api->context);
  free(thread_api);
}


static void *_threadpoolthreadapiimpl_start(struct ThreadPoolThreadAPI *thread_api, void *(*fn)(void *), void *args)
{
  if (thread_api == NULL)
  {
    return(NULL);
  }

  struct ThreadPoolAPIThread *thread = malloc(sizeof(struct ThreadPoolAPIThread));

  bool                       created = !pthread_create(&thread->thread, NULL, fn, args);
  if (!created)
  {
    free(thread);
    return(NULL);
  }

  return(thread);
}


static bool _threadpoolthreadapiimpl_stop(struct ThreadPoolThreadAPI *thread_api, void *thread_ptr)
{
  if (thread_api == NULL || thread_ptr == NULL)
  {
    return(false);
  }

  struct ThreadPoolAPIThread *thread = (struct ThreadPoolAPIThread *)thread_ptr;

  pthread_join(thread->thread, NULL);

  free(thread);

  return(true);
}

