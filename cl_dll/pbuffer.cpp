
#include <string>
#include <list>

#include "hud.h"
#include "cl_util.h"
#include "glmanager.h"
#include "texturemanager.h"
#include "pbuffer.h"

//=========================================================
//=========================================================
#define WGL_DRAW_TO_PBUFFER_ARB			0x202D
#define WGL_PBUFFER_WIDTH_ARB			0x2034
#define WGL_PBUFFER_HEIGHT_ARB			0x2035
#define WGL_PBUFFER_LOST_ARB			0x2036
#define WGL_SUPPORT_OPENGL_ARB			0x2010
#define WGL_DOUBLE_BUFFER_ARB			0x2011
#define WGL_RED_BITS_ARB				0x2015
#define WGL_GREEN_BITS_ARB				0x2017
#define WGL_BLUE_BITS_ARB				0x2019
#define WGL_ALPHA_BITS_ARB				0x201B
#define WGL_DEPTH_BITS_ARB				0x2022
#define WGL_BIND_TO_TEXTURE_RGBA_ARB	0x2071
#define WGL_TEXTURE_FORMAT_ARB			0x2072
#define WGL_TEXTURE_TARGET_ARB			0x2073
#define WGL_TEXTURE_RGBA_ARB			0x2076
#define WGL_TEXTURE_2D_ARB				0x207A
#define WGL_FRONT_LEFT_ARB				0x2083

typedef const char *( WINAPI *PFNWGLGETEXTENSIONSSTRINGARBPROC )( HDC hdc );
typedef HPBUFFERARB ( WINAPI *PFNWGLCREATEPBUFFERARBPROC )( HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList );
typedef HDC ( WINAPI *PFNWGLGETPBUFFERDCARBPROC )( HPBUFFERARB hPbuffer );
typedef int ( WINAPI *PFNWGLRELEASEPBUFFERDCARBPROC )( HPBUFFERARB hPbuffer, HDC hDC );
typedef BOOL ( WINAPI *PFNWGLDESTROYPBUFFERARBPROC )( HPBUFFERARB hPbuffer );
typedef BOOL ( WINAPI *PFNWGLQUERYPBUFFERARBPROC )( HPBUFFERARB hPbuffer, int iAttribute, int *piValue );
typedef BOOL ( WINAPI *PFNWGLGETPIXELFORMATATTRIBIVARBPROC )( HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues );
typedef BOOL ( WINAPI *PFNWGLGETPIXELFORMATATTRIBFVARBPROC )( HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues );
typedef BOOL ( WINAPI *PFNWGLCHOOSEPIXELFORMATARBPROC )( HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats );
typedef BOOL ( WINAPI *PFNWGLBINDTEXIMAGEARBPROC )( HPBUFFERARB hPbuffer, int iBuffer );
typedef BOOL ( WINAPI *PFNWGLRELEASETEXIMAGEARBPROC )( HPBUFFERARB hPbuffer, int iBuffer );

PFNWGLGETEXTENSIONSSTRINGARBPROC	wglGetExtensionsStringARB = NULL;
PFNWGLCREATEPBUFFERARBPROC			wglCreatePbufferARB = NULL;
PFNWGLGETPBUFFERDCARBPROC			wglGetPbufferDCARB = NULL;
PFNWGLRELEASEPBUFFERDCARBPROC		wglReleasePbufferDCARB = NULL;
PFNWGLDESTROYPBUFFERARBPROC			wglDestroyPbufferARB = NULL;
PFNWGLQUERYPBUFFERARBPROC			wglQueryPbufferARB = NULL;
PFNWGLGETPIXELFORMATATTRIBIVARBPROC	wglGetPixelFormatAttribivARB = NULL;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC	wglGetPixelFormatAttribfvARB = NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB = NULL;
PFNWGLBINDTEXIMAGEARBPROC			wglBindTexImageARB = NULL;
PFNWGLRELEASETEXIMAGEARBPROC		wglReleaseTexImageARB = NULL;

