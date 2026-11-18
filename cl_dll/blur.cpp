
#include "hud.h"
#include "client_port.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "glmanager.h"

extern cvar_t *cv_blurtest;

class CBlurTexture
{
public:
	CBlurTexture();

	void Init( int width, int height );
	void BindTexture( int width, int height );
	void DrawQuad( int width, int height, int of );
	void DrawBlended( int width, int height );

	unsigned int	m_uiTexture;
	float			m_flAlpha;
	float			m_flRed;
	float			m_flGreen;
	float			m_flBlue;
};

class CBlur
{
public:
	void Initialize( void );
	void DrawBlur( void );
	bool AnimateNextFrame( int desiredFrameRate );

	int					m_iActive;
	CBlurTexture	m_Output;
};

CBlurTexture::CBlurTexture()
{
}

void CBlurTexture::Init( int width, int height )
{
	unsigned char *pBlankTex = new unsigned char[width * height * 3];
	memset( pBlankTex, 0, width * height * 3 );

	glGenTextures( 1, &m_uiTexture );
	glBindTexture( GL_TEXTURE_RECTANGLE_NV, m_uiTexture );
	glTexParameteri( GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA8, width, height, 0, GL_RGBA8, GL_UNSIGNED_BYTE, NULL );

	delete[] pBlankTex;
}

void CBlurTexture::BindTexture( int width, int height )
{
	glBindTexture( GL_TEXTURE_RECTANGLE_NV, m_uiTexture );
	glCopyTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA8, 0, 0, ScreenWidth, ScreenHeight, 0 );
}

void CBlurTexture::DrawQuad( int width, int height, int of )
{
	glTexCoord2f( -of, -of );
	glVertex3f( 0, 1, -1 );
	glTexCoord2f( -of, height + of );
	glVertex3f( 0, 0, -1 );
	glTexCoord2f( width + of, height + of );
	glVertex3f( 1, 0, -1 );
	glTexCoord2f( width + of, -of );
	glVertex3f( 1, 1, -1 );
}

void CBlurTexture::DrawBlended( int width, int height )
{
	glEnable( GL_TEXTURE_RECTANGLE_NV );
	glColor3f( 1, 1, 1 );
	glDisable( GL_DEPTH_TEST );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, 1, 1, 0, 0.1, 100 );

	glBindTexture( GL_TEXTURE_RECTANGLE_NV, m_uiTexture );
	glColor4f( m_flRed, m_flGreen, m_flBlue, m_flAlpha );

	if ( cv_blurtest->value )
	{
		glBegin( GL_QUADS );
		DrawQuad( ScreenWidth, ScreenHeight, 0 );
		glEnd();
	}

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	glDisable( GL_TEXTURE_RECTANGLE_NV );
	glEnable( GL_DEPTH_TEST );
}

void CBlur::Initialize( void )
{
	m_iActive = 1;
	m_Output.Init( 320, 240 );
	m_Output.m_flRed = 1.0;
	m_Output.m_flGreen = 1.0;
	m_Output.m_flBlue = 1.0;
}

void CBlur::DrawBlur( void )
{
	if ( cv_blurtest->value )
	{
		glBlendFunc( GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA );
		glEnable( GL_BLEND );

		m_Output.m_flAlpha = 0.4;
		m_Output.m_flRed = 1.0;
		m_Output.m_flGreen = 1.0;
		m_Output.m_flBlue = 1.0;
		m_Output.DrawBlended( ScreenWidth, ScreenHeight );
		m_Output.BindTexture( ScreenWidth, ScreenHeight );

		glDisable( GL_BLEND );
	}
}

bool CBlur::AnimateNextFrame( int desiredFrameRate )
{
	static float lastTime = 0.0f;
	float elapsedTime = 0.0;

	float currentTime = GetTickCount() * 0.001f;
	elapsedTime = currentTime - lastTime;

	if ( elapsedTime > ( 1.0f / desiredFrameRate ) )
	{
		lastTime = currentTime;
		return true;
	}

	return false;
}
