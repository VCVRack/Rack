#include "osdialog.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>


static char *strndup_better(const char *s, int n) {
	char *d = malloc(n + 1);
	memcpy(d, s, n);
	d[n] = '\0';
	return d;
}

osdialog_filters *osdialog_filters_parse(const char *str) {
	osdialog_filters *filters_head = malloc(sizeof(osdialog_filters));
	filters_head->next = NULL;

	osdialog_filters *filters = filters_head;
	osdialog_filter_patterns *patterns = NULL;

	const char *text = str;
	while (1) {
		switch (*str) {
			case ':': {
				filters->name = strndup_better(text, str - text);
				filters->patterns = malloc(sizeof(osdialog_filter_patterns));
				patterns = filters->patterns;
				patterns->next = NULL;
				text = str + 1;
			} break;
			case ',': {
				assert(patterns);
				patterns->pattern = strndup_better(text, str - text);
				patterns->next = malloc(sizeof(osdialog_filter_patterns));
				patterns = patterns->next;
				patterns->next = NULL;
				text = str + 1;
			} break;
			case ';': {
				assert(patterns);
				patterns->pattern = strndup_better(text, str - text);
				filters->next = malloc(sizeof(osdialog_filters));
				filters = filters->next;
				filters->next = NULL;
				patterns = NULL;
				text = str + 1;
			} break;
			case '\0': {
				assert(patterns);
				patterns->pattern = strndup_better(text, str - text);
			} break;
			default: break;
		}
		if (!*str)
			break;
		str++;
	}

	return filters_head;
}

static void patterns_free(osdialog_filter_patterns *patterns) {
	if (!patterns)
		return;
	free(patterns->pattern);
	osdialog_filter_patterns *next = patterns->next;
	free(patterns);
	patterns_free(next);
}

void osdialog_filters_free(osdialog_filters *filters) {
	if (!filters)
		return;
	free(filters->name);
	patterns_free(filters->patterns);
	osdialog_filters *next = filters->next;
	free(filters);
	osdialog_filters_free(next);
}
