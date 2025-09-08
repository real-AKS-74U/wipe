#include "scenes.h"
#include "styling.h"
#include "utils.h"
#include "wipe.h"

GtkWidget *stack;
char *waitingMsg;

void setWaitingMsg(char *msg) {
	waitingMsg = msg;
}

static int activate(GtkApplication *app, gpointer user_data) {
		GtkWidget *window;
		GtkWidget *button;

		window = gtk_application_window_new(app);
		gtk_window_set_title(GTK_WINDOW(window), TITLE);
		gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);

		stack = gtk_stack_new();

		sceneMain();
		sceneWaiting();
		sceneSelectDevice();
		// sceneWipeout(); // Dont intitalised it here cuz of reactivity issues selecting the device; instead after we select the device then

		loadCSS(cssRoot);
		gtk_stack_set_visible_child_name(GTK_STACK(stack), "scene-main");
		gtk_window_set_child(GTK_WINDOW(window), stack);
		gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
	GtkApplication *app;
	int status;

	app = gtk_application_new("xyz.aks74u.wipe", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);


	// cleanup
	cleanupUtils();

	return status;
}