#include "hud.h"
#include "cl_util.h"
#include "event_api.h"
#include "glmanager.h"
#include "irrklang.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define SOUND_MUSIC	1
#define SOUND_SFX	2
#define SFX_POOL	32

namespace irrklang
{
static ISoundEngine g_SoundEngine;
static ISound g_Music;
static ISound g_SFXPool[SFX_POOL];
static int g_iCoFNextSFX;
static float g_flCoFMusicFade;
static bool g_bCoFPaused;

static void SetMusicFade( float percent )
{
	if( percent < 0.0f )
		percent = 0.0f;
	if( percent > 100.0f )
		percent = 100.0f;

	g_flCoFMusicFade = percent;

	if( g_pXashRenderAPI && g_pXashRenderAPI->S_FadeMusicVolume )
		g_pXashRenderAPI->S_FadeMusicVolume( g_bCoFPaused ? 100.0f : g_flCoFMusicFade );
}

static const char *StripGameDir( const char *path )
{
	const char *gamedir = gEngfuncs.pfnGetGameDirectory();
	size_t len;

	if( gamedir && *gamedir )
	{
		len = strlen( gamedir );

		if( !strncasecmp( path, gamedir, len ) && ( path[len] == '/' || path[len] == '\\' ))
			return path + len + 1;
	}

	return path;
}

static int LocalPlayerIndex( void )
{
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();
	return player ? player->index : 0;
}

static float WavLoopLength( const char *sample )
{
	char path[300];
	int length = 0;
	float seconds = 0.0f;
	int rate = 0, channels = 0, bits = 0;
	bool cue = false;
	unsigned int datalen = 0;

	snprintf( path, sizeof( path ), "sound/%s", sample );

	unsigned char *buf = gEngfuncs.COM_LoadFile( path, 5, &length );

	if( !buf )
		return 0.0f;

	if( length >= 12 && !memcmp( buf, "RIFF", 4 ) && !memcmp( buf + 8, "WAVE", 4 ))
	{
		int pos = 12;

		while( pos + 8 <= length )
		{
			unsigned int len = buf[pos + 4] | ( buf[pos + 5] << 8 ) | ( buf[pos + 6] << 16 ) | ( (unsigned int)buf[pos + 7] << 24 );

			if( !memcmp( buf + pos, "fmt ", 4 ) && pos + 24 <= length )
			{
				channels = buf[pos + 10] | ( buf[pos + 11] << 8 );
				rate = buf[pos + 12] | ( buf[pos + 13] << 8 ) | ( buf[pos + 14] << 16 ) | ( buf[pos + 15] << 24 );
				bits = buf[pos + 22] | ( buf[pos + 23] << 8 );
			}
			else if( !memcmp( buf + pos, "data", 4 ))
				datalen = len;
			else if( !memcmp( buf + pos, "cue ", 4 ))
				cue = true;

			if( len > (unsigned int)length )
				break;

			pos += 8 + len + ( len & 1 );
		}

		if( !cue && rate > 0 && channels > 0 && bits >= 8 )
			seconds = (float)datalen / (float)( rate * channels * ( bits / 8 ));
	}

	gEngfuncs.COM_FreeFile( buf );
	return seconds;
}

ISound::ISound()
{
	m_iKind = 0;
	m_bLoop = false;
	m_bPlaying = false;
	m_flVolume = 1.0f;
	m_flSpeed = 1.0f;
	m_flLoopLength = 0.0f;
	m_flNextLoop = 0.0f;
	m_szSample[0] = '\0';
}

void ISound::setVolume( float volume )
{
	if( volume < 0.0f )
		volume = 0.0f;
	if( volume > 1.0f )
		volume = 1.0f;

	m_flVolume = volume;

	if( m_iKind == SOUND_MUSIC )
	{
		float flMusicVolume = gEngfuncs.pfnGetCvarFloat( "MP3Volume" );

		if( flMusicVolume <= 0.0f )
			SetMusicFade( 0.0f );
		else
			SetMusicFade(( 1.0f - volume / flMusicVolume ) * 100.0f );
	}
}

void ISound::stop( void )
{
	if( m_iKind == SOUND_MUSIC )
	{
		if( m_bPlaying )
			gEngfuncs.pfnPrimeMusicStream( NULL, 0 );

		SetMusicFade( 0.0f );
	}
	else if( m_iKind == SOUND_SFX && m_bPlaying && m_szSample[0] )
	{
		gEngfuncs.pEventAPI->EV_StopSound( LocalPlayerIndex(), m_bLoop ? CHAN_STATIC : CHAN_AUTO, m_szSample );
	}

	m_bPlaying = false;
}

// irrklang is gone, play the sound through the xash sound system
void ISound::setIsPaused( bool paused )
{
	if( paused || m_bPlaying )
		return;

	if( m_iKind == SOUND_MUSIC )
	{
		gEngfuncs.pfnPrimeMusicStream( m_szSample, m_bLoop ? 1 : 0 );
		m_bPlaying = true;
	}
	else if( m_iKind == SOUND_SFX )
	{
		cl_entity_t *player = gEngfuncs.GetLocalPlayer();
		static char s_szLastSample[256];
		static float s_flLastTime = -1000.0f;
		float now = gEngfuncs.GetClientTime();

		if( !player )
			return;

		// same sound in the same frame, skip it (m16 burst is 0.05 apart)
		if( !m_bLoop && !strcasecmp( s_szLastSample, m_szSample ) && now >= s_flLastTime && now - s_flLastTime < 0.03f )
		{
			m_bPlaying = true;
			return;
		}

		strncpy( s_szLastSample, m_szSample, sizeof( s_szLastSample ) - 1 );
		s_flLastTime = now;

		gEngfuncs.pEventAPI->EV_PlaySound( player->index, player->origin, m_bLoop ? CHAN_STATIC : CHAN_AUTO, m_szSample, VOL_NORM, ATTN_NONE, 0, PITCH_NORM );
		m_bPlaying = true;

		if( m_bLoop )
		{
			m_flLoopLength = WavLoopLength( m_szSample );
			m_flNextLoop = gEngfuncs.GetClientTime() + m_flLoopLength;
		}
	}
}

void ISound::setPlaybackSpeed( float speed )
{
	m_flSpeed = speed;
}

ISound *ISoundEngine::play2D( const char *soundFileName, bool playLooped, bool startPaused, bool track )
{
	const char *rel;
	ISound *sound;

	if( !soundFileName || !*soundFileName )
		return NULL;

	rel = StripGameDir( soundFileName );

	if( !strncasecmp( rel, "sound/", 6 ) || !strncasecmp( rel, "sound\\", 6 ))
	{
		sound = &g_SFXPool[g_iCoFNextSFX];
		g_iCoFNextSFX = ( g_iCoFNextSFX + 1 ) % SFX_POOL;

		if( sound->m_bPlaying && sound->m_bLoop )
			sound->stop();

		*sound = ISound();
		sound->m_iKind = SOUND_SFX;
		strncpy( sound->m_szSample, rel + 6, sizeof( sound->m_szSample ) - 1 );
	}
	else
	{
		sound = &g_Music;
		sound->stop();
		*sound = ISound();
		sound->m_iKind = SOUND_MUSIC;
		strncpy( sound->m_szSample, rel, sizeof( sound->m_szSample ) - 1 );
	}

	sound->m_bLoop = playLooped;

	if( !startPaused )
		sound->setIsPaused( false );

	return sound;
}

void ISoundEngine::setAllSoundsPaused( bool paused )
{
	g_bCoFPaused = paused;
	SetMusicFade( g_flCoFMusicFade );
}

void ISoundEngine::drop( void )
{
	g_Music.stop();
}

// restarts looping sounds that have no cue chunk
void SoundFrame( void )
{
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();
	float now = gEngfuncs.GetClientTime();

	if( !player )
		return;

	for( int i = 0; i < SFX_POOL; i++ )
	{
		ISound *sound = &g_SFXPool[i];

		if( !sound->m_bPlaying || !sound->m_bLoop || sound->m_flLoopLength <= 0.05f )
			continue;

		if( now + 1.0f < sound->m_flNextLoop - sound->m_flLoopLength )
			sound->m_flNextLoop = now + sound->m_flLoopLength;

		if( now < sound->m_flNextLoop )
			continue;

		gEngfuncs.pEventAPI->EV_StopSound( player->index, CHAN_STATIC, sound->m_szSample );
		gEngfuncs.pEventAPI->EV_PlaySound( player->index, player->origin, CHAN_STATIC, sound->m_szSample, VOL_NORM, ATTN_NONE, 0, PITCH_NORM );
		sound->m_flNextLoop = now + sound->m_flLoopLength;
	}
}

ISoundEngine *createIrrKlangDevice( void )
{
	return &g_SoundEngine;
}
}
