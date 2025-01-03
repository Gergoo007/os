#include <modules/modules.h>
#include <util/mem.h>
#include <util/string.h>
#include <gfx/console.h>
#include <mm/paging.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

#define MODULE_BASE 0xffffffffb0000000
#define MODULE_SIZE 0x0000000002000000

u32 num_modules = 0;
module* modules = 0;

#include <serial/serial.h>

enum {
	R_X86_64_NONE = 0,       // No reloc
	R_X86_64_64 = 1,         // Direct 64 bit
	R_X86_64_PC32 = 2,       // PC relative 32 bit signed
	R_X86_64_GOT32 = 3,      // 32 bit GOT entry
	R_X86_64_PLT32 = 4,      // 32 bit PLT address
	R_X86_64_COPY = 5,       // Copy symbol at runtime
	R_X86_64_GLOB_DAT = 6,   // Create GOT entry
	R_X86_64_JUMP_SLOT = 7,  // Create PLT entry
	R_X86_64_RELATIVE = 8,   // Adjust by program base
	R_X86_64_GOTPCREL = 9,   // 32 bit signed PC relative offset to GOT
	R_X86_64_32 = 10,        // Direct 32 bit zero extended
	R_X86_64_32S = 11,       // Direct 32 bit sign extended
	R_X86_64_16 = 12,        // Direct 16 bit zero extended
	R_X86_64_PC16 = 13,      // 16 bit sign extended pc relative
	R_X86_64_8 = 14,         // Direct 8 bit sign extended
	R_X86_64_PC8 = 15,       // 8 bit sign extended pc relative

	// TLS relocations
	R_X86_64_DTPMOD64 = 16,  // ID of module containing symbol
	R_X86_64_DTPOFF64 = 17,  // Offset in module's TLS block
	R_X86_64_TPOFF64 = 18,   // Offset in initial TLS block
	R_X86_64_TLSGD = 19,     // 32 bit signed PC relative offset to two
							// GOT entries for GD symbol
	R_X86_64_TLSLD = 20,     // 32 bit signed PC relative offset to two
							// GOT entries for LD symbol
	R_X86_64_DTPOFF32 = 21,  // Offset in TLS block
	R_X86_64_GOTTPOFF = 22,  // 32 bit signed PC relative offset to GOT
							// entry for IE symbol
	R_X86_64_TPOFF32 = 23,   // Offset in initial TLS block

	R_X86_64_PC64 = 24,      // 64-bit PC relative
	R_X86_64_GOTOFF64 = 25,  // 64-bit GOT offset
	R_X86_64_GOTPC32 = 26,   // 32-bit PC relative offset to GOT

	R_X86_64_GOT64 = 27,     // 64-bit GOT entry offset
	R_X86_64_GOTPCREL64 = 28, // 64-bit PC relative offset to GOT entry
	R_X86_64_GOTPC64 = 29,   // 64-bit PC relative offset to GOT
	R_X86_64_GOTPLT64 = 30,  // Like GOT64, indicates that PLT entry needed
	R_X86_64_PLTOFF64 = 31,  // 64-bit GOT relative offset to PLT entry

	R_X86_64_SIZE32 = 32,
	R_X86_64_SIZE64 = 33,

