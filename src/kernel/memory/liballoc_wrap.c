#include "liballoc.h"
#include <asmutil.h>

// TODO: Spinlock for threading
int liballoc_lock()
{
	asm_cli();

	return 0;
}

int liballoc_unlock()
{
	asm_sti();

	return 0;
}

void* liballoc_alloc(int pages)
{
	return NULL;
}

int liballoc_free(void* ptr, int pages)
{
	return 0;
}