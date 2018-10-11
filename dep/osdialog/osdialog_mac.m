#include "osdialog.h"
#include <AppKit/AppKit.h>
#include <Availability.h>


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message) {
	NSAlert *alert = [[NSAlert alloc] init];

	switch (level) {
		default:
#ifdef __MAC_10_12
		case OSDIALOG_INFO: [alert setAlertStyle:NSAlertStyleInformational]; break;
		case OSDIALOG_WARNING: [alert setAlertStyle:NSAlertStyleWarning]; break;
		case OSDIALOG_ERROR: [alert setAlertStyle:NSAlertStyleCritical]; break;
#else
		case OSDIALOG_INFO: [alert setAlertStyle:NSInformationalAlertStyle]; break;
		case OSDIALOG_WARNING: [alert setAlertStyle:NSWarningAlertStyle]; break;
		case OSDIALOG_ERROR: [alert setAlertStyle:NSCriticalAlertStyle]; break;
#endif
	}

	switch (buttons) {
		default:
		case OSDIALOG_OK:
			[alert addButtonWithTitle:@"OK"];
			break;
		case OSDIALOG_OK_CANCEL:
			[alert addButtonWithTitle:@"OK"];
			[alert addButtonWithTitle:@"Cancel"];
			break;
		case OSDIALOG_YES_NO:
			[alert addButtonWithTitle:@"Yes"];
			[alert addButtonWithTitle:@"No"];
			break;
	}

	NSString *messageString = [NSString stringWithUTF8String:message];
	// [alert setInformativeText:messageString];
	[alert setMessageText:messageString];

	int result;
	if ([alert runModal] == NSAlertFirstButtonReturn) {
		result = 1;
	}
	else {
		result = 0;
	}

	[alert release];
	return result;
}


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, osdialog_filters *filters) {
	NSSavePanel *panel;
	NSOpenPanel *open_panel;

	// No idea how to manage memory with Objective C. Please help!
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (action == OSDIALOG_OPEN || action == OSDIALOG_OPEN_DIR) {
		open_panel = [NSOpenPanel openPanel];
		panel = open_panel;
	}
	else {
		panel = [NSSavePanel savePanel];
	}

	// Bring dialog to front
	// https://stackoverflow.com/a/2402069
	// Thanks Dave!
	[panel setLevel:CGShieldingWindowLevel()];

	if (filters) {
		NSMutableArray *fileTypes = [[NSMutableArray alloc] init];
		
		for (; filters; filters = filters->next) {
			for (osdialog_filter_patterns *patterns = filters->patterns; patterns; patterns = patterns->next) {
				NSString *fileType = [NSString stringWithUTF8String:patterns->pattern];
				[fileTypes addObject:fileType];
			}
		}

		[panel setAllowedFileTypes:fileTypes];
		// [fileTypes release];
	}

	if (action == OSDIALOG_OPEN || action == OSDIALOG_OPEN_DIR) {
		open_panel.allowsMultipleSelection = NO;
	}
	if (action == OSDIALOG_OPEN) {
		open_panel.canChooseDirectories = NO;
		open_panel.canChooseFiles = YES;
	}
	if (action == OSDIALOG_OPEN_DIR) {
		open_panel.canCreateDirectories = YES;
		open_panel.canChooseDirectories = YES;
		open_panel.canChooseFiles = NO;
	}

	if (path) {
		NSString *path_str = [NSString stringWithUTF8String:path];
		NSURL *path_url = [NSURL fileURLWithPath:path_str];
		panel.directoryURL = path_url;
		// [path_url release];
		// [path_str release];
	}

	if (filename) {
		NSString *filenameString = [NSString stringWithUTF8String:filename];
		panel.nameFieldStringValue = filenameString;
		// [filenameString release];
	}

	char *result = NULL;

#ifdef __MAC_10_9
	#define OK NSModalResponseOK
#else
	#define OK NSOKButton
#endif
	if ([panel runModal] == OK) {
		NSURL *result_url = [panel URL];
		result = strdup([[result_url path] UTF8String]);
		// [result_url release];
	}

	// [panel release];
	[pool release];
	return result;
}


int osdialog_color_picker(osdialog_color *color, int opacity) {
	assert(0);

	// TODO I have no idea what I'm doing here
	NSColorPanel *panel = [NSColorPanel sharedColorPanel];
	// [panel setDelegate:self];
	[panel isVisible];

	// if (opacity)
	// 	[panel setShowAlpha:YES];
	// else
	// 	[panel setShowAlpha:NO];

	// [panel makeKeyAndOrderFront:self];

	return 0;
}
