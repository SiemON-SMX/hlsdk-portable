
#include <string>
#include <list>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "cl_entity.h"
#include "studio_util.h"
#include "r_studioint.h"
#include "glmanager.h"
#include "gl_texloader.h"
#include "cgwater.h"
#include "xash_render.h"

extern engine_studio_api_t IEngineStudio;
extern int current_ext_texture_id;
extern GLint g_iWaterOldTexture;

int iUnknown1054506C;
CCgWater g_CgWater;

static GLint g_iCoFWaterViewport[4];

static void WaterUpdateViewport( void )
{
	glGetIntegerv( GL_VIEWPORT, g_iCoFWaterViewport );

	if ( g_iCoFWaterViewport[2] <= 0 || g_iCoFWaterViewport[3] <= 0 )
	{
		g_iCoFWaterViewport[0] = g_iCoFWaterViewport[1] = 0;
		g_iCoFWaterViewport[2] = ScreenWidth;
		g_iCoFWaterViewport[3] = ScreenHeight;
	}
}

#define WATER_X g_iCoFWaterViewport[0]
#define WATER_Y g_iCoFWaterViewport[1]
#define WATER_W g_iCoFWaterViewport[2]
#define WATER_H g_iCoFWaterViewport[3]

CCgWater::CCgWater()
{
	m_bEnabled = false;
}

void CCgWater::DrawBlackQuad( void )
{
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, 1, 1, 0, 0.1f, 100 );

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glDisable( GL_TEXTURE_2D );
	glColor4f( 0, 0, 0, 1 );

	glBegin( GL_QUADS );
	glVertex3f( 0, 1, -1 );
	glVertex3f( 1, 1, -1 );
	glVertex3f( 1, 0, -1 );
	glVertex3f( 0, 0, -1 );
	glEnd();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
}

void CCgWater::EndShaders( void )
{
	g_CgContext.DisableFragmentProfile();
	g_CgContext.DisableVertexProfile();
}

void CCgWater::Init( void )
{
	if ( IEngineStudio.IsHardware() != 1 )
		return;

	glActiveTextureARB = gl.glActiveTextureARB;
	glMultiTexCoord2fARB = gl.glMultiTexCoord2fARB;

	m_bAllowTextureShader = true;
	m_bEnabled = true;
	m_bDrawn = false;

	if ( !g_TextureManager.IsHardware() )
	{
		m_bEnabled = false;
		return;
	}

	if ( !(int)gEngfuncs.pfnGetCvarFloat( "cg_water" ) )
	{
		gEngfuncs.Con_DPrintf( "Disabling CG Effects\n" );
		m_bEnabled = false;
	}

	bool bTextureShader = true;
	if ( !isExtensionSupported( "GL_NV_texture_shader", strdup( (const char *)glGetString( GL_EXTENSIONS ) ) ) )
		bTextureShader = false;

	if ( !m_bEnabled )
		return;

	if ( !CgAvailable() || !glActiveTextureARB || !glMultiTexCoord2fARB )
	{
		m_bEnabled = false;
		return;
	}

	WaterUpdateViewport();

	m_flTexWidth = 2;
	while ( m_flTexWidth < WATER_W )
		m_flTexWidth *= 2;

	m_flTexHeight = 2;
	while ( m_flTexHeight < WATER_H )
		m_flTexHeight *= 2;

	m_iSceneTexture = 256;
	g_TextureManager.CreateTexture( m_flTexWidth, m_flTexHeight, &m_iSceneTexture, GL_TEXTURE_2D, GL_RGB );
	g_TextureManager.CreateTexture( m_flTexWidth, m_flTexHeight, &m_iMaskTexture, GL_TEXTURE_2D, GL_RGB );
	g_TextureManager.CreateTexture( 256, 256, &m_iOffsetTexture, GL_TEXTURE_2D, GL_RGB );
	g_TextureManager.CreateTexture( m_flTexWidth, m_flTexHeight, &m_iDistortTexture, GL_TEXTURE_2D, GL_RGB );

	unsigned char offsets[256][256][2];

	for ( int t = 0; t < 256; t++ )
	{
		for ( int s = 0; s < 256; s++ )
		{
			offsets[t][s][0] = s;
			offsets[t][s][1] = t;
		}
	}

	glBindTexture( GL_TEXTURE_2D, m_iOffsetTexture );

	if ( bTextureShader )
		glTexImage2D( GL_TEXTURE_2D, 0, GL_DSDT8_NV, 256, 256, 0, GL_DSDT_NV, GL_UNSIGNED_BYTE, offsets );
	else
		glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, offsets );

	if ( !m_iNoiseTexture )
		m_iNoiseTexture = current_ext_texture_id++;
	CreateTexture( "textures/noise.tga", 1, m_iNoiseTexture );

	m_pWaterVP = g_CgContext.AddProgram( "water_vp.cg", CGTYPE_VERTEX );

	if ( m_pWaterVP )
	{
		m_pModelViewProj = cgGetNamedParameter( m_pWaterVP->program, "ModelViewProj" );
		m_pScreen = cgGetNamedParameter( m_pWaterVP->program, "flScreen" );
	}
	else
		gEngfuncs.Con_Printf( "Error on vertex\n" );

	const char *water, *black;

	if ( g_CgContext.GetFragmentProfile() == CG_PROFILE_FP20 )
	{
		black = "fp20_black_fp.cg";
		water = "fp20_water_fp.cg";
	}
	else
	{
		black = "black_fp.cg";
		water = "water_fp.cg";
	}

	m_pWaterFP = g_CgContext.AddProgram( water, CGTYPE_FRAGMENT );
	m_pBlackFP = g_CgContext.AddProgram( black, CGTYPE_FRAGMENT );
	m_pBlackVP = g_CgContext.AddProgram( "black_vp.cg", CGTYPE_VERTEX );
	m_pBlackModelViewProj = m_pBlackVP ? cgGetNamedParameter( m_pBlackVP->program, "ModelViewProj" ) : NULL;

	if ( !m_pBlackFP )
		gEngfuncs.Con_Printf( "Error Loading Black Shader\n" );

	if ( m_pWaterFP )
		m_pTexOffset = cgGetNamedParameter( m_pWaterFP->program, "TexOffset" );
	else
		gEngfuncs.Con_Printf( "Error on fragment\n" );

	m_pFastWaterVP = m_pFastWaterFP = NULL;
	m_pFastTexOffset = m_pFastScreenInfo = m_pFastModelViewProj = NULL;
