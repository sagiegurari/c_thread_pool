#include "test.h"
#include "threadpool.h"
#include "threadpoollock.h"


static void test_impl()
{
  struct ThreadPoolOptions options = threadpool_new_default_options();

  options.max_size = 0;
  struct ThreadPool *pool = threadpool_new_with_options(options, NULL);
  assert_true_with_description(pool == NULL, "pool created with invalid options");
}


int main()
{
  test_run(test_impl);
}

