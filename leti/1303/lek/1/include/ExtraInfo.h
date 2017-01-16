#ifndef EXTRA_INFO_H
#define EXTRA_INFO_H

#include <tbb/task_scheduler_init.h>

// Structure for common threads information.
struct ExtraInfo
{
    /// Auto-counted by TBB number of threads.
    static const unsigned int THREADS_NUM = tbb::task_scheduler_init::automatic;
    /// Brightness value choosen by user.
    static unsigned char currentBrightness;
    static bool debugMode;

};
#endif