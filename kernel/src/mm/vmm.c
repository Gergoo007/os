#include <mm/vmm.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <util/mem.h>

bitmap vmm_bm;

#include <gfx/console.h>
#include <serial/serial.h>

#define KHEAPBM	0xffffb00000000000
#define KHEAP 	0xffffc00000000000
#define KMMIO 	0xffffd00000000000

vmem* first_link;

bitmap region_bm;

u64 kmmio_marker = 0xffffd00000000000;

u64 vmm_mem_used;
u64 vmm_mem_free;

static vmem* new_link() {
	return first_link + bm_alloc(&region_bm);
}

static void delete_link(vmem* l) {
	bm_set(&region_bm, (l - first_link) >> 5, 0);
}

void vmm_dump() {
	vmem* m = first_link;
	sprintk("====================\n\r");
	while (m) {
		sprintk("%d; %p byte\n\r", m->sts, m->len);
		m = m->next;
	}
	sprintk("====================\n\r");
}

void vmm_init() {
	vmm_mem_free = pmm_mem_free;
	vmm_mem_used = pmm_mem_used;

	for (u32 i = 0; i != 0x200000; i += 0x1000) {
		map_page(KHEAPBM+i, (u64)pmm_alloc(), 0b11);
	}

	first_link = (void*)KHEAPBM;
	// Kivonok ~2M-et a kernel meg egyéb fenn nem tartott
	// használatban lévő memóriáért
	first_link->len = pmm_mem_free - 0x200000;
	first_link->sts = 0;
	first_link->prev = 0;
	first_link->next = 0;

	region_bm.base = vmm_alloc();
	region_bm.size = 0x1000;
	bm_init(&region_bm);

	bm_set(&region_bm, 0, 1);
}

__attribute__((__malloc__)) void* kmalloc(u64 bytes) {
	if (!bytes) return 0;

	if (bytes < 16)
		bytes = 16;
	else
		if (bytes & 15)
			bytes = (bytes | 15) + 1;

	vmm_mem_used += bytes;
	vmm_mem_free -= bytes;

	u64 a = KHEAP;
	vmem* l = first_link;
	while (l) {
		if (l->sts == 0) { // free?
			// Van szabad hely?
			if (l->len > bytes) {
				// Új láncszem készítése
				vmem* new = new_link();
				new->prev = l->prev;
				new->next = l;
				new->sts = 1;
				new->len = bytes;

				// Láncszem beillesztése
				if (l->prev)
					l->prev->next = new;
				l->prev = new;
				l->len -= bytes;

				// Ha ez volt a legelső allocation
				if (first_link == l)
					first_link = new;
			} else if (l->len == bytes) {
				l->sts = 1;
			}
			
			kpremap((void*)a);
			return (void*)a;
		}

		a += l->len;
		l = l->next;
	}

	return 0;
}

// TODO: -O3-al kompatibilis??
void kpremap(void* p) {
	u64 a = KHEAP;
	vmem* l = first_link;
	while (l) {
		if (a == (u64)p) {
			// Align
			u64 toalign = (u64)p+l->len;
			if (toalign & 0x0fff) {
				toalign |= 0x0fff;
				toalign++;
			}

			for (u64 i = (u64)p; i < toalign; i += 0x1000) {
				// Kell egy olvasást végezni, hogy megizonyosodjunk róla
				// hogy mappelve van fizikai címre ez a page
				asm volatile ("" :: "r"(*(char*)i));
				// printk("page: %p\n", i);
			}
			return;
		}

		a += l->len;
		l = l->next;
	}
}

// Ezt nem lehet futtatni??? TODO: "Utánajárni"
void kfree(void* p) {
	// u64 a = KHEAP;
	// vmem* l = first_link;
	// while (l) {
	// 	if (a == (u64)p) {
	// 		// Szabadnak jelölés
	// 		l->sts = 0;
	// 		vmm_mem_used -= l->len;
	// 		vmm_mem_free += l->len;

	// 		// Utána lévő szabad szakasszal
	// 		// összeolvasztás (l törlése)
	// 		if (l->next) {
	// 			if (l->next->sts == 0) {
	// 				l->next->len += l->len;

	// 				// Láncszem kikerülése (effektíven törlése)
	// 				if (l->prev)
	// 					l->prev->next = l->next;
	// 				if (l->next)
	// 					l->next->prev = l->prev;

	// 				if (l == first_link)
	// 					first_link = l->next;

	// 				delete_link(l);
	// 			}
	// 		}

	// 		// Előzővel szakasszal való
	// 		// összeolvasztás (l törlése)
	// 		if (l->prev) {
	// 			if (l->prev->sts == 0) {
	// 				l->prev->len += l->len;

	// 				// Láncszem kikerülése (effektíven törlése)
	// 				if (l->next)
	// 					l->next->prev = l->prev;
	// 				if (l->prev)
	// 					l->prev->next = l->next;

	// 				delete_link(l);
	// 			}
	// 		}

	// 		return;
	// 	}

	// 	a += l->len;
	// 	l = l->next;
	// }

	// error("free: Érvénytelen pointer!");
}

void* krealloc(void* p, u64 new) {
	if (new & 0b1111) new = (new | 0b1111) + 1;

	if (!new) {
		kfree(p);
		return 0;
	}

	if (!p)
		return kmalloc(new);

	u64 a = KHEAP;
	vmem* l = first_link;
	u64 oldsize;
	while (l) {
		if (a == (u64)p) {
			oldsize = l->len;
			// Ha 16 byte alatti az effektív növekedés,
			// nem kell hogy történjen semmi a 16 byte-os igazítás miatt
			if ((new - oldsize) < 16) return p;

			if (l->len < new) {
				// Meg kell nagyobbítani
				if (l->next) {
					vmem* next = l->next;
					if (next->sts == 0 && next->len >= new-oldsize) {
						// Van utána elég szabad hely
						if (next->len == new) {
							// Éppen akkora, tehát a next-et meg kell semmisíteni
							l->next = next->next;
							next->next->prev = l;
							delete_link(next);
						} else {
							// Nagyobb
							next->len -= (new - oldsize);
						}

						l->len = new;
						return p;
					} else {
						// Nincs utána hely
						void* p2 = kmalloc(new);
						memcpy(p2, p, oldsize);

						// Régi hely szabaddá tétele
						kfree(p);

						return p2;
					}
				}
			} else if (l->len > new) {
				// Csökkenteni kell
				if (l->next) {
					if (l->next->sts == 0) {
						// Az utána lévő szegmens szabad
						l->next->len += oldsize - new;
						l->len = new;
						return p;
					} else {
						// Nem szabad a következő, újat kell csinálni a kettő közé
						vmem* newlink = new_link();
						newlink->len = oldsize - new;
						newlink->sts = 0;
						
						l->next->next->prev = newlink;
						newlink->prev = l;
						newlink->next = l->next->next;
						l->next = newlink;

						l->len = new;
						return p;
					}
				}
			}
		}

		a += l->len;
		l = l->next;
	}
	return 0;
}

void* vmm_alloc() {
	return pmm_alloc() + 0xffff800000000000;
}

void vmm_free(void* a) {
	pmm_free(a - 0xffff800000000000);
}

void* vmm_map_mmio(u64 phys, u64 bytes) {
	if (bytes & 0x0fff) bytes = (bytes | 0x0fff) + 1; // page-align

	u64 addr = kmmio_marker;

	while (bytes) {
		map_page(kmmio_marker, phys, 0b11); // Attribútumok maradhatnak (Uncached)

		bytes -= 0x1000;
		kmmio_marker += 0x1000;
	}

	return (void*)addr;
}