#if defined( __ANDROID__ ) || defined( __APPLE__ )
	m_pFastWaterVP = g_CgContext.AddProgram( "fastwater_vp.cg", CGTYPE_VERTEX );
	m_pFastWaterFP = g_CgContext.AddProgram( "fastwater_fp.cg", CGTYPE_FRAGMENT );

	if ( m_pFastWaterVP )
		m_pFastModelViewProj = cgGetNamedParameter( m_pFastWaterVP->program, "ModelViewProj" );

	if ( m_pFastWaterFP )
	{
		m_pFastTexOffset = cgGetNamedParameter( m_pFastWaterFP->program, "TexOffset" );
		m_pFastScreenInfo = cgGetNamedParameter( m_pFastWaterFP->program, "ScreenInfo" );
	}
#endif

	glActiveTextureARB( GL_TEXTURE0_ARB );
}

void CCgWater::PostDraw( void )
{
	if ( !g_TextureManager.IsHardware() )
		return;

	if ( !glActiveTextureARB )
		return;

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glDisable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_iSavedTexture1 );

	glActiveTextureARB( GL_TEXTURE2_ARB );
	glDisable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_iSavedTexture2 );

	glActiveTextureARB( GL_TEXTURE3_ARB );
	glDisable( GL_TEXTURE_2D );

	glActiveTextureARB( GL_TEXTURE0_ARB );
}

void CCgWater::PreDraw( void )
{
	if ( !g_TextureManager.IsHardware() )
		return;

	if ( !glActiveTextureARB )
		return;

	if ( !m_iSavedTexture1 || !m_iSavedTexture2 )
		return;

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glGetIntegerv( GL_TEXTURE_BINDING_2D, &m_iSavedTexture1 );

	glActiveTextureARB( GL_TEXTURE2_ARB );
	glGetIntegerv( GL_TEXTURE_BINDING_2D, &m_iSavedTexture2 );

	glActiveTextureARB( GL_TEXTURE0_ARB );
}

static bool WaterEntityPresent( void )
{
	for ( int i = 0; i < 1024; i++ )
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( i );

		if ( ent && ent->curstate.skin == CONTENTS_WATER && ent->model )
			return true;
	}

	return false;
}

struct WaterUnitState
{
	GLboolean tex2d;
	GLint bind;
	GLint envmode;
};

