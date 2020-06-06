#ifndef LOGGER_H
#define LOGGER_H

#pragma GCC system_header

#include <stdio.h>

#define LOG_DEBUG(fmt, ...)                                     printf("DBG | %s():%d | " fmt "\n",  __func__,  __LINE__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)                                      printf("INF | %s():%d | " fmt "\n",  __func__,  __LINE__, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...)                                   printf("WRG | %s():%d | " fmt "\n",  __func__,  __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)                                     printf("ERR | %s():%d | " fmt "\n",  __func__,  __LINE__, ##__VA_ARGS__)

#define LOG_ASSERT(COND)                                        do{if(!(COND)){fprintf(stderr, "Assertion in file %s at line %d\n", __FILE__, __LINE__);abort();}}while(0)

#endif /* LOGGER_H */