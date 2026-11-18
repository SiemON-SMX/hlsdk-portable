#include "hud.h"
#include "cl_util.h"
#include "glmanager.h"
#include "image.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct cof_ilimage_s
{
	int width;
	int height;
	unsigned char *data;
} cof_ilimage_t;

static cof_ilimage_t g_Image;

static void FreeImage( void )
{
	if( g_Image.data )
		free( g_Image.data );

	g_Image.data = NULL;
	g_Image.width = 0;
	g_Image.height = 0;
}

static const char *RelativePath( const char *path )
{
	const char *gamedir = gEngfuncs.pfnGetGameDirectory();
	size_t len;

	if( !gamedir || !*gamedir )
		return path;

	len = strlen( gamedir );

	if( !strncasecmp( path, gamedir, len ) && ( path[len] == '/' || path[len] == '\\' ) )
		return path + len + 1;

	return path;
}

static const char *Extension( const char *path )
{
	const char *dot = strrchr( path, '.' );
	return dot ? dot + 1 : "";
}

static int DecodeTGA( const unsigned char *buf, int size )
{
	int idlen, cmaptype, type, width, height, bpp, desc;
	int pixels, i;
	const unsigned char *src, *end;
	unsigned char *out;

	if( size < 18 )
		return 0;

	idlen = buf[0];
	cmaptype = buf[1];
	type = buf[2];
	width = buf[12] | ( buf[13] << 8 );
	height = buf[14] | ( buf[15] << 8 );
	bpp = buf[16];
	desc = buf[17];

	if( cmaptype != 0 || ( type != 2 && type != 3 && type != 10 && type != 11 ))
		return 0;

	if( bpp != 8 && bpp != 24 && bpp != 32 )
		return 0;

	if( width <= 0 || height <= 0 )
		return 0;

	pixels = width * height;
	out = (unsigned char *)malloc( pixels * 4 );
	src = buf + 18 + idlen;
	end = buf + size;

	memset( out, 0, pixels * 4 );

	for( i = 0; i < pixels; )
	{
		int count = 1, raw = 1;

		if( type == 10 || type == 11 )
		{
			if( src >= end )
				break;

			count = ( *src & 0x7F ) + 1;
			raw = !( *src & 0x80 );
			src++;
		}

		for( int j = 0; j < count && i < pixels; j++, i++ )
		{
			unsigned char r, g, b, a = 255;

			if( src + ( bpp >> 3 ) > end )
			{
				i = pixels;
				break;
			}

			if( bpp == 8 )
			{
				r = g = b = src[0];
			}
			else
			{
				b = src[0];
				g = src[1];
				r = src[2];

				if( bpp == 32 )
					a = src[3];
			}

			out[i * 4 + 0] = r;
			out[i * 4 + 1] = g;
			out[i * 4 + 2] = b;
			out[i * 4 + 3] = a;

			if( raw || j == count - 1 )
				src += bpp >> 3;
		}
	}

	if( !( desc & 0x20 ))
	{
		int stride = width * 4;
		unsigned char *row = (unsigned char *)malloc( stride );

		for( int y = 0; y < height / 2; y++ )
		{
			memcpy( row, out + y * stride, stride );
			memcpy( out + y * stride, out + ( height - 1 - y ) * stride, stride );
			memcpy( out + ( height - 1 - y ) * stride, row, stride );
		}

		free( row );
	}

	g_Image.width = width;
	g_Image.height = height;
	g_Image.data = out;
	return 1;
}

