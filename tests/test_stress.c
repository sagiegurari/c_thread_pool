#include "test.h"
#include "threadlock.h"
#include "threadpool.h"

static int               _global_invoked = 0;
static struct ThreadLock *_global_lock   = NULL;


static void _run(void *args)
{
  assert_true_with_description(args != NULL, "args are null");
  assert_string_equal("test", (char *)args);
  threadlock_lock(_global_lock);
  _global_invoked++;
  threadlock_unlock(_global_lock);
}


static void test_impl()
{
  _global_lock = threadlock_new();
  struct ThreadPool *pool = threadpool_new();

  assert_true_with_description(pool != NULL, "pool is null");

  int loops    = 100;
  int parallel = 100;
  for (int loop = 0; loop < loops; loop++)
  {
    for (int index = 0; index < parallel; index++)
    {
      assert_true_with_description(threadpool_invoke(pool, _run, "test"), "Failed to submit job to thread");
    }
    threadpool_block(pool);
  }
  assert_num_equal(_global_invoked, loops * parallel);

  threadpool_release(pool);
  threadlock_release(_global_lock);
}


int main()
{
  test_run(test_impl);
}

