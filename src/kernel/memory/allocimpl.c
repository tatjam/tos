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
	//klog("Allocating %i pages...", pages);
	void* ret = palloc(NULL, pages, false, true);
	//klog("@ 0x%p\n", ret);
	return ret;
}

int liballoc_free(void* ptr, int pages)
{
	//klog("Freeing %i pages...", pages);
	pfree(ptr, pages, NULL);
	return 0;
}