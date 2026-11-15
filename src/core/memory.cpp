#include "memory.h"

#include <malloc.h>

void releaseFreedMemory()
{
    malloc_trim(0);
}
