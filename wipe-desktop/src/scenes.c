#include <string.h>

#include "scenes.h"
#include "styling.h"
#include "utils.h"
#include "wipe.h"

void sceneMain() {
	GtkWidget *scene = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	GtkWidget *buttonGS = gtk_button_new_with_label("Get Started");

	// signals
	g_signal_connect(buttonGS, "clicked", G_CALLBACK(Callback_sceneMain_buttonGS), NULL);

	// add class names
	gtk_widget_add_css_class(scene, "scene-main");
	gtk_widget_add_css_class(scene, "scene");
	gtk_widget_add_css_class(buttonGS, "getstarted-btn");

	// append to box
	gtk_box_append(GTK_BOX(scene), buttonGS);

	// append scene to stack
	gtk_stack_add_named(GTK_STACK(stack), scene, "scene-main");

	// apply the css
	loadCSS(cssSceneMain);
}

void sceneWaiting() {
	GtkWidget *scene = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	GtkWidget *label = gtk_label_new(NULL);

	gtk_label_set_text(GTK_LABEL(label), waitingMsg);

	// add class names
	gtk_widget_add_css_class(scene, "scene");

	// append to box
	gtk_box_append(GTK_BOX(scene), label);

	// append scene to stack
	gtk_stack_add_named(GTK_STACK(stack), scene, "scene-wait");
}

void sceneSelectDevice() {
	GtkWidget *scene = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	GtkWidget *label = gtk_label_new("Select the storage device to wipe");
	GtkWidget *deviceList = gtk_drop_down_new(NULL, NULL);
	GtkWidget *continueWipeoutBtn = gtk_button_new_with_label("Continue");
	GtkWidget *checkboxShowAll = gtk_check_button_new_with_mnemonic("_Show all devices");


	// signals
	g_signal_connect(continueWipeoutBtn, "clicked", G_CALLBACK(Callback_sceneSelectDevice_continueWB), NULL);
	g_signal_connect(checkboxShowAll, "toggled", G_CALLBACK(Callback_sceneSelectDevice_checkboxSA), NULL);

	// add the options to the dropdown
	findDevices();
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

	// add class names
	gtk_widget_add_css_class(scene, "scene");
	gtk_widget_add_css_class(label, "label-scene-select");
	gtk_widget_add_css_class(continueWipeoutBtn, "continue-wipeout-btn");
	gtk_widget_add_css_class(checkboxShowAll, "showall-checkbox");

	// append to box
	gtk_box_append(GTK_BOX(scene), label);
	gtk_box_append(GTK_BOX(scene), deviceList);
	gtk_box_append(GTK_BOX(scene), checkboxShowAll);
	gtk_box_append(GTK_BOX(scene), continueWipeoutBtn);

	// append scene to stack
	gtk_stack_add_named(GTK_STACK(stack), scene, "scene-select-device");
}

void sceneWipeout() {
	GtkWidget *scene = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	GtkWidget *label = gtk_label_new("Core Logic for wipeout lies here");
	GtkWidget *labelTodo = gtk_label_new("TODO");
	GtkWidget *labelDetails = gtk_label_new(NULL);
	GtkWidget *continueEndBtn = gtk_button_new_with_label("Continue");

	// signals
	g_signal_connect(continueEndBtn, "clicked", G_CALLBACK(Callback_sceneEnd_continueEB), NULL);

	// update the details
	char details[2048] = "";
	const char *devnode = selectedDevice.devnode ?: "(no devnode)";
    const char *vendor = selectedDevice.vendor ?: "(unknown vendor)";
    const char *model = selectedDevice.model ?: "(unknown model)";
    const char *bus = selectedDevice.bus ?: "(unknown bus)";
    const char *rem = selectedDevice.removable ? "true" : "false";
    long long bytes = selectedDevice.bytes;
    double gb = (bytes > 0) ? (bytes / (1000.0 * 1000 * 1000)) : 0.0;

	char *devnodeDetails = g_strdup_printf("Devnode - %s\n", devnode);
	char *vendorDetails = g_strdup_printf("Vendor - %s\n", devnode);
	char *modelDetails = g_strdup_printf("Model - %s\n", model);
	char *busDetails = g_strdup_printf("Bus - %s\n", bus);
	char *removableDetails = g_strdup_printf("Removable = %s\n", rem);
	char *sizeDetails = g_strdup_printf("Size - %lld bytes / %.1f GB", bytes, gb);

	strcat(details, "Details:\n");
	strcat(details, devnodeDetails);
	strcat(details, vendorDetails);
	strcat(details, modelDetails);
	strcat(details, busDetails);
	strcat(details, removableDetails);
	strcat(details, sizeDetails);

	printf("%s\n", details);
	
	gtk_label_set_label(GTK_LABEL(labelDetails), details);

	// add class names
	gtk_widget_add_css_class(scene, "scene");
	gtk_widget_add_css_class(label, "label-scene-wipeout");
	gtk_widget_add_css_class(continueEndBtn, "continue-end-btn");

	// append to box
	gtk_box_append(GTK_BOX(scene), label);
	gtk_box_append(GTK_BOX(scene), labelTodo);
	gtk_box_append(GTK_BOX(scene), labelDetails);
	gtk_box_append(GTK_BOX(scene), continueEndBtn);

	// append scene to stack
	gtk_stack_add_named(GTK_STACK(stack), scene, "scene-wipeout");
}

void sceneEnd() {
	GtkWidget *scene = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	GtkWidget *label = gtk_label_new("Successfully wiped the device");
	GtkWidget *labelTodo = gtk_label_new("TODO: QR Generation from essential data from the device");

	// add class names
	gtk_widget_add_css_class(scene, "scene");
	gtk_widget_add_css_class(label, "label-scene-end");

	// append to box
	gtk_box_append(GTK_BOX(scene), label);
	gtk_box_append(GTK_BOX(scene), labelTodo);

	// append scene to stack
	gtk_stack_add_named(GTK_STACK(stack), scene, "scene-end");
}