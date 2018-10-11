#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>


typedef enum {
	OSDIALOG_INFO,
	OSDIALOG_WARNING,
	OSDIALOG_ERROR,
} osdialog_message_level;

typedef enum {
	OSDIALOG_OK,
	OSDIALOG_OK_CANCEL,
	OSDIALOG_YES_NO,
} osdialog_message_buttons;

/** Launches a message box
Returns 1 if the "OK" or "Yes" button was pressed
*/
int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message);


typedef enum {
	OSDIALOG_OPEN,
	OSDIALOG_OPEN_DIR,
	OSDIALOG_SAVE,
} osdialog_file_action;

/** Linked list of patterns */
typedef struct osdialog_filter_patterns {
	char *pattern;
	struct osdialog_filter_patterns *next;
} osdialog_filter_patterns;

/** Linked list of file filters */
typedef struct osdialog_filters {
	char *name;
	osdialog_filter_patterns *patterns;
	struct osdialog_filters *next;
} osdialog_filters;

/** Launches a file dialog and returns the selected path or NULL if nothing was selected
If the return result is not NULL, caller must free() it

`path` is the default folder the file dialog will attempt to open in.
`filename` is the default text that will appear in the filename input. Relevant to save dialog only.
`filters` is a list of patterns to filter the file selection, or NULL
*/
char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, osdialog_filters *filters);

/** Parses a filter string of the form
	Source:c,cpp,m;Header:h,hpp
Caller must eventually free with osdialog_filters_free()
*/
osdialog_filters *osdialog_filters_parse(const char *str);
void osdialog_filters_free(osdialog_filters *filters);


typedef struct {
	uint8_t r, g, b, a;
} osdialog_color;

/** Launches an RGBA color picker dialog and sets `color` to the selected color
Returns 1 if "OK" was pressed

`color` should be set to the initial color before calling. It is only overwritten if the user selects "OK".
`opacity` enables the opacity slider by setting to 1. Not supported on Windows.

TODO Implement on Mac
*/
int osdialog_color_picker(osdialog_color *color, int opacity);


#ifdef __cplusplus
}
#endif
