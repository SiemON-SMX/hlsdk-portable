#pragma once
#if !defined(PORT_H)
#define PORT_H

#if !defined(_WIN32)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <fnmatch.h>
#include <unistd.h>
#include <sys/stat.h>

#if !defined(MAX_PATH)
#define MAX_PATH 260
#endif

static inline char *itoa( int value, char *str, int radix )
{
	if( radix == 16 )
		sprintf( str, "%x", value );
	else if( radix == 8 )
		sprintf( str, "%o", value );
	else
		sprintf( str, "%d", value );
	return str;
}

#include <time.h>

static inline unsigned int GetTickCount( void )
{
	struct timespec ts;
	clock_gettime( CLOCK_MONOTONIC, &ts );
	return (unsigned int)( ts.tv_sec * 1000 + ts.tv_nsec / 1000000 );
}

#define sprintf_s snprintf
#define _getcwd getcwd
#define _access access
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define INVALID_FILE_ATTRIBUTES ( (unsigned int)-1 )
#define FILE_ATTRIBUTE_DIRECTORY 0x10

static inline unsigned int GetFileAttributes( const char *path )
{
	struct stat st;

	if( stat( path, &st ) != 0 )
		return INVALID_FILE_ATTRIBUTES;

	return S_ISDIR( st.st_mode ) ? FILE_ATTRIBUTE_DIRECTORY : 0x80;
}

static inline int CreateDirectory( const char *path, void *attributes )
{
	return mkdir( path, 0777 ) == 0;
}
#define _mkdir( p ) mkdir( p, 0777 )
#define DeleteFile( p ) ( unlink( p ) == 0 )

typedef struct cof_find_s
{
	DIR *dir;
	char pattern[MAX_PATH];
} *HANDLE;

typedef struct
{
	char cFileName[MAX_PATH];
} WIN32_FIND_DATA;

#define INVALID_HANDLE_VALUE ( (HANDLE)NULL )

static inline int FindMatch( HANDLE h, WIN32_FIND_DATA *data )
{
	struct dirent *ent;

	while( ( ent = readdir( h->dir ) ) != NULL )
	{
		if( !fnmatch( h->pattern, ent->d_name, FNM_CASEFOLD ) )
		{
			strncpy( data->cFileName, ent->d_name, MAX_PATH - 1 );
			data->cFileName[MAX_PATH - 1] = '\0';
			return 1;
		}
	}

	return 0;
}

static inline int FindClose( HANDLE h )
{
	if( !h )
		return 0;

	if( h->dir )
		closedir( h->dir );
	free( h );
	return 1;
}

static inline HANDLE FindFirstFile( const char *path, WIN32_FIND_DATA *data )
{
	char dir[MAX_PATH];
	const char *slash;
	HANDLE h;

	strncpy( dir, path, MAX_PATH - 1 );
	dir[MAX_PATH - 1] = '\0';

	for( char *p = dir; *p; p++ )
	{
		if( *p == '\\' )
			*p = '/';
	}

	slash = strrchr( dir, '/' );

	h = (HANDLE)calloc( 1, sizeof( *h ) );

	if( slash )
	{
		strncpy( h->pattern, slash + 1, MAX_PATH - 1 );
		dir[slash - dir + 1] = '\0';
	}
	else
	{
		strncpy( h->pattern, dir, MAX_PATH - 1 );
		strcpy( dir, "./" );
	}

	h->dir = opendir( dir );

	if( !h->dir || !FindMatch( h, data ) )
	{
		FindClose( h );
		return INVALID_HANDLE_VALUE;
	}

	return h;
}

static inline int FindNextFile( HANDLE h, WIN32_FIND_DATA *data )
{
	if( !h || !h->dir )
		return 0;

	if( FindMatch( h, data ) )
		return 1;

	FindClose( h );
	return 0;
}
#else
#include <direct.h>
#endif

#endif
