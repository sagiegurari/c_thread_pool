#include "test.h"
#include "threadpool.h"
#include "threadpoollock.h"

static int                   _global_invoked = 0;
static struct ThreadPoolLock *_global_lock   = NULL;


static void _run(void *args)
{
  assert_true_with_description(args != NULL, "args are null");
  assert_string_equal("test", (char *)args);
  threadpoollock_lock(_global_lock);
  _global_invoked++;
  threadpoollock_unlock(_global_lock);
}


static void test_impl()
{
  _global_lock = threadpoollock_new(0);
  struct ThreadPoolOptions options = threadpool_new_default_options();
  options.max_size = 2;
  struct ThreadPool        *pool = threadpool_new_with_options(options, NULL);

  assert_true_with_description(pool != NULL, "pool is null");

  int parallel = 10;
  for (int index = 0; index < parallel; index++)
  {
    assert_true_with_description(threadpool_invoke(pool, _run, "test"), "Failed to submit job to thread");
  }
  threadpool_block(pool);
  assert_num_equal(_global_invoked, parallel);

  threadpool_release(pool);
  threadpoollock_release(_global_lock);
}


int main()
{
  test_run(test_impl);
}

