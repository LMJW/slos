/*
  kernel/inc/mm.h 
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/

#ifndef __MM_H__
#define __MM_H__
#include <stdint-gcc.h>
#include <frame_pool.h>
#include <page_table.h>
#include <vm_pool.h>

struct mm_struct {
	struct framepool kfp;
	struct pagetable pgt;
	struct vmpool heap;
};
void init_kernmem(struct framepool *kfp, 
		struct pagetable *pgt, 
		struct vmpool *kheap);
void init_pgt(void) __attribute__((section("PGT_INIT")));
void *kmalloc(uint32_t size);
void kfree(uint32_t addr);
#endif
