#pragma once

#include <pcie/pcie.h>

#include <util/attrs.h>

typedef struct pci_pair {
	u16 id;
	char* name;
} pci_pair;

extern pci_pair vendors[9];
extern char* classes[14];

char* pci_vendor(u16 id);
char* pci_class(u8 class);
