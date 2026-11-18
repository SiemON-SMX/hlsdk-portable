#pragma once
#if !defined(MSGBOX_H)
#define MSGBOX_H

#if !_WIN32
#define MB_OK		0x00000000
#define MB_YESNO	0x00000004
#define IDOK		1
#define IDYES		6
#define IDNO		7
int ShowMessageBox( void *hWnd, const char *pszText, const char *pszCaption, unsigned int uType );
#define MessageBox ShowMessageBox
#endif

#endif
