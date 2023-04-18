#ifndef THREADPOOLTHREADAPIIMPL_H
#define THREADPOOLTHREADAPIIMPL_H

#include "threadpool.h"

struct ThreadPoolThreadAPI *threadpoolthreadapiimpl_new();
void threadpoolthreadapiimpl_release(struct ThreadPoolThreadAPI *);

#endif

