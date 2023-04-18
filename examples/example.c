#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>

static void do_in_background(void *);


int main()
{
  printf("Library Examples:\n");

  struct ThreadPool *pool = threadpool_new();

  int               loops    = 100;
  int               parallel = 100;
  for (int loop = 0; loop < loops; loop++)
  {
    for (int index = 0; index < parallel; index++)
    {
      char *args = (char *)malloc(sizeof(char) * 100);
      sprintf(args, "loop: %d sub loop: %d", loop, index);

      // invoke function in the background
      threadpool_invoke(pool, do_in_background, args);
    }

    // block until all threads finished processing all requests
    threadpool_block(pool);
  }

  // Once done, release the pool.
  // This will block until all threads are done.
  threadpool_release(pool);

  return(0);
}


static void do_in_background(void *args)
{
  printf("In thread, args: %s\n", (char *)args);
}

