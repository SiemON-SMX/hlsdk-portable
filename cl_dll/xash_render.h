#pragma once
#if !defined(XASH_RENDER_H)
#define XASH_RENDER_H

#include <stdint.h>
#include <stddef.h>

#define RENDER_INTERFACE_VERSION 37

#define PARM_TEX_WIDTH	1
#define PARM_TEX_HEIGHT	2
#define PARM_TEX_TEXNUM	9

typedef struct cof_render_api_s
{
	intptr_t (*RenderGetParm)( int parm, int arg );
	void *pfnUnused1[14];
	int (*GL_LoadTexture)( const char *name, const unsigned char *buf, size_t size, int flags );
	void *pfnUnused2[3];
	void (*GL_FreeTexture)( unsigned int texnum );
	void *pfnUnused3[13];
	void (*GL_Bind)( int tmu, unsigned int texnum );
	void (*GL_SelectTexture)( int tmu );
	void (*GL_LoadTextureMatrix)( const float *glmatrix );
	void (*GL_TexMatrixIdentity)( void );
	void (*GL_CleanUpTextureUnits)( int last );
	void (*GL_TexGen)( unsigned int coord, unsigned int mode );
	void (*GL_TextureTarget)( unsigned int target );
	void (*GL_TexCoordArrayMode)( unsigned int texmode );
	void *(*GL_GetProcAddress)( const char *name );
	void (*GL_UpdateTexSize)( int texnum, int width, int height, int depth );
	void (*GL_ResyncClientState)( void );
	void *pfnUnused4[19];
	void (*S_FadeMusicVolume)( float fadePercent );
} cof_render_api_t;

extern cof_render_api_t *g_pXashRenderAPI;

static inline unsigned int EngineTexNum( int handle )
{
	if( g_pXashRenderAPI && g_pXashRenderAPI->RenderGetParm )
	{
		intptr_t texnum = g_pXashRenderAPI->RenderGetParm( PARM_TEX_TEXNUM, handle );

		if( texnum )
			return (unsigned int)texnum;
	}

	return (unsigned int)( handle & 0xFFFF );
}

void *GL_GetEngineProcAddress( const char *name );
void GL_ResetEngineState( void );

#endif
