#pragma once
#if !defined(IRRKLANG_H)
#define IRRKLANG_H

namespace irrklang
{
class ISound
{
public:
	ISound();
	void setVolume( float volume );
	void stop( void );
	void setIsPaused( bool paused );
	void setPlaybackSpeed( float speed );

	int m_iKind;
	bool m_bLoop;
	bool m_bPlaying;
	float m_flVolume;
	float m_flSpeed;
	float m_flLoopLength;
	float m_flNextLoop;
	char m_szSample[256];
};

class ISoundEngine
{
public:
	ISound *play2D( const char *soundFileName, bool playLooped, bool startPaused, bool track );
	void setAllSoundsPaused( bool paused );
	void drop( void );
};

ISoundEngine *createIrrKlangDevice( void );
void SoundFrame( void );
}

#endif