static void WaterSaveState( WaterUnitState *units, GLint *active )
{
	glGetIntegerv( GL_ACTIVE_TEXTURE_ARB, active );
	gl.glPushAttrib( GL_ALL_ATTRIB_BITS );

	for ( int i = 0; i < 4; i++ )
	{
		gl.glActiveTextureARB( GL_TEXTURE0_ARB + i );
		units[i].tex2d = gl.glIsEnabled( GL_TEXTURE_2D );
		glGetIntegerv( GL_TEXTURE_BINDING_2D, &units[i].bind );
		gl.glGetTexEnviv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &units[i].envmode );

		if ( i > 0 )
			glDisable( GL_TEXTURE_2D );
		else
			gl.glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	}

	gl.glActiveTextureARB( GL_TEXTURE0_ARB );
}

static void WaterRestoreState( const WaterUnitState *units, GLint active )
{
	for ( int i = 3; i >= 0; i-- )
	{
		gl.glActiveTextureARB( GL_TEXTURE0_ARB + i );
		glBindTexture( GL_TEXTURE_2D, units[i].bind );
		gl.glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, units[i].envmode );

		if ( units[i].tex2d )
			glEnable( GL_TEXTURE_2D );
		else
			glDisable( GL_TEXTURE_2D );
	}

	gl.glActiveTextureARB( active );
	gl.glPopAttrib();
	GL_ResetEngineState();
}

void CCgWater::Render( void )
{
	if ( !m_bEnabled )
		Init();

	m_bDrawn = true;

	if ( !m_bEnabled || !WaterEntityPresent() )
		return;

	WaterUnitState units[4];
	GLint active = GL_TEXTURE0_ARB;

	WaterSaveState( units, &active );
	RenderWater();
	WaterRestoreState( units, active );
}

