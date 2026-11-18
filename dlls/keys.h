#pragma once
#if !defined(KEYS_H)
#define KEYS_H

#define VKEY_TAB		0x09
#define VKEY_RETURN	0x0D
#define VKEY_ESCAPE	0x1B
#define VKEY_NUMPAD0	0x60
#define VKEY_NUMPAD9	0x69

#if !defined(_WIN32)
#define VK_TAB		VKEY_TAB
#define VK_RETURN	VKEY_RETURN
#define VK_ESCAPE	VKEY_ESCAPE
#define VK_NUMPAD0	0x60
#define VK_NUMPAD1	0x61
#define VK_NUMPAD2	0x62
#define VK_NUMPAD3	0x63
#define VK_NUMPAD4	0x64
#define VK_NUMPAD5	0x65
#define VK_NUMPAD6	0x66
#define VK_NUMPAD7	0x67
#define VK_NUMPAD8	0x68
#define VK_NUMPAD9	0x69
#endif

int KeyState( int vk );
void KeyCommand( int vk, int down );
void KeyTap( int vk );

#endif
