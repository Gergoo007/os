#include <stdio.h>
#include <stdlib.h>
#include <elf.h>

#define HDRLEN 0x8c

// Ez a header fix HDRLEN byte lesz; eme fix méret segítségével
// az adat mindig 4k-s határon kezdődik, egy kicsi linker script magic-kel
struct __attribute__((packed)) header {
	void* entry;
	unsigned int num_segs; // segs xddddddddddddd
	struct __attribute__((packed)) {
		unsigned int offset;
		unsigned int memsz;
		unsigned long long virtaddr;
	} segs[(HDRLEN - 12) / 16];
};

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: %s <input> <output>\n", argv[0]);
		return 1;
	}

	FILE* in = fopen(argv[1], "r");
	FILE* segs = fopen(argv[2], "w");

	fseek(in, 0, SEEK_END);
	unsigned long long s = ftell(in);
	fseek(in, 0, SEEK_SET);

	void* k = malloc(s);
	fread(k, s, 1, in);

	Elf64_Ehdr* e = k;
	Elf64_Phdr* ph = k + e->e_phoff;
	if (e->e_phnum > (HDRLEN-12) / 16) {
		printf("Több szegmens van a fájlban mint amennyit támogat a preloader! (%d vs %d)\n", e->e_phnum, (HDRLEN-12) / 16);
		return 1;
	}
	struct header h;
	h.num_segs = 0;
	h.entry = (void*)e->e_entry;
	unsigned int offs = 0;
	for (int i = 0; i < e->e_phnum; i++) {
		unsigned int alignedsz = ph[i].p_memsz & 0x0fff ? (ph[i].p_memsz | 0x0fff) + 1 : ph[i].p_memsz;
		printf("aligned: %x\n", alignedsz);
		h.segs[h.num_segs].offset = offs;
		offs += alignedsz;
		h.segs[h.num_segs].memsz = alignedsz;
		h.segs[h.num_segs].virtaddr = ph[i].p_vaddr;
		h.num_segs++;
	}

	printf("turi ip %lx\n", ftell(segs));
	fwrite(&h, sizeof(struct header), 1, segs);
	printf("turi ip %lx\n", ftell(segs));
	for (int i = 0; i < e->e_phnum; i++) {
		fwrite((void*)e + ph[i].p_offset, ph[i].p_filesz, 1, segs);
		unsigned int alignedsz = ph[i].p_memsz & 0x0fff ? (ph[i].p_memsz | 0x0fff) + 1 : ph[i].p_memsz;
		for (int j = ph[i].p_filesz; j < alignedsz; j++) {
			char c = 0;
			fwrite(&c, 1, 1, segs); // gondolom ez nem jó
		}

		printf("turi ip %lx\n", ftell(segs));
	}

	printf("\n");
	for (int i = 0; i < h.num_segs; i++) {
		printf("%d: %p; memsz %x; offs %x\n", i, (void*)h.segs[i].virtaddr, h.segs[i].memsz, h.segs[i].offset);
	}

	fclose(in);
	fclose(segs);
}
