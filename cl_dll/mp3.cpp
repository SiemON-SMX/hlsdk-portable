//mp3 support added by Killar

#include "hud.h"
#include "cl_util.h"
#include "mp3.h"

#include "irrklang.h"

using namespace irrklang;

char *UTIL_VarArgs( const char *format, ... );

static CMP3		*g_pMP3;
static ISoundEngine	*g_pSoundEngine;
static ISound		*g_pSFX;
static ISound		*g_pMusic;

CMP3::CMP3()
{
	VectorClear( m_vecUnknown08 );
	VectorClear( m_vecUnknown14 );
	VectorClear( m_vecUnknown20 );
}

void CMP3::Disabled( void )
{
	return;
}

void CMP3::Frame( void )
{
	if ( !g_pSoundEngine || !m_bInitialized )
		return;

	if ( !m_pLocalPlayer )
		m_pLocalPlayer = gEngfuncs.GetLocalPlayer();

	if ( g_pSFX )
	{
		float flVolume = CVAR_GET_FLOAT( "volume" );
		if ( flVolume > 1 )
			flVolume = 1;
		if ( flVolume < 0 )
			flVolume = 0;
		g_pSFX->setVolume( flVolume );
	}

	if ( g_pMusic )
	{
		float flVolume = CVAR_GET_FLOAT( "MP3Volume" );
		if ( flVolume > 1 )
			flVolume = 1;
		if ( flVolume < 0 )
			flVolume = 0;
		g_pMusic->setVolume( flVolume );
	}

	if ( g_pMusic && m_flFadeoutStart + m_flFadeoutDuration > gEngfuncs.GetClientTime() )
	{
		float delta = ( gEngfuncs.GetClientTime() - m_flFadeoutStart ) / m_flFadeoutDuration;
		if ( delta >= 0.95 )
		{
			// fadeout finished, turn off
			g_pMusic->stop();
			return;
		}

		int vol = ( CVAR_GET_FLOAT( "MP3Volume" ) * 255 ) - (int)( delta * 255 );
		if ( vol < 0 )
			vol = 0;
		if ( vol > 255 )
			vol = 255;

		float flVolume = vol / 255.0f;
		if ( flVolume < 0 )
			flVolume = 0;
		if ( flVolume > 1 )
			flVolume = 1;

		if ( flVolume > 0.05 )
			g_pMusic->setVolume( flVolume );
		else
			g_pMusic->stop();
	}

	if ( m_bHold && g_pMusic )
	{
		float flVolume = m_iVolume / 255.0f;
		if ( flVolume < 0 )
			flVolume = 0;
		if ( flVolume > 1 )
			flVolume = 1;
		g_pMusic->setVolume( flVolume );
	}
}

int CMP3::Initialize( void )
{
	m_bInitialized = true;

	if ( g_pSoundEngine )
	{
		m_pszPending = NULL;
		m_pLocalPlayer = NULL;
		m_iUnknown2C = 0;
		m_iUnknown30 = 0;
		m_bUnknown34 = false;
		m_bUnknown35 = false;
		m_szMP3File[0] = 0;
		m_iIsPlaying = 0;
		m_iIsPaused = 0;
		m_iVolume = 0;
		m_bHold = false;
		m_flFadeoutStart = 0;
		m_flFadeoutDuration = 0;
		m_flFadeinStart = 0;
		m_flFadeinDuration = 0;
		m_bLoop = false;
		m_bStopping = false;
		return 1;
	}

	return 0;
}

void IrrKlangCreate( void )
{
	IrrKlangDestroy();

	g_pMP3 = new CMP3;
	if ( g_pMP3 )
	{
		g_pMP3->m_bInitialized = false;
		g_pSoundEngine = createIrrKlangDevice();
	}
}

void IrrKlangDestroy( void )
{
	if ( g_pSoundEngine )
		g_pSoundEngine->drop();

	if ( g_pMP3 )
	{
		delete g_pMP3;
		g_pMP3 = NULL;
	}
}

void IrrKlangFrame( void )
{
	if ( g_pMP3 )
	{
		if ( !g_pMP3->m_bInitialized )
			g_pMP3->Initialize();
		g_pMP3->Frame();
	}
}

void IrrKlangInitialize( void )
{
	if ( g_pMP3 )
		g_pMP3->Initialize();
}

