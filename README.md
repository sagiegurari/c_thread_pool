# Thread Pool

[![CI](https://github.com/sagiegurari/c_thread_pool/workflows/CI/badge.svg?branch=master)](https://github.com/sagiegurari/c_thread_pool/actions)
[![Release](https://img.shields.io/github/v/release/sagiegurari/c_thread_pool)](https://github.com/sagiegurari/c_thread_pool/releases)
[![license](https://img.shields.io/github/license/sagiegurari/c_thread_pool)](https://github.com/sagiegurari/c_thread_pool/blob/master/LICENSE)

> Thread Pool for C.

* [Overview](#overview)
* [Usage](#usage)
* [Contributing](.github/CONTRIBUTING.md)
* [Release History](CHANGELOG.md)
* [License](#license)

<a name="overview"></a>
## Overview
This library provides a simple thread pool and enables running functions in the background.

<a name="usage"></a>
## Usage

<!-- example source start -->
```c
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
```
<!-- example source end -->

## Contributing
See [contributing guide](.github/CONTRIBUTING.md)

<a name="history"></a>
## Release History

See [Changelog](CHANGELOG.md)

<a name="license"></a>
## License
Developed by Sagie Gur-Ari and licensed under the Apache 2 open source license.
