#ifndef WIPE_H
#define WIPE_H

#include <gtk/gtk.h>

#define HEIGHT 600
#define WIDTH  1000
#define TITLE "Wipe Tool - v0.0.1"

#define DEBUG false // TODO

extern GtkWidget *stack;
extern char *waitingMsg;

void setWaitingMsg(char *msg);

#endif // WIPE_H