#define INIT_ENTRY_POINT( funcname, type )	\
	funcname = (type)wglGetProcAddress( #funcname );	\
	if ( !funcname )	\
		gEngfuncs.Con_Printf( "#funcname() not Initialized!\n" );

CPBuffer g_PBuffer;

CPBuffer::CPBuffer()
{
	m_bValid = true;
}

CPBuffer::~CPBuffer()
{
}

void CPBuffer::BindTexture( void )
{
	if ( !wglBindTexImageARB( m_hPBuffer, WGL_FRONT_LEFT_ARB ) )
		gEngfuncs.Con_Printf( "Couldnt bind pbuffer to render texture\n" );
}

void CPBuffer::Create( void )
{
	m_hWindowDC = wglGetCurrentDC();
	m_hWindowRC = wglGetCurrentContext();

	int attribs[] =
	{
		WGL_SUPPORT_OPENGL_ARB, TRUE,
		WGL_DRAW_TO_PBUFFER_ARB, TRUE,
		WGL_BIND_TO_TEXTURE_RGBA_ARB, TRUE,
		WGL_RED_BITS_ARB, 8,
		WGL_GREEN_BITS_ARB, 8,
		WGL_BLUE_BITS_ARB, 8,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 16,
		WGL_DOUBLE_BUFFER_ARB, FALSE,
		0
	};
	int format;
	UINT count = 0;

	wglChoosePixelFormatARB( m_hWindowDC, attribs, NULL, 1, &format, &count );

	if ( !count )
	{
		gEngfuncs.Con_Printf( "Problem Finding Pixel Format\n" );
		m_bValid = false;
	}

	int pbattribs[] =
	{
		WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGBA_ARB,
		WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,
		0
	};

	m_iWidth = 1;
	while ( m_iWidth < ScreenWidth )
		m_iWidth *= 2;
	m_iHeight = m_iWidth;

	g_TextureManager.CreateTexture( m_iWidth, m_iHeight, &m_iTexture, GL_TEXTURE_2D, GL_RGBA );

	m_hPBuffer = wglCreatePbufferARB( m_hWindowDC, format, m_iWidth, m_iHeight, pbattribs );
	m_hDC = wglGetPbufferDCARB( m_hPBuffer );
	m_hRC = wglCreateContext( m_hDC );

	if ( !m_hPBuffer )
	{
		gEngfuncs.Con_Printf( "Problem Creating PBuffer\n" );
		m_bValid = false;
	}

	int width, height;
	wglQueryPbufferARB( m_hPBuffer, WGL_PBUFFER_WIDTH_ARB, &width );
	wglQueryPbufferARB( m_hPBuffer, WGL_PBUFFER_HEIGHT_ARB, &height );

	if ( height != m_iHeight || width != m_iWidth )
	{
		gEngfuncs.Con_Printf( "Pbuffer Size doesnt match\n" );
		m_bValid = false;
	}

	if ( !wglMakeCurrent( m_hDC, m_hRC ) )
	{
		gEngfuncs.Con_Printf( "Couldnt make Pbuffer Current\n" );
		m_bValid = false;
	}

	SwitchToWindow();
}

void CPBuffer::MakeCurrent( void )
{
	int lost = 0;

	wglQueryPbufferARB( m_hPBuffer, WGL_PBUFFER_LOST_ARB, &lost );

	if ( lost )
		gEngfuncs.Con_Printf( "Pbuffer was Lost\n" );

	if ( !wglMakeCurrent( m_hDC, m_hRC ) )
		gEngfuncs.Con_Printf( "Couldnt make Pbuffer Current\n" );
}

void CPBuffer::Prepare( void )
{
	wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress( "wglGetExtensionsStringARB" );

	if ( !wglGetExtensionsStringARB )
		return;

	const char *extensions = wglGetExtensionsStringARB( wglGetCurrentDC() );

	if ( !isExtensionSupported( "WGL_ARB_pixel_format", extensions ) || !isExtensionSupported( "WGL_ARB_pbuffer", extensions ) )
	{
		m_bValid = false;
		return;
	}

	if ( !m_bValid )
		return;

	INIT_ENTRY_POINT( wglCreatePbufferARB, PFNWGLCREATEPBUFFERARBPROC );
	INIT_ENTRY_POINT( wglGetPbufferDCARB, PFNWGLGETPBUFFERDCARBPROC );
	INIT_ENTRY_POINT( wglReleasePbufferDCARB, PFNWGLRELEASEPBUFFERDCARBPROC );
	INIT_ENTRY_POINT( wglDestroyPbufferARB, PFNWGLDESTROYPBUFFERARBPROC );
	INIT_ENTRY_POINT( wglQueryPbufferARB, PFNWGLQUERYPBUFFERARBPROC );
	INIT_ENTRY_POINT( wglGetPixelFormatAttribivARB, PFNWGLGETPIXELFORMATATTRIBIVARBPROC );
	INIT_ENTRY_POINT( wglGetPixelFormatAttribfvARB, PFNWGLGETPIXELFORMATATTRIBFVARBPROC );
	INIT_ENTRY_POINT( wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC );
	INIT_ENTRY_POINT( wglBindTexImageARB, PFNWGLBINDTEXIMAGEARBPROC );
	INIT_ENTRY_POINT( wglReleaseTexImageARB, PFNWGLRELEASETEXIMAGEARBPROC );

	Create();
}

void CPBuffer::ReleaseTexture( void )
{
	if ( !wglReleaseTexImageARB( m_hPBuffer, WGL_FRONT_LEFT_ARB ) )
		gEngfuncs.Con_Printf( "Couldnt release pbuffer to render texture\n" );
}

void CPBuffer::Shutdown( void )
{
}

void CPBuffer::SwitchToWindow( void )
{
	if ( !wglMakeCurrent( m_hWindowDC, m_hWindowRC ) )
		gEngfuncs.Con_Printf( "Couldnt Restore Renderer\n" );
}