bool IrrKlangIsPaused( void )
{
	if ( g_pMP3 && g_pMP3->m_iIsPaused )
		return true;
	return false;
}

void IrrKlangPlayMP3( const char *pszFile )
{
	if ( g_pMP3 )
		g_pMP3->PlayMP3( pszFile );
}

void IrrKlangPlaySFX( const char *pszFile, int iType, bool bFlag, vec3_t vecOrigin )
{
	if ( g_pMP3 )
		g_pMP3->PlaySFX( pszFile, iType, bFlag, vecOrigin );
}

void IrrKlangSetFade( float flParm1, float flParm2, float flParm3, float flParm4 )
{
	if ( g_pMP3 )
	{
		if ( flParm1 >= 0 )
			g_pMP3->m_flFadeoutStart = flParm1;
		if ( flParm2 >= 0 )
			g_pMP3->m_flFadeoutDuration = flParm2;
		if ( flParm3 >= 0 )
			g_pMP3->m_flFadeinStart = flParm3;
		if ( flParm4 >= 0 )
			g_pMP3->m_flFadeinDuration = flParm4;
	}
}

void IrrKlangSetLoop( bool bLoop )
{
	if ( g_pMP3 )
		g_pMP3->m_bLoop = bLoop;
}

void IrrKlangSetPaused( bool bPaused )
{
	if ( g_pMP3 )
		g_pMP3->SetPaused( bPaused );
}

void IrrKlangSetStopping( bool bStopping )
{
	if ( g_pMP3 )
		g_pMP3->m_bStopping = bStopping;
}

void IrrKlangStop( float flFadeTime )
{
	if ( g_pMP3 )
		g_pMP3->StopMP3( flFadeTime );
}

void __stdcall OutputAverage( const unsigned char *pData, unsigned int iSize )
{
	int iTotal = 0;

	for ( unsigned int i = 0; i < iSize; i++ )
		iTotal += pData[i];

	gEngfuncs.pfnCenterPrint( UTIL_VarArgs( "%.3f\n", (float)iTotal / (float)iSize ) );
}

int CMP3::PlayMP3( const char *pszSong )
{
	if ( !m_bInitialized || !g_pSoundEngine || gEngfuncs.CheckParm( "-nomusic", NULL ) )
		return 0;

	if ( pszSong )
	{
		if ( !strcmp( pszSong, "ap3hall1end.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "aparthall2end.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "aparthall2end2.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "aphall1end.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "basement1end.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "cityend.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "coopend.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "doctorend.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "docvic.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "hit1.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "hit2.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "hit3.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "nightmareend.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "sawend.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "sewerhit.mp3" ) )
			m_bLoop = false;
		if ( !strcmp( pszSong, "trainhit.mp3" ) )
			m_bLoop = false;
	}

	if ( m_bHold )
	{
		m_pszPending = pszSong;
		return 0;
	}

	if ( m_pszPending && pszSong && !strcmp( m_pszPending, pszSong ) )
		return 0;

	// buz: we are playing this song now, dont do anything
	if ( !strcmp( pszSong, m_szMP3File ) )
		return 0;

	m_flFadeoutStart = 0;
	m_flFadeoutDuration = 0;
	m_flFadeinStart = 0;
	m_flFadeinDuration = 0;

	char song[256];

	sprintf( song, "%s/mp3/%s", gEngfuncs.pfnGetGameDirectory(), pszSong );

	if ( m_bLoop )
	{
		if ( g_pMusic )
			g_pMusic->stop();
		g_pMusic = g_pSoundEngine->play2D( song, true, true, true );
		if ( g_pMusic )
		{
			float flVolume = CVAR_GET_FLOAT( "MP3Volume" );
			if ( flVolume > 1 )
				flVolume = 1;
			if ( flVolume < 0 )
				flVolume = 0;
			g_pMusic->setVolume( flVolume );
			g_pMusic->setIsPaused( false );
		}
	}
	else
	{
		if ( g_pMusic )
			g_pMusic->stop();
		g_pMusic = g_pSoundEngine->play2D( song, false, true, true );
		if ( g_pMusic )
		{
			float flVolume = CVAR_GET_FLOAT( "MP3Volume" );
			if ( flVolume > 1 )
				flVolume = 1;
			if ( flVolume < 0 )
				flVolume = 0;
			g_pMusic->setVolume( flVolume );
			g_pMusic->setIsPaused( false );
		}
	}

	m_iIsPlaying = 1;
	strcpy( m_szMP3File, pszSong );
	return 1;
}

