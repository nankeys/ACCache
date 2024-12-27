#ifndef __RANDOM_H_
#define __RANDOM_H_

#include "config.h"

void random_read_file(const workload_type& type, const int& snum);
void random_init();
static void *twitter_query_exec(void* param);
static void *meta_query_exec(void* param);
static void *ibm_query_exec(void* param);
void random_test(const workload_type& type, const int& snum); // Main Test

#endif //__RANDOM_H_