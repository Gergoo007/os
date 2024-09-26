#include <pci/pci_ids.h>

#include <util/string.h>

pci_pair vendors[] = {
	{ 0x8086, "Intel" },
	{ 0x80ee, "VirtualBox" },
	{ 0x1234, "Bochs" },
	{ 0x1002, "AMD" },
	{ 0x12d2, "NVidia" },
	{ 0x1b21, "ASMedia" },
	{ 0x1849, "ASRock" },
	{ 0x17aa, "Lenovo" },
	{ 0x17ef, "Lenovo" },
};

char* classes[] = {
	"Unclassified",
	"Mass Storage Controller",
	"Network Controller",
	"Display Controller",
	"Multimedia Controller",
	"Memory Controller",
	"Bridge",
	"Simple Communication Controller",
	"Base System Peripheral",
	"Input Device Controller",
	"Docking Station",
	"Processor",
	"Serial Bus Controller",
	"Wireless Controller",
};

char str[5];

char* pci_vendor(u16 id) {
	for (u32 i = 0; i < sizeof(vendors) / 10; i++) {
		if (vendors[i].id == id)
			return vendors[i].name;
	}

	hexn_to_str(id, str, 4);
	return str;
}

char* pci_class(u8 class) {
	if (class > 0x14) {
		hexn_to_str(class, str, 2);
		return str;
	}

	return classes[class];
}
