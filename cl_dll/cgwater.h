
#ifndef CGWATER_H
#define CGWATER_H

#include "cgcontext.h"

class CCgWater
{
public:
	CCgWater();

	void DrawBlackQuad( void );
	void EndShaders( void );
	void Init( void );
	void PostDraw( void );
	void PreDraw( void );
	void Render( void );
	void RenderWater( void );
	void ScreenQuad( void );
	void SetBlackShader( void );
	void UpdateEntities( void );

	PFNGLACTIVETEXTUREARBPROC	glActiveTextureARB;
	PFNGLMULTITEXCOORD2FARBPROC	glMultiTexCoord2fARB;
	bool	m_bAllowTextureShader;
	bool	m_bEnabled;
	bool	m_bDrawn;
	float	m_flTexWidth;
	float	m_flTexHeight;
	GLuint	m_iMaskTexture;
	GLuint	m_iDistortTexture;
	GLuint	m_iNoiseTexture;
	GLuint	m_iSceneTexture;
	GLuint	m_iOffsetTexture;
	cgprogram_t *m_pWaterFP;
	cgprogram_t *m_pWaterVP;
	cgprogram_t *m_pBlackFP;
	cgprogram_t *m_pBlackVP;
	CGparameter	m_pBlackModelViewProj;
	CGparameter	m_pModelViewProj;
	CGparameter	m_pTexOffset;
	CGparameter	m_pScreen;
	GLint	m_iSavedTexture1;
	GLint	m_iSavedTexture2;
};

extern CCgWater g_CgWater;

#endif
