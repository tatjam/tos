#include "liballoc.h"


// TODO: Spinlock for threading
int liballoc_lock()
{
	asm_cli();
}

int liballoc_unlock()
{
	asm_sti();
}

void* liballoc_alloc(int pages)
{

}

int liballoc_free(void* ptr, int pages)
{

}