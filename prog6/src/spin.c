#include "spin.h"
#include "tas.h"
#include <sched.h>

void spin_lock(volatile char *lock)
{
    while (tas(lock))
        sched_yield(); // No need to check sched yeild because it always succeeds in the linux implementation
}

void spin_unlock(volatile char *lock)
{
    *lock = 0;
}