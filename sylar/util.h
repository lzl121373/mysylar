#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__
#include <sys/syscall.h>       /* Definition of  SYS_*  constants */
#include <unistd.h>
#include <stdint.h>

namespace sylar {
    
    pid_t GetThreadId();
    uint32_t GetFiberId();

}

#endif