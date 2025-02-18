#include <devmgr/devmgr.h>
#include <arch/x86/cpuid.h>
#include <gfx/console.h>
#include <mm/vmm.h>
#include <util/mem.h>
#include <util/dynlist.h>
#include <util/stacktrace.h>
#include <arch/x86/apic/ioapic.h>
#include <devmgr/drives.h>

const char* dev_types[] = {
	TYPE_LIST
};

computer_x86* computer;

dynlist drives;

typedef struct _attr_packed callback_pair {
	void (*onconnect)(dev_misc*);
	void (*ondisconnect)(dev_misc*);
	dev_cb_filter filter;
} callback_pair;

dynlist callbacks;

void devmgr_init() {
	computer = kmalloc(sizeof(computer_x86));
	memset(computer, 0, sizeof(computer_x86));
	computer->hdr.type = DEV_ROOT;
	computer->acpi = 1;

	drives = dynlist_new(8, sizeof(dev_drive*));

	computer->ioapics = kmalloc(sizeof(dev_ioapic) * 1);
	computer->cpus = kmalloc(sizeof(dev_cpu) * 8);

	callbacks = dynlist_new(8, sizeof(callback_pair));
}

void devmgr_onchange(dev_misc* dev, enum signal sig) {
	u8 dev_subsys = dev->hdr.type & 0b11110000;
	// A callback_pair elejétől számítva a keresendő callback offszete
	// Ezt a signal alapján már hamar ki lehet találni, így
	// nem kell if-else-ezgetni
	void* callback_offset = (void*)((u64)offsetof(callback_pair, onconnect) + sig * sizeof(void (*)(void)));

	// dynlist_foreach(&callbacks, callback_pair*, i) {
	for (callback_pair* i = callbacks.list; (u64)i < (u64)callbacks.list + callbacks.current_count * callbacks.item_size; i = (void*)i + callbacks.item_size) {
		dev_cb_filter* f = &i->filter;
		u8 filter_subsys = f->type & 0b11110000;

		if (dev_subsys != filter_subsys) continue;

		switch (f->type & 0b00001111) {
			case DEV_TRIG_CLASS: {
				if (f->class.class == ((dev_misc_pci_data*)dev->hdr.handle)->class) {
					// A filter egyezik!
					// A callbacket le lehet most hívni:
					goto call;
				}
				break;
			}
			case DEV_TRIG_CLASS_SUBCLASS: {
				if (f->class.class == ((dev_misc_pci_data*)dev->hdr.handle)->class &&
				f->class.subclass == ((dev_misc_pci_data*)dev->hdr.handle)->subclass) {
					goto call;
				}
				break;
			}
			case DEV_TRIG_CLASS_SUBCLASS_PROGIF: {
				if (f->class.class == ((dev_misc_pci_data*)dev->hdr.handle)->class &&
				f->class.subclass == ((dev_misc_pci_data*)dev->hdr.handle)->subclass &&
				f->class.progif == ((dev_misc_pci_data*)dev->hdr.handle)->prog_if) {
					goto call;
				}
				break;
			}
			case DEV_TRIG_VENDOR_PRODUCT: {
				if (f->vp.vendor == ((dev_misc_pci_data*)dev->hdr.handle)->vendor &&
				f->vp.product == ((dev_misc_pci_data*)dev->hdr.handle)->product) {
					goto call;
				}
				break;
			}
		}

		continue;
	call:
		((void (*)(dev_misc*))(*(u64*)((u64)i + callback_offset)))(dev);
		return;
	}
}

void dev_add(void* vparent, void* vchild) {
	dev_hdr* parent = vparent;
	dev_hdr* child = vchild;
	if (!parent->children) {
		parent->children = kmalloc(sizeof(dev_hdr*) * 4);
	} else {
		parent->children = krealloc(parent->children, sizeof(dev_hdr*) * (parent->num_children + 1));
	}

	parent->children[parent->num_children] = child;
	parent->num_children++;

	if (child->type == DEV_ATA || child->type == DEV_ATAPI ||
	child->type == DEV_NVME || child->type == DEV_USB_MSD) {
		dynlist_append(&drives, &child);
		drive_init(vchild);
	}

	devmgr_onchange((dev_misc*) child, DEV_SIG_CONNECT);
}

void dev_set_callback(enum signal s, dev_cb_filter filter, void (*onconnect)(dev_misc*), void (*ondisconnect)(dev_misc*)) {
	dynlist_append(&callbacks, &(callback_pair) { .filter = filter, .onconnect = onconnect, .ondisconnect = ondisconnect });
}
