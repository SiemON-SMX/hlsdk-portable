
#include "client_port.h"
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hud.h"
#include "cl_util.h"
#include "tgaloader.h"

bool CTGALoader::LoadTGA( Texture *texture, char *filename )
{
	FILE *fTGA;
	fTGA = fopen( filename, "rb" );

	if ( fTGA == NULL )
	{
		gEngfuncs.Con_Printf( "TGA LOAD: Could not open texture file\n" );
		return false;
	}

	if ( fread( &tgaheader, sizeof( TGAHeader ), 1, fTGA ) == 0 )
	{
		gEngfuncs.Con_Printf( "TGA LOAD: Could not read file header\n" );
		if ( fTGA != NULL )
			fclose( fTGA );
		return false;
	}

	if ( memcmp( uTGAcompare, &tgaheader, sizeof( tgaheader ) ) == 0 )
	{
		LoadUncompressedTGA( texture, filename, fTGA );
	}
	else if ( memcmp( cTGAcompare, &tgaheader, sizeof( tgaheader ) ) == 0 )
	{
		LoadCompressedTGA( texture, filename, fTGA );
	}
	else
	{
		gEngfuncs.Con_Printf( "TGA LOAD: TGA file must be type 2 or type 10\n" );
		fclose( fTGA );
		return false;
	}

	return true;
}

bool CTGALoader::LoadUncompressedTGA( Texture *texture, char *filename, FILE *fTGA )
{
	if ( fread( tga.header, sizeof( tga.header ), 1, fTGA ) == 0 )
	{
		gEngfuncs.Con_Printf( "TGA LOAD: Could not read info header\n" );
		if ( fTGA != NULL )
			fclose( fTGA );
		return false;
	}

	texture->width	= tga.header[1] * 256 + tga.header[0];
	texture->height	= tga.header[3] * 256 + tga.header[2];
	texture->bpp		= tga.header[4];
	tga.Width	= texture->width;
	tga.Height	= texture->height;
	tga.Bpp		= texture->bpp;

	if ( ( texture->width <= 0 ) || ( texture->height <= 0 ) || ( ( texture->bpp != 24 ) && ( texture->bpp != 32 ) ) )
	{
		gEngfuncs.Con_Printf( "TGA LOAD: Invalid texture information\n" );
		if ( fTGA != NULL )
			fclose( fTGA );
		return false;
	}

	if ( texture->bpp == 24 )
		texture->type = GL_RGB;
	else
		texture->type = GL_RGBA;

	tga.bytesPerPixel	= ( tga.Bpp / 8 );
	tga.imageSize		= ( tga.bytesPerPixel * tga.Width * tga.Height );
	texture->imageData		= (GLubyte *)malloc( tga.imageSize );

	if ( texture->imageData == NULL )
	{
		gEngfuncs.Con_Printf( "TGA LOAD: Could not allocate memory for image\n" );
		fclose( fTGA );
		return false;
	}

	if ( fread( texture->imageData, 1, tga.imageSize, fTGA ) != tga.imageSize )
	{
		gEngfuncs.Con_Printf( "TGA LOAD: Could not read image data\n" );
		if ( texture->imageData != NULL )
			free( texture->imageData );
		fclose( fTGA );
		return false;
	}

	for ( GLuint cswap = 0; cswap < (int)tga.imageSize; cswap += tga.bytesPerPixel )
	{
		texture->imageData[cswap] ^= texture->imageData[cswap + 2] ^=
		texture->imageData[cswap] ^= texture->imageData[cswap + 2];
	}

	fclose( fTGA );
	return true;
}