static int DecodeBMP( const unsigned char *buf, int size )
{
	int offset, width, height, bpp, compression, colors;
	int stride, flip = 1;
	const unsigned char *palette;
	unsigned char *out;

	if( size < 54 || buf[0] != 'B' || buf[1] != 'M' )
		return 0;

	offset = buf[10] | ( buf[11] << 8 ) | ( buf[12] << 16 ) | ( buf[13] << 24 );
	width = buf[18] | ( buf[19] << 8 ) | ( buf[20] << 16 ) | ( buf[21] << 24 );
	height = buf[22] | ( buf[23] << 8 ) | ( buf[24] << 16 ) | ( buf[25] << 24 );
	bpp = buf[28] | ( buf[29] << 8 );
	compression = buf[30] | ( buf[31] << 8 ) | ( buf[32] << 16 ) | ( buf[33] << 24 );
	colors = buf[46] | ( buf[47] << 8 ) | ( buf[48] << 16 ) | ( buf[49] << 24 );

	if( compression != 0 || ( bpp != 8 && bpp != 24 && bpp != 32 ))
		return 0;

	if( height < 0 )
	{
		height = -height;
		flip = 0;
	}

	if( width <= 0 || height <= 0 )
		return 0;

	if( !colors )
		colors = 256;

	palette = buf + 14 + ( buf[14] | ( buf[15] << 8 ) | ( buf[16] << 16 ) | ( buf[17] << 24 ));
	stride = (( width * bpp + 31 ) / 32 ) * 4;

	if( offset + stride * height > size )
		return 0;

	out = (unsigned char *)malloc( width * height * 4 );

	for( int y = 0; y < height; y++ )
	{
		const unsigned char *row = buf + offset + stride * ( flip ? ( height - 1 - y ) : y );

		for( int x = 0; x < width; x++ )
		{
			unsigned char *dst = out + ( y * width + x ) * 4;

			if( bpp == 8 )
			{
				int idx = row[x];

				if( idx >= colors || palette + idx * 4 + 3 > buf + size )
					idx = 0;

				dst[0] = palette[idx * 4 + 2];
				dst[1] = palette[idx * 4 + 1];
				dst[2] = palette[idx * 4 + 0];
				dst[3] = 255;
			}
			else
			{
				const unsigned char *p = row + x * ( bpp >> 3 );
				dst[0] = p[2];
				dst[1] = p[1];
				dst[2] = p[0];
				dst[3] = ( bpp == 32 ) ? p[3] : 255;
			}
		}
	}

	g_Image.width = width;
	g_Image.height = height;
	g_Image.data = out;
	return 1;
}

static int DecodeWithEngine( const char *relpath, const unsigned char *buf, int size )
{
	int texnum, glnum, width = 0, height = 0;

	if( !g_pXashRenderAPI || !g_pXashRenderAPI->GL_LoadTexture || !g_pXashRenderAPI->RenderGetParm )
		return 0;

	if( !gl.glGetTexImage || !gl.glGetTexLevelParameteriv || !gl.glBindTexture )
		return 0;

	texnum = g_pXashRenderAPI->GL_LoadTexture( relpath, buf, size, ( 1 << 12 ));

	if( !texnum )
		return 0;

	glnum = (int)g_pXashRenderAPI->RenderGetParm( PARM_TEX_TEXNUM, texnum );

	gl.glBindTexture( GL_TEXTURE_2D, glnum );
	gl.glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
	gl.glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );

	if( width > 0 && height > 0 )
	{
		g_Image.data = (unsigned char *)malloc( width * height * 4 );
		g_Image.width = width;
		g_Image.height = height;
		gl.glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, g_Image.data );
	}

	if( g_pXashRenderAPI->GL_FreeTexture )
		g_pXashRenderAPI->GL_FreeTexture( texnum );

	GL_ResetEngineState();

	return g_Image.data != NULL;
}

void ilInit( void )
{
}

void ilBindImage( ILuint Image )
{
}

ILboolean ilLoadImage( const char *FileName )
{
	const char *relpath, *ext;
	unsigned char *buf;
	int size = 0, ok = 0;

	FreeImage();

	if( !FileName || !*FileName )
		return 0;

	relpath = RelativePath( FileName );
	buf = gEngfuncs.COM_LoadFile( (char *)relpath, 5, &size );

	if( !buf )
		return 0;

	ext = Extension( relpath );

	if( !strcasecmp( ext, "tga" ))
		ok = DecodeTGA( buf, size );
	else if( !strcasecmp( ext, "bmp" ))
		ok = DecodeBMP( buf, size );

	if( !ok )
		ok = DecodeWithEngine( relpath, buf, size );

	gEngfuncs.COM_FreeFile( buf );

	return ok ? 1 : 0;
}

ILubyte *ilGetData( void )
{
	return g_Image.data;
}

ILint ilGetInteger( ILenum Mode )
{
	switch( Mode )
	{
	case IL_IMAGE_WIDTH:
		return g_Image.width;
	case IL_IMAGE_HEIGHT:
		return g_Image.height;
	case IL_IMAGE_BPP:
		return 4;
	case IL_IMAGE_FORMAT:
		return GL_RGBA;
	}

	return 0;
}