	R_X86_64_GOTPC32_TLSDESC = 34, // 32-bit PC relative to TLS descriptor in GOT
	R_X86_64_TLSDESC_CALL = 35,    // Relaxable call through TLS descriptor
	R_X86_64_TLSDESC = 36,         // 2 by 64-bit TLS descriptor
	R_X86_64_IRELATIVE = 37,          // Adjust indirectly by program base
	R_X86_64_RELATIVE64 = 38,      // 64-bit adjust by program base
	R_X86_64_PC32_BND = 39,  // PC relative 32 bit signed with BND prefix
	R_X86_64_PLT32_BND = 40, // 32 bit PLT address with BND prefix
	R_X86_64_GOTPCRELX = 41, // 32 bit signed PC relative offset to GOT
				// without REX nor REX2 prefixes, relaxable.
	R_X86_64_REX_GOTPCRELX = 42, // 32 bit signed PC relative offset to GOT
					// with REX prefix, relaxable.
	R_X86_64_CODE_4_GOTPCRELX = 43, // 32 bit signed PC relative offset to
					// GOT if the instruction starts at 4
					// bytes before the relocation offset,
					// relaxable.
	R_X86_64_CODE_4_GOTTPOFF = 44,  // 32 bit signed PC relative offset to
					// GOT entry for IE symbol if the
					// instruction starts at 4 bytes before
					// the relocation offset.
	R_X86_64_CODE_4_GOTPC32_TLSDESC = 45, // 32-bit PC relative to TLS
						// descriptor in GOT if the
						// instruction starts at 4 bytes
						// before the relocation offset.
	R_X86_64_CODE_5_GOTPCRELX = 46, // 32 bit signed PC relative offset to
					// GOT if the instruction starts at 5
					// bytes before the relocation offset,
					// relaxable.
	R_X86_64_CODE_5_GOTTPOFF = 47,  // 32 bit signed PC relative offset to
					// GOT entry for IE symbol if the
					// instruction starts at 5 bytes before
					// the relocation offset.
	R_X86_64_CODE_5_GOTPC32_TLSDESC = 48, // 32-bit PC relative to TLS
						// descriptor in GOT if the
						// instruction starts at 5 bytes
						// before the relocation offset.
	R_X86_64_CODE_6_GOTPCRELX = 49, // 32 bit signed PC relative offset to
					// GOT if the instruction starts at 6
					// bytes before the relocation offset,
					// relaxable.
	R_X86_64_CODE_6_GOTTPOFF = 50,  // 32 bit signed PC relative offset to
					// GOT entry for IE symbol if the
					// instruction starts at 6 bytes before
					// the relocation offset.
	R_X86_64_CODE_6_GOTPC32_TLSDESC = 51, // 32-bit PC relative to TLS
						// descriptor in GOT if the
						// instruction starts at 6 bytes
						// before the relocation offset.
	// GNU vtable garbage collection extensions.
	R_X86_64_GNU_VTINHERIT = 250,
	R_X86_64_GNU_VTENTRY = 251
};

u64 get_kernel_symbol(char* name, void* m) {
	// KERNEL SZIMBÓLUMOK
	{
		// TODO: kernel .(ro)data használata a modulban
		u32 i = 0;
		while (i * sizeof(Elf64_Sym) < ksymtab_size) {
			if (!strcmp(kstrtab + ksymtab[i].st_name, name)) {
				sprintk("match; val: %p\n\r", ksymtab[i].st_value);
				return ksymtab[i].st_value;
			}
			i++;
		}
	}

	// SAJÁT SZIMBÓLUMOK
	{
		Elf64_Ehdr* e = m;
		void* strtab1 = 0;
		void* strtab2 = 0;
		Elf64_Sym* symtab = 0;
		u64 symtab_size;
		// két találat lesz: .strtab, .shstrtab
		// remélhetőleg ebben a sorrendben
		Elf64_Shdr* sh = m + e->e_shoff;
		for (u32 i = 0; i < e->e_shnum; i++, sh++) {
			if (sh->sh_type == SHT_STRTAB) {
				if (!strtab1) {
					strtab1 = m + sh->sh_offset;
				} else {
					strtab2 = m + sh->sh_offset;
				}
			} else if (sh->sh_type == SHT_SYMTAB) {
				symtab = m + sh->sh_offset;
				symtab_size = sh->sh_size;
			}
		}

		u32 i = 0;
		while (i * sizeof(Elf64_Sym) < symtab_size) {
			if (!strcmp(strtab1 + symtab[i].st_name, name)) {
				return symtab[i].st_value;
			} else if (!strcmp(strtab2 + symtab[i].st_name, name)) {
				return symtab[i].st_value;
			}
			i++;
		}
	}
	return -1;
}

