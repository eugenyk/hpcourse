#ifndef THREADS_INFO_H
#define THREADS_INFO_H

#include <tbb/task_scheduler_init.h>

// Structure for common threads information.
struct ThreadsInfo
{
    /// Auto-counted by TBB number of threads.
    static const unsigned int THREADS_NUM = tbb::task_scheduler_init::automatic;
    // Coefficient of increasing number of threads.
    static const unsigned int THREADS_INC_COEF = 4;
    static unsigned char currentBrightness;
};
#endif