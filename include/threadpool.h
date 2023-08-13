#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdbool.h>
#include <stddef.h>

/**
 * This is the top level header that exposes this library API.
 * Other headers should be considered internal and should not be included directly.
 */

#define THREADPOOL_DEFAULT_MAX_SIZE                     20
#define THREADPOOL_DEFAULT_WAIT_TIMEOUT_MILLISECONDS    5000 // 5 seconds

struct ThreadPool;

/**
 * Custom threading API used by the ThreadPool and provided
 * by the caller.
 * The API will only require to handle a single thread at a time.
 */
struct ThreadPoolThreadAPI
{
  /**
   * Creates and starts a new thread.
   * A basic implementation can be to simply wrap the pthread_create call and
   * return the thread.
   */
  void * (*start)(struct ThreadPoolThreadAPI *, void *(*fn)(void *), void *args);

  /**
   * Stops the currently running thread (if exists).
   * A basic implementation can be to simply wrap the pthread_join call.
   */
  bool (*stop)(struct ThreadPoolThreadAPI *, void *);

  /**
   * Any custom context used by the caller (not used by ThreadPool).
   */
  void *context;
};

struct ThreadPoolOptions
{
  size_t                     max_size;
  bool                       wait_for_available_thread;
  struct ThreadPoolThreadAPI thread_api;
  // internal wait/signal timeout, <= 0 for unlimited
  int                        wait_timeout_in_milliseconds;
};

/**
 * Returns default options struct that can be modified and used to later
 * create a new thread pool.
 */
struct ThreadPoolOptions threadpool_new_default_options(void);

/**
 * Creates a new thread pool with default options.
 */
struct ThreadPool *threadpool_new(void);

/**
 * Creates a new thread pool with provided options.
 * The optional thread api can be provided to gain more control over thread creation.
 */
struct ThreadPool *threadpool_new_with_options(struct ThreadPoolOptions, struct ThreadPoolThreadAPI *);

/**
 * Finishes the currently running work and releases all the
 * thread pool. All functions in the backlog are dropped and the
 * thread pool is no longer usable.
 */
void threadpool_release(struct ThreadPool *);

/**
 * Invokes the function via thread from the pool.
 * If no thread is available, a new thread will be created or (if max reached), the
 * function will wait for a free thread (in case wait_for_available_thread is true).
 */
bool threadpool_invoke(struct ThreadPool *, void (*fn)(void *), void *);

/**
 * Blocks until all threads finish their work (including any waiting work).
 */
void threadpool_block(struct ThreadPool *);

#endif

