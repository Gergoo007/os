#include <stdio.h>
#include <stdlib.h>
#include <elf.h>

int main(int argc, char** argv) {
	if (argc != 5) {
		printf("Usage: %s <input> <symtab> <strtab> <shstrtab>\n", argv[0]);
		return 1;
	}

	FILE* in = fopen(argv[1], "r");
	FILE* symtab = fopen(argv[2], "w");
	FILE* strtab = fopen(argv[3], "w");
	FILE* shstrtab = fopen(argv[4], "w");

	fseek(in, 0, SEEK_END);
	unsigned long long s = ftell(in);
	fseek(in, 0, SEEK_SET);

	void* k = malloc(s);
	fread(k, s, 1, in);

	Elf64_Ehdr* e = k;
	Elf64_Shdr* sh = k + e->e_shoff;
	char second = 0;
	for (int i = 0; i < e->e_shnum; i++) {
		if (sh[i].sh_type == SHT_SYMTAB) {
			fwrite(k + sh[i].sh_offset, sh[i].sh_size, 1, symtab);
		} else if (sh[i].sh_type == SHT_STRTAB && second) {
			fwrite(k + sh[i].sh_offset, sh[i].sh_size, 1, shstrtab);
		} else if (sh[i].sh_type == SHT_STRTAB && !second) {
			fwrite(k + sh[i].sh_offset, sh[i].sh_size, 1, strtab);
			second = 1;
		}
	}

	fclose(in);
	fclose(symtab);
	fclose(strtab);
	fclose(shstrtab);
}
