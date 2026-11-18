#pragma once
#if !defined(CLIENT_PORT_H)
#define CLIENT_PORT_H

#include "../dlls/port.h"
#include "../dlls/keys.h"

int ClientKeyState( int vk );
void ClientKeyEvent( int down, int keynum );

#if defined(GetAsyncKeyState)
#undef GetAsyncKeyState
#endif
#define GetAsyncKeyState ClientKeyState

void InitClientKeys( void );


#endif
