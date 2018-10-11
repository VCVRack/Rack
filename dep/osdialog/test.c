#include <stdlib.h>
#include <stdio.h>
#include "osdialog.h"


int main() {
	// Message
	if (0) {
		int res;
		printf("message info\n");
		res = osdialog_message(OSDIALOG_INFO, OSDIALOG_OK, "Info");
		printf("\t%d\n", res);
		printf("message warning\n");
		res = osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK_CANCEL, "Warning");
		printf("\t%d\n", res);
		printf("message error\n");
		res = osdialog_message(OSDIALOG_ERROR, OSDIALOG_YES_NO, "Error");
		printf("\t%d\n", res);
	}

	// Open file with default arguments
	if (0) {
		printf("file open\n");
		char *filename = osdialog_file(OSDIALOG_OPEN, NULL, NULL, NULL);
		if (filename) {
			printf("\t%s\n", filename);
			free(filename);
		}
		else {
			printf("\tCanceled\n");
		}
	}

	// Open directory with default arguments
	if (0) {
		printf("file open dir\n");
		char *filename = osdialog_file(OSDIALOG_OPEN_DIR, NULL, NULL, NULL);
		if (filename) {
			printf("\t%s\n", filename);
			free(filename);
		}
		else {
			printf("\tCanceled\n");
		}
	}

	// Save file with default arguments
	if (0) {
		printf("file save\n");
		char *filename = osdialog_file(OSDIALOG_SAVE, NULL, NULL, NULL);
		if (filename) {
			printf("\t%s\n", filename);
			free(filename);
		}
		else {
			printf("\tCanceled\n");
		}
	}

	// Open file with custom arguments
	if (1) {
		printf("file open in cwd\n");
		osdialog_filters *filters = osdialog_filters_parse("Source:c,cpp,m;Header:h,hpp");
		char *filename = osdialog_file(OSDIALOG_OPEN, ".", "test", filters);
		if (filename) {
			printf("\t%s\n", filename);
			free(filename);
		}
		else {
			printf("\tCanceled\n");
		}
		osdialog_filters_free(filters);
	}

	// Open directory with custom arguments
	if (0) {
		printf("file open dir in cwd\n");
		char *filename = osdialog_file(OSDIALOG_OPEN_DIR, ".", "test", NULL);
		if (filename) {
			printf("\t%s\n", filename);
			free(filename);
		}
		else {
			printf("\tCanceled\n");
		}
	}

	// Save file with custom arguments
	if (0) {
		printf("file save in cwd\n");
		char *filename = osdialog_file(OSDIALOG_SAVE, ".", "test", NULL);
		if (filename) {
			printf("\t%s\n", filename);
			free(filename);
		}
		else {
			printf("\tCanceled\n");
		}
	}

	// Color selector
	if (0) {
		int res;
		osdialog_color color = {255, 0, 255, 255};
		printf("color picker\n");
		res = osdialog_color_picker(&color, 0);
		printf("\t%d\n", res);
		printf("\t#%02x%02x%02x%02x\n", color.r, color.g, color.b, color.a);
		printf("color picker with opacity\n");
		res = osdialog_color_picker(&color, 1);
		printf("\t%d\n", res);
		printf("\t#%02x%02x%02x%02x\n", color.r, color.g, color.b, color.a);
	}
}