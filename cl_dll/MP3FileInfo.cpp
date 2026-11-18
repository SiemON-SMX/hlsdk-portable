#include "hud.h"
#include "cl_util.h"
#include "MP3FileInfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *MP3_NewString( const char *src, int len )
{
	char *out;
	int i, j = 0;

	while( len > 0 && ( src[len - 1] == ' ' || src[len - 1] == '\0' ))
		len--;

	if( len <= 0 )
		return 0;

	out = new char[len + 1];

	for( i = 0; i < len; i++ )
	{
		if( src[i] )
			out[j++] = src[i];
	}

	out[j] = '\0';
	return out;
}

static void MP3_FreeString( char *&str )
{
	if( str )
		delete[] str;
	str = 0;
}

static int MP3_SyncSafe( const unsigned char *p )
{
	return ( p[0] << 21 ) | ( p[1] << 14 ) | ( p[2] << 7 ) | p[3];
}

static int MP3_BigEndian( const unsigned char *p )
{
	return ( p[0] << 24 ) | ( p[1] << 16 ) | ( p[2] << 8 ) | p[3];
}

static char *MP3_TextFrame( const unsigned char *data, int size )
{
	if( size <= 1 )
		return 0;

	return MP3_NewString( (const char *)data + 1, size - 1 );
}

bool MP3FileInfo::m_readTags( void )
{
	unsigned char *buf;
	int size = 0;

	buf = gEngfuncs.COM_LoadFile( szFilename, 5, &size );

	if( !buf )
		return false;

	nFileSize = size;

	if( size >= 128 && !memcmp( buf + size - 128, "TAG", 3 ))
	{
		const char *tag = (const char *)buf + size - 128;
		char year[5];

		bHasV1Tag = true;
		szTitle = MP3_NewString( tag + 3, 30 );
		szArtist = MP3_NewString( tag + 33, 30 );
		szAlbum = MP3_NewString( tag + 63, 30 );
		memcpy( year, tag + 93, 4 );
		year[4] = '\0';
		szYear = MP3_NewString( year, 4 );
		nYear = atoi( year );
		szComment = MP3_NewString( tag + 97, 28 );

		if( tag[125] == 0 && tag[126] != 0 )
		{
			char track[8];
			nTrack = (unsigned char)tag[126];
			sprintf( track, "%d", nTrack );
			szTrack = MP3_NewString( track, strlen( track ));
		}
	}

	if( size >= 10 && !memcmp( buf, "ID3", 3 ))
	{
		int version = buf[3];
		int tagsize = MP3_SyncSafe( buf + 6 ) + 10;
		int pos = 10;

		bHasV2Tag = true;

		if( tagsize > size )
			tagsize = size;

		while( version >= 3 && pos + 10 <= tagsize )
		{
			const char *id = (const char *)buf + pos;
			int framesize = ( version == 4 ) ? MP3_SyncSafe( buf + pos + 4 ) : MP3_BigEndian( buf + pos + 4 );
			const unsigned char *data = buf + pos + 10;
			char **target = 0;

			if( id[0] == '\0' || framesize <= 0 || pos + 10 + framesize > tagsize )
				break;

			if( !strncmp( id, "TIT2", 4 )) target = &szTitle;
			else if( !strncmp( id, "TPE1", 4 )) target = &szArtist;
			else if( !strncmp( id, "TALB", 4 )) target = &szAlbum;
			else if( !strncmp( id, "TCOM", 4 )) target = &szComposer;
			else if( !strncmp( id, "TCOP", 4 )) target = &szCopyright;
			else if( !strncmp( id, "TENC", 4 )) target = &szEncodedBy;
			else if( !strncmp( id, "TOPE", 4 )) target = &szOriginalArtist;
			else if( !strncmp( id, "TCON", 4 )) target = &szGenre;

			if( target )
			{
				char *str = MP3_TextFrame( data, framesize );

				if( str )
				{
					MP3_FreeString( *target );
					*target = str;
				}
			}

			pos += 10 + framesize;
		}
	}

	gEngfuncs.COM_FreeFile( buf );
	return true;
}

bool MP3FileInfo::Init( const char *cszFilename )
{
	Free();
	m_bOK = true;

	szFilename = new char[strlen( cszFilename ) + 1];
	strcpy( szFilename, cszFilename );

	if( !m_readTags())
	{
		m_bOK = false;
		Free();
		return false;
	}

	return true;
}

char *MP3FileInfo::GetTitle( const char *cszFilename )
{
	if( !Init( cszFilename ))
		return 0;

	return szTitle;
}

void MP3FileInfo::Free( void )
{
	MP3_FreeString( szFilename );
	MP3_FreeString( szBitRate );
	MP3_FreeString( szMpegLayer );
	MP3_FreeString( szMpegVersion );
	MP3_FreeString( szChannelMode );
	MP3_FreeString( szArtist );
	MP3_FreeString( szTitle );
	MP3_FreeString( szAlbum );
	MP3_FreeString( szComment );
	MP3_FreeString( szTrack );
	MP3_FreeString( szYear );
	MP3_FreeString( szGenre );
	MP3_FreeString( szComposer );
	MP3_FreeString( szCopyright );
	MP3_FreeString( szEncodedBy );
	MP3_FreeString( szOriginalArtist );
	MP3_FreeString( szURL );

	bHasLyrics = bHasV1Tag = bHasV2Tag = false;
	nFileSize = -1;
	nCbrBitRate = nVbrBitRate = nBitRate = nSampleRate = nLength = 0;
	nTrack = nYear = 0;
}

void MP3FileInfo::ShowAllFrames( void )
{
	if( szTitle )
		gEngfuncs.Con_Printf( "Title: %s\n", szTitle );
	if( szArtist )
		gEngfuncs.Con_Printf( "Artist: %s\n", szArtist );
	if( szAlbum )
		gEngfuncs.Con_Printf( "Album: %s\n", szAlbum );
}