void CCgWater::RenderWater( void )
{
	WaterUpdateViewport();

	if ( WATER_W > m_flTexWidth || WATER_H > m_flTexHeight )
	{
		m_bEnabled = false;
		Init();
		if ( !m_bEnabled )
			return;
	}

	if ( m_pFastWaterVP && m_pFastWaterFP && m_pFastTexOffset && m_pFastScreenInfo && m_pFastModelViewProj )
	{
		RenderFastWater();
		return;
	}

	glGetIntegerv( GL_TEXTURE_BINDING_2D, &g_iWaterOldTexture );

	glDisable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );

	// the scene
	glBindTexture( GL_TEXTURE_2D, m_iSceneTexture );
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, WATER_X, WATER_Y, WATER_W, WATER_H );
	DrawBlackQuad();

	glDisable( GL_CULL_FACE );
	glDepthMask( GL_FALSE );
	glBindTexture( GL_TEXTURE_2D, m_iNoiseTexture );
	glColor4f( 1, 1, 1, 1 );

	int i;
	cl_entity_t *ent;

	for ( i = 0; i < 1024; i++ )
	{
		ent = gEngfuncs.GetEntityByIndex( i );

		if ( !ent || ent->curstate.skin != CONTENTS_WATER )
			continue;

		float maxs1 = ent->curstate.maxs[1];
		float mins1 = ent->curstate.mins[1];
		float maxs0 = ent->curstate.maxs[0];
		float mins0 = ent->curstate.mins[0];

		float s = gEngfuncs.pfnGetCvarFloat( "cl_waterspeed_x" );
		s = gEngfuncs.GetClientTime() / s;
		float t = gEngfuncs.pfnGetCvarFloat( "cl_waterspeed_y" );
		t = cos( gEngfuncs.GetClientTime() ) / t;

		vec3_t mins = ent->curstate.mins;
		vec3_t maxs = ent->curstate.maxs;
		float z = maxs[2] + 1.0f;

		vec3_t v1, v2, v3, v4;
		v1[0] = mins[0];
		v1[1] = mins[1];
		v1[2] = z;
		v2[0] = mins[0];
		v2[1] = maxs[1];
		v2[2] = z;
		v3[0] = maxs[0];
		v3[1] = maxs[1];
		v3[2] = z;
		v4[0] = maxs[0];
		v4[1] = mins[1];
		v4[2] = z;

		glBegin( GL_QUADS );

		vec3_t tc1, tc2, tc3, tc4;
		float s0 = s + 0;
		float t0 = t + 0;
		float r0 = t0 + 0;
		tc1[0] = s0;
		tc1[1] = t0;
		tc1[2] = r0;
		glTexCoord2fv( tc1 );
		glVertex3fv( v1 );

		float t1 = ( maxs1 - mins1 ) * ( 1.0f / 128 ) + t;
		tc2[0] = s0;
		tc2[1] = t1;
		tc2[2] = r0;
		glTexCoord2fv( tc2 );
		glVertex3fv( v2 );

		float s1 = ( maxs0 - mins0 ) * ( 1.0f / 128 ) + s;
		tc3[0] = s1;
		tc3[1] = t1;
		tc3[2] = r0;
		glTexCoord2fv( tc3 );
		glVertex3fv( v3 );

		tc4[0] = s1;
		tc4[1] = t0;
		tc4[2] = r0;
		glTexCoord2fv( tc4 );
		glVertex3fv( v4 );

		glEnd();
	}

	glEnable( GL_CULL_FACE );
	glDepthMask( GL_TRUE );
	glBindTexture( GL_TEXTURE_2D, m_iDistortTexture );
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, WATER_X, WATER_Y, WATER_W, WATER_H );
	DrawBlackQuad();

	glDisable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );
	glDisable( GL_CULL_FACE );

	for ( i = 0; i < 1024; i++ )
	{
		ent = gEngfuncs.GetEntityByIndex( i );

		if ( !ent || ent->curstate.skin != CONTENTS_WATER )
			continue;

		vec3_t mins = ent->curstate.mins;
		vec3_t maxs = ent->curstate.maxs;
		vec3_t v1, v2, v3, v4;
		v1[0] = mins[0];
		v1[1] = mins[1];
		v1[2] = maxs[2];
		v2[0] = mins[0];
		v2[1] = maxs[1];
		v2[2] = maxs[2];
		v3[0] = maxs[0];
		v3[1] = maxs[1];
		v3[2] = maxs[2];
		v4[0] = maxs[0];
		v4[1] = mins[1];
		v4[2] = maxs[2];

		glBegin( GL_QUADS );
		glVertex3fv( v1 );
		glVertex3fv( v2 );
		glVertex3fv( v3 );
		glVertex3fv( v4 );
		glEnd();
	}

	glEnable( GL_CULL_FACE );
	glDepthMask( GL_FALSE );
	glBindTexture( GL_TEXTURE_2D, m_iMaskTexture );
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, WATER_X, WATER_Y, WATER_W, WATER_H );
	DrawBlackQuad();

	glBindTexture( GL_TEXTURE_2D, m_iSceneTexture );
	ScreenQuad();

	g_CgContext.EnableVertexProfile();
	g_CgContext.EnableFragmentProfile();

	if ( m_pWaterVP )
		g_CgContext.BindProgram( m_pWaterVP );
	else
		gEngfuncs.Con_Printf( "Error on Vertex\n" );

	if ( m_pWaterFP )
		g_CgContext.BindProgram( m_pWaterFP );
	else
		gEngfuncs.Con_Printf( "Error on fragment\n" );

	glActiveTextureARB( GL_TEXTURE0_ARB );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_iDistortTexture );

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_iOffsetTexture );

	glActiveTextureARB( GL_TEXTURE2_ARB );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_iSceneTexture );

	glActiveTextureARB( GL_TEXTURE3_ARB );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_iMaskTexture );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, 1, 1, 0, 0.1f, 4096 );

	cgGLSetStateMatrixParameter( m_pModelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY );

	float offset[4] = { 0, 0.05f, 0, 0.05f };
	cgGLSetParameter4fv( m_pTexOffset, offset );
	cgGLSetParameter1f( m_pScreen, 0.965f );

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glColor4f( 1, 1, 1, 1 );

	glBegin( GL_QUADS );
	glTexCoord2f( 0, 0 );
	glVertex3f( 0, 1, -1 );
	glTexCoord2f( WATER_W / m_flTexWidth, 0 );
	glVertex3f( 1, 1, -1 );
	glTexCoord2f( WATER_W / m_flTexWidth, WATER_H / m_flTexHeight );
	glVertex3f( 1, 0, -1 );
	glTexCoord2f( 0, WATER_H / m_flTexHeight );
	glVertex3f( 0, 0, -1 );
	glEnd();

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glDisable( GL_TEXTURE_2D );
	glActiveTextureARB( GL_TEXTURE2_ARB );
	glDisable( GL_TEXTURE_2D );
	glActiveTextureARB( GL_TEXTURE3_ARB );
	glDisable( GL_TEXTURE_2D );
	glActiveTextureARB( GL_TEXTURE0_ARB );

	g_CgContext.DisableFragmentProfile();
	g_CgContext.DisableVertexProfile();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glDisable( GL_BLEND );
	glBindTexture( GL_TEXTURE_2D, g_iWaterOldTexture );
}

