#include <ctype.h>
#include <libudev.h>
#include <stdlib.h>
#include <string.h>

#include "scenes.h"
#include "utils.h"
#include "wipe.h"

#define SAFE_STRDUP_AND_TRIM(s) \
    ((s) ? trim(strdup(s)) : NULL)

// Naming scheme for callbacks `Callback_<scene>_<widgetName>`

struct DeviceDetails selectedDevice = {NULL, NULL, NULL, false, NULL, (long long)0};
struct DeviceArray DEVICES = {0, NULL};

const char *cssRoot =
":root {"				\
"	background: #55F;"	\
"	font-weight: 700;"	\
"}"						\
".scene {"				\
"	font-size: 18px;"	\
"	margin: 10px;"		\
"}";

const char *cssSceneMain =
	".getstarted-btn {"				\
	"	background: #5F5;"			\
	"	margin: 50px;"				\
	"	font-weight: 700;"			\
	"	border-color: #3C3;"		\
	"	border-width: 4px;"			\
	"	min-height: 70px;"			\
	"	min-width: 100px;"			\
	"	font-size: 35px;"			\
	"}"								\
	".getstarted-btn:hover {"		\
	"	background: #4E4;"			\
	"}";

char *trim(const char *str) {
    if (str == NULL) return NULL;

    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return strdup("");

    const char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    size_t len = end - str + 1;
    char *out = malloc(len + 1);
    memcpy(out, str, len);
    out[len] = '\0';
    return out;
}

static long long get_size_in_bytes(const char *sys_path) {
    char size_path[512];
    snprintf(size_path, sizeof(size_path), "%s/size", sys_path);

    FILE *f = fopen(size_path, "r");
    if (!f) return -1;

    long long sectors = 0;
    fscanf(f, "%lld", &sectors);
    fclose(f);

    // Size = sectors * 512 bytes
    return sectors * 512LL;
}

void findDevices() {
	// allocate the memory
	DEVICES.devices = malloc(sizeof(struct DeviceDetails) * MAX_DEVICES);

	if (!DEVICES.devices) {
		fprintf(stderr, "malloc failed\n");
        exit(1);
	}

	struct udev *udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Failed to init udev\n");
        return;
    }

    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "disk"); // disks only
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);

        const char *devnode = udev_device_get_devnode(dev);
        if (!devnode) {
            udev_device_unref(dev);
            continue;
        }

        // sysfs attributes
        char *vendor = udev_device_get_sysattr_value(dev, "device/vendor");
        char *model  = udev_device_get_sysattr_value(dev, "device/model");
        char *removable = udev_device_get_sysattr_value(dev, "removable");
        char *bus = udev_device_get_property_value(dev, "ID_BUS");

        long long bytes = get_size_in_bytes(udev_device_get_syspath(dev));

        // fallback for nvme
        if (bus == NULL && devnode && strncmp(devnode, "/dev/nvme", 9) == 0) {
            bus = "nvme";
        }

        struct DeviceDetails details = {
		    devnode ? SAFE_STRDUP_AND_TRIM(devnode) : NULL,
		    vendor ? SAFE_STRDUP_AND_TRIM(vendor) : NULL,
		    model ? SAFE_STRDUP_AND_TRIM(model) : NULL,
		    (removable && strcmp(removable, "1") == 0) ? true : false,
		    bus ? SAFE_STRDUP_AND_TRIM(bus) : NULL,
		    bytes
		};

        DEVICES.devices[DEVICES.length] = details;
        DEVICES.length++;

        if (DEVICES.length >= MAX_DEVICES) {
            udev_device_unref(dev);
            break;
        }

        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return;
}

void listDevices() {
    if (!DEBUG) return;
    printf("Detected drives:\n\n");

    for(int i = 0; i < DEVICES.length; i++) {
    	double gb = (DEVICES.devices[i].bytes > 0) ? (DEVICES.devices[i].bytes / (1000.0 * 1000 * 1000)) : 0.0;

    	printf("[ %s ] ", DEVICES.devices[i].devnode);
        if (DEVICES.devices[i].vendor) printf("%s ", DEVICES.devices[i].vendor);
        if (DEVICES.devices[i].model)  printf("%s ", DEVICES.devices[i].model);
        if (gb > 0) printf("— %.1f GB ", gb);
        if (DEVICES.devices[i].removable) printf("— Removable ");
        if (DEVICES.devices[i].bus) printf("— %s ", DEVICES.devices[i].bus);

        printf("\n");
    }
}

void Callback_sceneMain_buttonGS(GtkWidget *widget, gpointer user_data) {
	// go to waiting scene while we're fetching the devices
	setWaitingMsg("Fetching the device details...");
	gtk_stack_set_visible_child_name(GTK_STACK(stack), "scene-wait");
	listDevices();

	// switch the scene to sceneSelectDevice
	gtk_stack_set_visible_child_name(GTK_STACK(stack), "scene-select-device");
}