bool CTGALoader::LoadCompressedTGA( Texture *texture, char *filename, FILE *fTGA )
{
	if ( fread( tga.header, sizeof( tga.header ), 1, fTGA ) == 0 )
	{
		gEngfuncs.Con_Printf( "TGA LOAD: Could not read info header\n" );
		if ( fTGA != NULL )
			fclose( fTGA );
		return false;
	}

	texture->width	= tga.header[1] * 256 + tga.header[0];
	texture->height	= tga.header[3] * 256 + tga.header[2];
	texture->bpp		= tga.header[4];
	tga.Width	= texture->width;
	tga.Height	= texture->height;
	tga.Bpp		= texture->bpp;

	if ( ( texture->width <= 0 ) || ( texture->height <= 0 ) || ( ( texture->bpp != 24 ) && ( texture->bpp != 32 ) ) )
	{
		gEngfuncs.Con_Printf( "TGA LOAD: Invalid texture information\n" );
		if ( fTGA != NULL )
			fclose( fTGA );
		return false;
	}

	if ( texture->bpp == 24 )
		texture->type = GL_RGB;
	else
		texture->type = GL_RGBA;

	tga.bytesPerPixel	= ( tga.Bpp / 8 );
	tga.imageSize		= ( tga.bytesPerPixel * tga.Width * tga.Height );
	texture->imageData		= (GLubyte *)malloc( tga.imageSize );

	if ( texture->imageData == NULL )
	{
		gEngfuncs.Con_Printf( "TGA LOAD: Could not allocate memory for image\n" );
		fclose( fTGA );
		return false;
	}

	GLuint pixelcount		= tga.Height * tga.Width;
	GLuint currentpixel		= 0;
	GLuint currentbyte		= 0;
	GLubyte *colorbuffer	= (GLubyte *)malloc( tga.bytesPerPixel );

	do
	{
		GLubyte chunkheader = 0;

		if ( fread( &chunkheader, sizeof( GLubyte ), 1, fTGA ) == 0 )
		{
			gEngfuncs.Con_Printf( "TGA LOAD: Could not read RLE header\n" );
			if ( fTGA != NULL )
				fclose( fTGA );
			if ( texture->imageData != NULL )
				free( texture->imageData );
			return false;
		}

		if ( chunkheader < 128 )
		{
			short counter;
			chunkheader++;
			for ( counter = 0; counter < chunkheader; counter++ )
			{
				if ( fread( colorbuffer, 1, tga.bytesPerPixel, fTGA ) != tga.bytesPerPixel )
				{
					gEngfuncs.Con_Printf( "TGA LOAD: Could not read image data\n" );
					if ( fTGA != NULL )
						fclose( fTGA );
					if ( colorbuffer != NULL )
						free( colorbuffer );
					if ( texture->imageData != NULL )
						free( texture->imageData );
					return false;
				}

				texture->imageData[currentbyte] = colorbuffer[2];
				texture->imageData[currentbyte + 1] = colorbuffer[1];
				texture->imageData[currentbyte + 2] = colorbuffer[0];

				if ( tga.bytesPerPixel == 4 )
					texture->imageData[currentbyte + 3] = colorbuffer[3];

				currentbyte += tga.bytesPerPixel;
				currentpixel++;

				if ( currentpixel > pixelcount )
				{
					gEngfuncs.Con_Printf( "TGA LOAD: Too many pixels read\n" );
					if ( fTGA != NULL )
						fclose( fTGA );
					if ( colorbuffer != NULL )
						free( colorbuffer );
					if ( texture->imageData != NULL )
						free( texture->imageData );
					return false;
				}
			}
		}
		else
		{
			short counter;
			chunkheader -= 127;
			if ( fread( colorbuffer, 1, tga.bytesPerPixel, fTGA ) != tga.bytesPerPixel )
			{
				gEngfuncs.Con_Printf( "TGA LOAD: Could not read from file\n" );
				if ( fTGA != NULL )
					fclose( fTGA );
				if ( colorbuffer != NULL )
					free( colorbuffer );
				if ( texture->imageData != NULL )
					free( texture->imageData );
				return false;
			}

			for ( counter = 0; counter < chunkheader; counter++ )
			{
				texture->imageData[currentbyte] = colorbuffer[2];
				texture->imageData[currentbyte + 1] = colorbuffer[1];
				texture->imageData[currentbyte + 2] = colorbuffer[0];

				if ( tga.bytesPerPixel == 4 )
					texture->imageData[currentbyte + 3] = colorbuffer[3];

				currentbyte += tga.bytesPerPixel;
				currentpixel++;

				if ( currentpixel > pixelcount )
				{
					gEngfuncs.Con_Printf( "TGA LOAD: Too many pixels read\n" );
					if ( fTGA != NULL )
						fclose( fTGA );
					if ( colorbuffer != NULL )
						free( colorbuffer );
					if ( texture->imageData != NULL )
						free( texture->imageData );
					return false;
				}
			}
		}
	}
	while ( currentpixel < pixelcount );

	fclose( fTGA );
	return true;
}
