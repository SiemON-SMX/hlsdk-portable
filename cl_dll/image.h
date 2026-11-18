#pragma once
#if !defined(IMAGE_H)
#define IMAGE_H

typedef unsigned int	ILenum;
typedef unsigned char	ILboolean;
typedef unsigned int	ILuint;
typedef int				ILint;
typedef unsigned char	ILubyte;

#define IL_IMAGE_WIDTH		0x0DE4
#define IL_IMAGE_HEIGHT		0x0DE5
#define IL_IMAGE_BPP		0x0DE8
#define IL_IMAGE_FORMAT		0x0DEA

void ilInit( void );
void ilBindImage( ILuint Image );
ILboolean ilLoadImage( const char *FileName );
ILubyte *ilGetData( void );
ILint ilGetInteger( ILenum Mode );

#endif
