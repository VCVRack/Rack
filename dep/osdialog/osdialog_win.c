#include "osdialog.h"
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <shlobj.h>


int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message) {
	UINT type = 0;
	switch (level) {
		default:
		case OSDIALOG_INFO: type |= MB_ICONINFORMATION; break;
		case OSDIALOG_WARNING: type |= MB_ICONWARNING; break;
		case OSDIALOG_ERROR: type |= MB_ICONERROR; break;
	}

	switch (buttons) {
		default:
		case OSDIALOG_OK: type |= MB_OK; break;
		case OSDIALOG_OK_CANCEL: type |= MB_OKCANCEL; break;
		case OSDIALOG_YES_NO: type |= MB_YESNO; break;
	}

	int result = MessageBox(NULL, message, "", type);
	switch (result) {
		case IDOK:
		case IDYES:
			return 1;
		default:
			return 0;
	}
}

char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, osdialog_filters *filters) {
	if (action == OSDIALOG_OPEN_DIR) {
		// open directory dialog
		TCHAR szDir[MAX_PATH] = "";

		BROWSEINFO bInfo;
		ZeroMemory(&bInfo, sizeof(bInfo));
		bInfo.hwndOwner = NULL;
		bInfo.pidlRoot = NULL; 
		bInfo.pszDisplayName = szDir;
		bInfo.lpszTitle = NULL;
		bInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
		bInfo.lpfn = NULL;
		bInfo.lParam = 0;
		bInfo.iImage = -1;

		LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
		if (lpItem) {
		  SHGetPathFromIDList(lpItem, szDir);
			return strdup(szDir);
		}
		else {
			return NULL;
		}
	}
	else {
		// open or save file dialog
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));

		char strFile[_MAX_PATH] = "";
		if (filename)
			snprintf(strFile, sizeof(strFile), "%s", filename);
		char *strInitialDir = path ? strdup(path) : NULL;

		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = strFile;
		ofn.nMaxFile = sizeof(strFile);
		ofn.lpstrInitialDir = strInitialDir;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (filters) {
			char fBuf[4096];
			int fLen = 0;

			for (; filters; filters = filters->next) {
				fLen += snprintf(fBuf + fLen, sizeof(fBuf) - fLen, "%s", filters->name);
				fLen++;
				for (osdialog_filter_patterns *patterns = filters->patterns; patterns; patterns = patterns->next) {
					fLen += snprintf(fBuf + fLen, sizeof(fBuf) - fLen, "*.%s", patterns->pattern);
					if (patterns->next)
						fLen += snprintf(fBuf + fLen, sizeof(fBuf) - fLen, ";");
				}
				fLen++;
			}

			ofn.lpstrFilter = fBuf;
			ofn.nFilterIndex = 1;
		}

		BOOL success;
		if (action == OSDIALOG_OPEN)
			success = GetOpenFileName(&ofn);
		else
			success = GetSaveFileName(&ofn);

		if (strInitialDir)
			free(strInitialDir);
		return success ? strdup(strFile) : NULL;
	}
}


int osdialog_color_picker(osdialog_color *color, int opacity) {
	if (!color)
		return 0;

	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(cc));

	COLORREF c = RGB(color->r, color->g, color->b);
	static COLORREF acrCustClr[16];

	cc.lStructSize = sizeof(cc);
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = c;
	cc.Flags = CC_FULLOPEN | CC_ANYCOLOR | CC_RGBINIT;

	if (ChooseColor(&cc)) {
		color->r = GetRValue(cc.rgbResult);
		color->g = GetGValue(cc.rgbResult);
		color->b = GetBValue(cc.rgbResult);
		color->a = 255;
		return 1;
	}

	return 0;
}