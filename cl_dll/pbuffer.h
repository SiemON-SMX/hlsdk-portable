
#ifndef PBUFFER_H
#define PBUFFER_H

DECLARE_HANDLE( HPBUFFERARB );

class CPBuffer
{
public:
	CPBuffer();
	~CPBuffer();

	void BindTexture( void );
	void Create( void );
	void MakeCurrent( void );
	void Prepare( void );
	void ReleaseTexture( void );
	void Shutdown( void );
	void SwitchToWindow( void );

	bool	m_bValid;
	int		m_iHeight;
	int		m_iWidth;
	GLuint	m_iTexture;
	HDC		m_hWindowDC;
	HDC		m_hDC;
	HGLRC	m_hRC;
	HPBUFFERARB m_hPBuffer;
	HGLRC	m_hWindowRC;
};

extern CPBuffer g_PBuffer;

#endif