u32 module_link(void* m) {
	// LINKELÉS
	Elf64_Ehdr* e = m;
	printk("magic %.4s\n", m);
	u64 address = MODULE_BASE;
	void* strtab1 = 0;
	void* strtab2 = 0;
	Elf64_Sym* symtab = 0;
	void* entry;
	u64 symtab_size;

	// két találat lesz: .strtab, .shstrtab
	// remélhetőleg ebben a sorrendben
	Elf64_Shdr* sh = m + e->e_shoff;
	for (u32 i = 0; i < e->e_shnum; i++, sh++) {
		// Section elhelyezése memóriában, ha betöltendő
		if (sh->sh_flags & SHF_ALLOC) {
			sh->sh_addr = address;
			address += sh->sh_size;
		}

		if (sh->sh_type == SHT_STRTAB) {
			if (!strtab1) {
				strtab1 = m + sh->sh_offset;
			} else {
				strtab2 = m + sh->sh_offset;
			}
		} else if (sh->sh_type == SHT_SYMTAB) {
			symtab = m + sh->sh_offset;
			symtab_size = sh->sh_size;
		}
	}

	Elf64_Rela* rela;
	u32 num_ents = 0;

	sh = m + e->e_shoff;
	for (u32 i = 0; i < e->e_shnum; i++) {
		if (sh[i].sh_type == SHT_RELA) {
			rela = m + sh[i].sh_offset;
			num_ents = sh[i].sh_size / sizeof(Elf64_Rela);
			Elf64_Shdr* toreloc = &sh[sh[i].sh_info];

			for (u32 k = 0; k < num_ents; k++, rela++) {
				char* search = strtab1 + symtab[rela->r_info >> 32].st_name;
				u64 to;
				if (!*search) search = strtab2 + symtab[rela->r_info >> 32].st_name;
				if (!*search) {
					// Hiányzik az st_name; mert például literal-ról van szó
					// Így nem lehet név alapján keresni
					// to = (u64)sh[symtab[rela->r_info >> 32].st_shndx].sh_addr + symtab[rela->r_info >> 32].st_value + rela->r_addend - rela->r_offset;
					to = sh[symtab[rela->r_info >> 32].st_shndx].sh_addr;
				} else {
					to = get_kernel_symbol(search, m);
				}

				printk("reloc type %d: ", rela->r_info & 0xff);

				// A: addend used to compute the value of the relocatable field.
				// B: base address at which a shared object has been loaded into memory during execution. Generally, a shared object is built with a 0 base virtual address, but the execution address will be different.
				// G: offset into the global offset table at which the relocation entry’s symbol will reside during execution. GOT Represents the address of the global offset table.
				// L: place (section offset or address) of the Procedure Linkage Table entry for a symbol.
				// P: place (section offset or address) of the storage unit being relocated (computed using r_offset).
				// S: value of the symbol whose index resides in the relocation entry.
				// Z: size of the symbol whose index resides in the relocation entry.
				switch (rela->r_info & 0xff) {
					// S + A - P
					case R_X86_64_PC32: {
						to += symtab[rela->r_info >> 32].st_value + rela->r_addend - rela->r_offset - toreloc->sh_addr;
						break;
					}

					// L + A - P
					case R_X86_64_PLT32: {
						to += symtab[rela->r_info >> 32].st_value + rela->r_addend - rela->r_offset;
						if (
							ELF64_ST_TYPE(symtab[rela->r_info >> 32].st_info) == STT_NOTYPE &&
							ELF64_ST_BIND(symtab[rela->r_info >> 32].st_info) == STB_GLOBAL
						) {
							// A szimbólum globális, tehát bele kell számítani a memóricímet
							to -= toreloc->sh_addr;
						}
						break;
					}

					// Undocumented
					case R_X86_64_REX_GOTPCRELX: {
						to += rela->r_addend - rela->r_offset - toreloc->sh_addr;

						// A GOT-ot nem használom, ezért nem onnan töltöm be pointert
						// Tehát mov helyett lea-ra kell állítani az utasítást
						// A cím előtti 2. byte 2. bitét kell 1-re állítani, attól lesz lea
						u8* byte = m + toreloc->sh_offset + rela->r_offset - 2;
						if (*byte == 0x8b)
							*byte = 0x8d;
						else
							printk("Ismeretlen byte: %02x @ %p\n", *byte, toreloc->sh_addr + rela->r_offset);

						break;
					}

					default: {
						printk("!!!!!!!!!! reloc type %02x nincs kezelve\n", rela->r_info & 0xff);
						break;
					}
				}

				printk("[%s] %p\n", search, (u32)to);
				u32* addr = m + toreloc->sh_offset + rela->r_offset;
				*addr = (u32)to;
			}
		}
		
		if (!strcmp(strtab2 + sh[i].sh_name, ".text")) {
			// Entry point megkeresése
			u32 l = 0;
			while (l * sizeof(Elf64_Sym) < (u64)symtab_size) {
				if (!strcmp(strtab1 + symtab[l].st_name, "mod_main")) {
					entry = (void*)symtab[l].st_value + sh[i].sh_addr;
				}

				l++;
			}
		}
	}

	map_page(MODULE_BASE, (u64)pmm_alloc(), 0b11);
	for (u32 i = 0; i < e->e_shnum; i++) {
		if (sh[i].sh_flags & SHF_ALLOC) {
			// map_page(sh[i].sh_addr, pmm_alloc(), 0b11);
			printk("[%s] %p -> %p; %d\n", strtab2 + sh[i].sh_name, m + sh[i].sh_offset, sh[i].sh_addr, sh[i].sh_size);
			memcpy((void*)sh[i].sh_addr, m + sh[i].sh_offset, sh[i].sh_size);
		}
	}

	if (!modules) modules = kmalloc(sizeof(module) * 256);
	modules[num_modules].entry = entry;

	return num_modules++;
}