void Callback_sceneSelectDevice_continueWB(GtkWidget *widget, gpointer user_data) {
    // go to waiting scene while we're fetching the devices
    setWaitingMsg("Loading...");
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "scene-wait");

    // find the dropdown
    GtkWidget *parent = gtk_widget_get_parent(widget);
    GtkWidget *deviceList = NULL;
    for(GtkWidget *child = gtk_widget_get_first_child(parent);
        child != NULL;
        child = gtk_widget_get_next_sibling(child)) {
        if (GTK_IS_DROP_DOWN(child)) {
            deviceList = child;
            break;
        }
    }
    if (deviceList == NULL) {
        fprintf(stderr, "Failed finding the dropdown\n");
        return;
    }

    // find the checkbox
    GtkWidget *checkboxShowAll = NULL;
    for(GtkWidget *child = gtk_widget_get_first_child(parent);
        child != NULL;
        child = gtk_widget_get_next_sibling(child)) {
        if (GTK_IS_CHECK_BUTTON(child)) {
            checkboxShowAll = child;
            break;
        }
    }
    if (checkboxShowAll == NULL) {
        fprintf(stderr, "Failed finding the checkbox\n");
        return;
    }

    // find the selected "DeviceDetails" and save it to "selectedDevice"
    guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(deviceList));
    gboolean isActive = gtk_check_button_get_active(GTK_CHECK_BUTTON(checkboxShowAll));
    int idx = 0;
    if (isActive) {
        // iterate normally and select the device
        for(guint idx = 0; idx < (guint)DEVICES.length; idx++) {
            if (idx == selected) {
                selectedDevice = DEVICES.devices[idx];
                break;
            }
        }
    } else {
        guint idx2 = 0;
        for(guint idx = 0; idx < (guint)DEVICES.length; idx++) {
            if (DEVICES.devices[idx].bus == NULL || g_strcmp0(DEVICES.devices[idx].bus, "ata") == 1 || g_strcmp0(DEVICES.devices[idx].bus, "nvme") == 1) continue;
            if (idx2 == selected) {
                selectedDevice = DEVICES.devices[idx2];
                break;
            }
            idx2++;
        }
    }

    if (selectedDevice.devnode == NULL) {
        fprintf(stderr, "failed selecting from dropdown\n");
        exit(1);
    }

    // switch the scene to sceneWipeout
    sceneWipeout();
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "scene-wipeout");
}

void Callback_sceneSelectDevice_checkboxSA(GtkWidget *widget, gpointer user_data) {
	gboolean isActive = gtk_check_button_get_active(GTK_CHECK_BUTTON(widget));

    // find the dropdown
    GtkWidget *parent = gtk_widget_get_parent(widget);
    GtkWidget *deviceList = NULL;
    for(GtkWidget *child = gtk_widget_get_first_child(parent);
        child != NULL;
        child = gtk_widget_get_next_sibling(child)) {
        if (GTK_IS_DROP_DOWN(child)) {
            deviceList = child;
            break;
        }
    }

    if (deviceList == NULL) {
        fprintf(stderr, "Failed finding the dropdown\n");
        return;
    }

	if (isActive) {
        // no checks, just list everything
		GtkStringList *list = gtk_string_list_new(NULL);
        for (int i = 0; i < DEVICES.length; i++) {
            const char *devnode = DEVICES.devices[i].devnode ?: "(no devnode)";
            const char *vendor = DEVICES.devices[i].vendor ?: "(unknown vendor)";
            const char *model = DEVICES.devices[i].model ?: "(unknown model)";
            const char *bus = DEVICES.devices[i].bus ?: "(unknown bus)";
            const char *rem = DEVICES.devices[i].removable ? "removable" : "internal";

            double gb = (DEVICES.devices[i].bytes > 0) ? (DEVICES.devices[i].bytes / (1000.0 * 1000 * 1000)) : 0.0;

            char *label = g_strdup_printf("%s - %s - %s - %s - %s - %.1f GB", devnode, vendor, model, rem, bus, gb);
            gtk_string_list_append(list, label);
            g_free(label);
        }
        gtk_drop_down_set_model(GTK_DROP_DOWN(deviceList), G_LIST_MODEL(list));
	} else {
        // only ata (hdd/ssd) and nvme (ssd) devices will be shown
		GtkStringList *list = gtk_string_list_new(NULL);
        for (int i = 0; i < DEVICES.length; i++) {
            if (DEVICES.devices[i].bus == NULL || g_strcmp0(DEVICES.devices[i].bus, "ata") == 1 || g_strcmp0(DEVICES.devices[i].bus, "nvme") == 1) continue;

            const char *devnode = DEVICES.devices[i].devnode ?: "(no devnode)";
            const char *vendor = DEVICES.devices[i].vendor ?: "(unknown vendor)";
            const char *model = DEVICES.devices[i].model ?: "(unknown model)";

            double gb = (DEVICES.devices[i].bytes > 0) ? (DEVICES.devices[i].bytes / (1000.0 * 1000 * 1000)) : 0.0;

            char *label = g_strdup_printf("%s - %s - %s - %.1f GB", devnode, vendor, model, gb);
            gtk_string_list_append(list, label);
            g_free(label);
        }
        gtk_drop_down_set_model(GTK_DROP_DOWN(deviceList), G_LIST_MODEL(list));
	}
}

void Callback_sceneEnd_continueEB(GtkWidget *widget, gpointer user_data) {
    // initialise the sceneEnd
    sceneEnd();
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "scene-end");
}

void cleanupUtils() {
	// free the memory
	free(DEVICES.devices);
}