// single pass water, one screen copy
void CCgWater::RenderFastWater( void )
{
	glGetIntegerv( GL_TEXTURE_BINDING_2D, &g_iWaterOldTexture );

	glDisable( GL_BLEND );

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_iSceneTexture );
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, WATER_X, WATER_Y, WATER_W, WATER_H );

	glActiveTextureARB( GL_TEXTURE0_ARB );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, m_iNoiseTexture );

	g_CgContext.EnableVertexProfile();
	g_CgContext.EnableFragmentProfile();
	g_CgContext.BindProgram( m_pFastWaterVP );
	g_CgContext.BindProgram( m_pFastWaterFP );

	float offset[4] = { 0, 0.05f, 0, 0.05f };
	float screen[4] = { 1.0f / m_flTexWidth, 1.0f / m_flTexHeight, (float)WATER_X, (float)WATER_Y };
	cgGLSetParameter4fv( m_pFastTexOffset, offset );
	cgGLSetParameter4fv( m_pFastScreenInfo, screen );
	cgGLSetStateMatrixParameter( m_pFastModelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY );

	glEnable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glDepthMask( GL_FALSE );
	glColor4f( 1, 1, 1, 1 );

	float speedx = gEngfuncs.pfnGetCvarFloat( "cl_waterspeed_x" );
	float speedy = gEngfuncs.pfnGetCvarFloat( "cl_waterspeed_y" );
	float time = gEngfuncs.GetClientTime();
	float s = time / speedx;
	float t = cos( time ) / speedy;

	for ( int i = 0; i < 1024; i++ )
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( i );

		if ( !ent || ent->curstate.skin != CONTENTS_WATER )
			continue;

		vec3_t mins = ent->curstate.mins;
		vec3_t maxs = ent->curstate.maxs;
		float s1 = ( maxs[0] - mins[0] ) * ( 1.0f / 128 ) + s;
		float t1 = ( maxs[1] - mins[1] ) * ( 1.0f / 128 ) + t;

		glBegin( GL_QUADS );
		glTexCoord2f( s, t );
		glVertex3f( mins[0], mins[1], maxs[2] );
		glTexCoord2f( s, t1 );
		glVertex3f( mins[0], maxs[1], maxs[2] );
		glTexCoord2f( s1, t1 );
		glVertex3f( maxs[0], maxs[1], maxs[2] );
		glTexCoord2f( s1, t );
		glVertex3f( maxs[0], mins[1], maxs[2] );
		glEnd();
	}

	g_CgContext.DisableFragmentProfile();
	g_CgContext.DisableVertexProfile();

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glDisable( GL_TEXTURE_2D );
	glActiveTextureARB( GL_TEXTURE0_ARB );

	glEnable( GL_CULL_FACE );
	glDepthMask( GL_TRUE );
	glBindTexture( GL_TEXTURE_2D, g_iWaterOldTexture );
}

void CCgWater::ScreenQuad( void )
{
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, 1, 1, 0, 0.1f, 100 );

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glColor4f( 1, 1, 1, 1 );

	glBegin( GL_QUADS );
	glTexCoord2f( 0, 0 );
	glVertex3f( 0, 1, -1 );
	glTexCoord2f( WATER_W / m_flTexWidth, 0 );
	glVertex3f( 1, 1, -1 );
	glTexCoord2f( WATER_W / m_flTexWidth, WATER_H / m_flTexHeight );
	glVertex3f( 1, 0, -1 );
	glTexCoord2f( 0, WATER_H / m_flTexHeight );
	glVertex3f( 0, 0, -1 );
	glEnd();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
}

void CCgWater::SetBlackShader( void )
{
	g_CgContext.EnableFragmentProfile();
	g_CgContext.EnableVertexProfile();

	if ( m_pBlackFP )
		g_CgContext.BindProgram( m_pBlackFP );

	if ( m_pBlackVP )
		g_CgContext.BindProgram( m_pBlackVP );

	cgGLSetStateMatrixParameter( m_pBlackModelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY );
}

void CCgWater::UpdateEntities( void )
{
	if ( !(int)gEngfuncs.pfnGetCvarFloat( "cg_water" ) )
	{
		m_bEnabled = false;
		return;
	}

	if ( !m_bEnabled )
		return;

	for ( int i = 0; i < 1024; i++ )
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex( i );

		if ( !ent || ent->curstate.skin != CONTENTS_WATER )
			continue;

		ent->baseline.effects = EF_NODRAW;
		ent->baseline.renderamt = 0;
		ent->baseline.rendermode = kRenderTransAdd;
		ent->prevstate.effects = EF_NODRAW;
		ent->prevstate.renderamt = 0;
		ent->prevstate.rendermode = kRenderTransAdd;
		ent->curstate.effects = EF_NODRAW;
		ent->curstate.renderamt = 0;
		ent->curstate.rendermode = kRenderTransAdd;
	}
}
