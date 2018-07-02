#pragma once

#ifdef ARCH_WIN
#include <windows.h>
#endif

#ifdef ARCH_LIN
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#endif

class ThreadPriority
{
public:
    /**
     * Attempts to boost priority of current thread, but not all
     * the way to a "realtime" priority. In genera, should not require
     * admin rights.
     */
    static bool boostNormal();

    /**
    * Attempts to boost priority of current thread all the way to
    * a "realtime" priority. Will require admin rights
    */
    static bool boostRealtime();

    static void restore();

#ifdef ARCH_WIN
    static bool boostRealtimeWindows();
#endif
private:

    static bool boostNormalPthread();
    static bool boostRealtimePthread();
    static void restorePthread();
#ifdef ARCH_LIN
    static bool boostNormalLinux();
#endif
};

#ifdef ARCH_WIN
inline bool ThreadPriority::boostRealtimeWindows()
{
    HANDLE h = GetCurrentProcess();
    auto x = GetPriorityClass(h);
    printf("cur priority class = %lx, realtime=%x", x, REALTIME_PRIORITY_CLASS);

    SetPriorityClass(h, HIGH_PRIORITY_CLASS);
    printf("set pri class to %x is now %lx\n", HIGH_PRIORITY_CLASS, GetPriorityClass(h));

    bool b = SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    int y = GetThreadPriority(GetCurrentThread());
    printf("set pri ret %d, is now %d want %d err=%ld\n",
        b,
        y,
        THREAD_PRIORITY_TIME_CRITICAL, GetLastError());
    fflush(stdout);
    return y == THREAD_PRIORITY_TIME_CRITICAL;

}
#endif

// Inside Visual Studio test we don't try to link in PThreads, 
// So they can't be used here. But they will work on all command line
// test builds, including Windows.
#if !defined(_MSC_VER) || defined(_VSCODE)
inline bool ThreadPriority::boostNormal()
{
#ifdef ARCH_LIN
    return  boostNormalLinux();
#else
    return boostNormalPthread();
#endif
}

inline bool ThreadPriority::boostRealtime()
{
#ifdef ARCH_WIN
    return boostRealtimeWindows();
#else
    return boostRealtimePthread();
#endif
}

inline void ThreadPriority::restore()
{
    restorePthread();
}

inline void ThreadPriority::restorePthread()
{
    const pthread_t threadHandle = pthread_self();
    struct sched_param params;
    params.sched_priority = 0;      // Note that on mac, this is not the default.
                                    // fix this.
    const int newPolicy = SCHED_OTHER;
    int x = pthread_setschedparam(threadHandle, newPolicy, &params);
    if (x != 0) {
        printf("failed to set reset sched %d\n", x);
    }
}


/**
 * Linux doesn't support pthread priorities, so
 * we use the common hack of setting niceness.
 */
#ifdef ARCH_LIN
inline bool  ThreadPriority::boostNormalLinux()
{
    pid_t tid = syscall(SYS_gettid);
    const int priority = -20;
    int ret = setpriority(PRIO_PROCESS, tid, priority);
    if (ret != 0) {
        printf("set pri failed, errno = %d\n", errno);
        printf("EACCESS=%d\n", EACCES);
    }
    printf("priority now %d\n", getpriority(PRIO_PROCESS, tid));
    return ret == 0;
}
#endif

inline bool ThreadPriority::boostNormalPthread()
{
    struct sched_param params;
    const pthread_t threadHandle = pthread_self();
    int initPolicy = -10;
    pthread_getschedparam(threadHandle, &initPolicy, &params);
    printf("in boost, policy was %d, pri was %d otherp=%d\n", initPolicy, params.sched_priority, SCHED_OTHER);

    const int initPriority = params.sched_priority;

    const int newPolicy = SCHED_OTHER;
    const int maxPriority = sched_get_priority_max(newPolicy);
#if 1
    {
        printf("for OTHER, pri range = %d,%d\n",
            sched_get_priority_min(newPolicy),
            sched_get_priority_max(newPolicy));
    }
#endif

    if (maxPriority <= initPriority) {
        // Linux seems to only offer one priority for SCHED_OTHER;
        printf("SCHED_OTHER only supports priority %d\n", maxPriority);
        return false;
    }

    params.sched_priority = maxPriority;
    int x = pthread_setschedparam(threadHandle, newPolicy, &params);
    if (x != 0) {
        printf("failed to set pri %d for SCHED_OTHER. error= %d\n", maxPriority, x);
    }
    fflush(stdout);
    return x == 0;
}

inline bool ThreadPriority::boostRealtimePthread()
{
    struct sched_param params;
    const pthread_t threadHandle = pthread_self();
    const int newPolicy = SCHED_RR;
    const int maxPriority = sched_get_priority_max(newPolicy);
    const int minPriority = sched_get_priority_min(newPolicy);


#if 0
    if ((maxPriority <= 0) || (minPriority < 0)) {
        printf("algorithm doesn't work with rt %d, %d\n", minPriority, maxPriority);
        return false;
    }
#endif

    // use the mean of min and max. These should all be higher than "non realtime" 
    // on mac the mean was not as good as elevating as other, to let's try max/
    //const int newPriority = (maxPriority + minPriority) / 2;
    const int newPriority = maxPriority;

    printf("realtime min = %d max = %d will use %d\n", minPriority, maxPriority, newPriority);
    params.sched_priority = newPriority;
    int x = pthread_setschedparam(threadHandle, newPolicy, &params);
    if (x != 0) {
        printf("failed to set pri %d for SCHED_RR. error= %d\n", newPriority, x);
    }
    return x == 0;
}

#else
inline bool ThreadPriority::boostNormal()
{
    return true;
}

inline bool ThreadPriority::boostRealtime()
{
    return true;
}
inline void ThreadPriority::restore()
{

}
#endif
