#include "osdialog.h"
#include <assert.h>
#include <string.h>
#include <gtk/gtk.h>



int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message) {
	assert(gtk_init_check(NULL, NULL));

	GtkMessageType messageType;
	switch (level) {
		default:
		case OSDIALOG_INFO: messageType = GTK_MESSAGE_INFO; break;
		case OSDIALOG_WARNING: messageType = GTK_MESSAGE_WARNING; break;
		case OSDIALOG_ERROR: messageType = GTK_MESSAGE_ERROR; break;
	}

	GtkButtonsType buttonsType;
	switch (buttons) {
		default:
		case OSDIALOG_OK: buttonsType = GTK_BUTTONS_OK; break;
		case OSDIALOG_OK_CANCEL: buttonsType = GTK_BUTTONS_OK_CANCEL; break;
		case OSDIALOG_YES_NO: buttonsType = GTK_BUTTONS_YES_NO; break;
	}

	GtkWidget *dialog = gtk_message_dialog_new (NULL,
		0,
		messageType,
		buttonsType,
		"%s", message);

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();

	switch (result) {
		case GTK_RESPONSE_OK:
		case GTK_RESPONSE_YES:
			return 1;
		default:
			return 0;
	}
}


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, osdialog_filters *filters) {
   printf("xxx osdialog_file: 1\n");
	assert(gtk_init_check(NULL, NULL));
   printf("xxx osdialog_file: 2\n");

	GtkFileChooserAction gtkAction;
	const char *title;
	const char *acceptText;
	if (action == OSDIALOG_OPEN) {
		title = "Open File";
		acceptText = "Open";
		gtkAction = GTK_FILE_CHOOSER_ACTION_OPEN;
	}
	else if (action == OSDIALOG_OPEN_DIR) {
		title = "Open Folder";
		acceptText = "Open Folder";
		gtkAction = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	}
	else {
		title = "Save File";
		acceptText = "Save";
		gtkAction = GTK_FILE_CHOOSER_ACTION_SAVE;
	}

   printf("xxx osdialog_file: 3\n");

	GtkWidget *dialog = gtk_file_chooser_dialog_new(
		title,
		NULL,
		gtkAction,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		acceptText, GTK_RESPONSE_ACCEPT,
		NULL);

   printf("xxx osdialog_file: 4\n");

	for (; filters; filters = filters->next) {
		GtkFileFilter *fileFilter = gtk_file_filter_new();
		gtk_file_filter_set_name(fileFilter, filters->name);
		for (osdialog_filter_patterns *patterns = filters->patterns; patterns; patterns = patterns->next) {
			char patternBuf[1024];
			snprintf(patternBuf, sizeof(patternBuf), "*.%s", patterns->pattern);
			gtk_file_filter_add_pattern(fileFilter, patternBuf);
		}
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), fileFilter);
	}

   printf("xxx osdialog_file: 5\n");

	if (action == OSDIALOG_SAVE)
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (path)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);

   printf("xxx osdialog_file: 6\n");

	if (action == OSDIALOG_SAVE && filename)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);

   printf("xxx osdialog_file: 7\n");

	gtk_widget_show(dialog);  // [bsp] 26Oct2018

   printf("xxx osdialog_file: 8\n");

	char *result = NULL;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		result = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}
   printf("xxx osdialog_file: 9\n");
	gtk_widget_destroy(dialog);

   printf("xxx osdialog_file: 10\n");

	while (gtk_events_pending())
		gtk_main_iteration();
   printf("xxx osdialog_file: 11\n");
	return result;
}


int osdialog_color_picker(osdialog_color *color, int opacity) {
	if (!color)
		return 0;
	assert(gtk_init_check(NULL, NULL));

	GtkWidget *dialog = gtk_color_selection_dialog_new("Color");
	GtkColorSelection *colorsel = GTK_COLOR_SELECTION(gtk_color_selection_dialog_get_color_selection(GTK_COLOR_SELECTION_DIALOG(dialog)));

	gtk_color_selection_set_has_opacity_control(colorsel, opacity);

	int result = 0;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		GdkColor c;
		gtk_color_selection_get_current_color(colorsel, &c);
		color->r = c.red >> 8;
		color->g = c.green >> 8;
		color->b = c.blue >> 8;
		color->a = gtk_color_selection_get_current_alpha(colorsel) >> 8;
		result = 1;
	}
	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();
	return result;
}
