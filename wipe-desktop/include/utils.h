#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>
#include <stdbool.h>

#define MAX_DEVICES 100

struct DeviceDetails {
	char *devnode;
	char *vendor;
	char *model;
	bool removable;
	char *bus;
	long long bytes;
};

struct DeviceArray {
	int length;
	struct DeviceDetails *devices;
};

extern struct DeviceDetails selectedDevice;
extern struct DeviceArray DEVICES;
extern const char *cssRoot;
extern const char *cssSceneMain;

char *trim(const char *str);
void findDevices();
void Callback_sceneMain_buttonGS(GtkWidget *widget, gpointer user_data);
void Callback_sceneSelectDevice_continueWB(GtkWidget *widget, gpointer user_data);
void Callback_sceneSelectDevice_checkboxSA(GtkWidget *widget, gpointer user_data);
void Callback_sceneEnd_continueEB(GtkWidget *widget, gpointer user_data);
void cleanupUtils();

#endif // UTILS_H