int CMP3::PlayMP3Path( const char *pszPath )
{
	if ( !m_bInitialized || !g_pSoundEngine || gEngfuncs.CheckParm( "-nomusic", NULL ) )
		return 0;

	if ( m_iIsPlaying && g_pMusic )
		g_pMusic->stop();

	char song[256];

	sprintf( song, "%s", pszPath );

	if ( g_pMusic )
		g_pMusic->stop();
	g_pMusic = g_pSoundEngine->play2D( pszPath, false, true, true );
	if ( g_pMusic )
	{
		g_pMusic->setVolume( CVAR_GET_FLOAT( "MP3Volume" ) );
		g_pMusic->setIsPaused( false );
	}

	m_iIsPlaying = 1;
	return 1;
}

void CMP3::PlaySFX( const char *pszSample, int iType, bool bFlag, Vector vecOrigin )
{
	if ( !m_bInitialized || !g_pSoundEngine )
		return;

	char szNewSample[256];
	char szPath[256];

	if ( strstr( pszSample, "weapons/" ) )
	{
		bool bFound = false;
		int j = 0;

		for ( unsigned int i = 0; i < strlen( pszSample ); i++ )
		{
			if ( bFound )
				szPath[j++] = pszSample[i];
			else if ( pszSample[i] == '/' || pszSample[i] == '\\' )
				bFound = true;
		}

		if ( bFound )
		{
			szPath[j] = 0;
			strcpy( szNewSample, "weapons_new/" );
			strcat( szNewSample, szPath );
			pszSample = szNewSample;
		}
	}

	if ( iType == 1 )
	{
		if ( g_pSFX )
			g_pSFX->stop();

		sprintf( szPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), pszSample );
		g_pSFX = g_pSoundEngine->play2D( szPath, true, true, true );
		if ( g_pSFX )
		{
			float flVolume = CVAR_GET_FLOAT( "volume" );
			if ( flVolume > 1 )
				flVolume = 1;
			if ( flVolume < 0 )
				flVolume = 0;
			g_pSFX->setVolume( flVolume );
			g_pSFX->setPlaybackSpeed( gEngfuncs.pfnRandomFloat( -0.1, 0.1 ) + 1.0 );
			g_pSFX->setIsPaused( false );
		}
	}
	else if ( iType == 2 )
	{
		if ( g_pSFX )
			g_pSFX->stop();
	}
	else
	{
		sprintf( szPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), pszSample );
		ISound *pSound = g_pSoundEngine->play2D( szPath, false, true, true );
		if ( pSound )
		{
			float flVolume = CVAR_GET_FLOAT( "volume" );
			if ( flVolume > 1 )
				flVolume = 1;
			if ( flVolume < 0 )
				flVolume = 0;
			pSound->setVolume( flVolume );
			pSound->setPlaybackSpeed( gEngfuncs.pfnRandomFloat( -0.1, 0.1 ) + 1.0 );
			pSound->setIsPaused( false );
		}
	}
}

int CMP3::SetPaused( bool bPaused )
{
	if ( !m_bInitialized || !g_pSoundEngine )
		return 0;

	if ( bPaused )
	{
		m_iIsPaused = 1;
		g_pSoundEngine->setAllSoundsPaused( true );
		return 1;
	}

	m_iIsPaused = 0;
	g_pSoundEngine->setAllSoundsPaused( false );
	return 1;
}

int CMP3::Shutdown( void )
{
	return 1;
}

int CMP3::StopMP3( float fade )
{
	if ( !m_bInitialized )
		return 0;

	if ( !g_pSoundEngine || gEngfuncs.CheckParm( "-nomusic", NULL ) )
		return 1;

	m_bStopping = true;

	if ( m_bHold )
		m_bHold = false;

	m_szMP3File[0] = 0;

	if ( fade == 0 )
	{
		if ( g_pMusic )
			g_pMusic->stop();
		m_iIsPlaying = 0;
		m_flFadeoutStart = 0;
		m_flFadeoutDuration = 0;
		m_bStopping = false;
		return 1;
	}

	m_flFadeoutStart = gEngfuncs.GetClientTime();
	m_flFadeoutDuration = fade;
	return 1;
}
