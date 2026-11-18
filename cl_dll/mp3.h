// mp3 support added by Killar

#ifndef MP3_H
#define MP3_H

class CMP3
{
public:
	CMP3();

	void	Disabled( void );
	void	Frame( void );
	int		Initialize( void );
	int		PlayMP3( const char *pszSong );
	int		PlayMP3Path( const char *pszPath );
	void	PlaySFX( const char *pszSample, int iType, bool bFlag, Vector vecOrigin );
	int		SetPaused( bool bPaused );
	int		Shutdown( void );
	int		StopMP3( float fade );

	bool	m_bInitialized;
	cl_entity_t	*m_pLocalPlayer;
	Vector	m_vecUnknown08;
	Vector	m_vecUnknown14;
	Vector	m_vecUnknown20;
	int		m_iUnknown2C;
	int		m_iUnknown30;
	bool	m_bUnknown34;
	bool	m_bUnknown35;

	// buz: mp3 file name:
	char	m_szMP3File[128];
	int		m_iIsPlaying;
	int		m_iIsPaused;

	int		m_iVolume;
	bool	m_bHold;
	const char	*m_pszPending;

	// buz: for fadeout:
	float	m_flFadeoutStart;
	float	m_flFadeoutDuration;
	float	m_flFadeinStart;
	float	m_flFadeinDuration;

	bool	m_bLoop;
	bool	m_bStopping;
};

void IrrKlangCreate( void );
void IrrKlangDestroy( void );
void IrrKlangFrame( void );
void IrrKlangInitialize( void );
bool IrrKlangIsPaused( void );
void IrrKlangPlayMP3( const char *pszFile );
void IrrKlangPlaySFX( const char *pszFile, int iType, bool bFlag, vec3_t vecOrigin );
void IrrKlangSetFade( float flParm1, float flParm2, float flParm3, float flParm4 );
void IrrKlangSetLoop( bool bLoop );
void IrrKlangSetPaused( bool bPaused );
void IrrKlangSetStopping( bool bStopping );
void IrrKlangStop( float flFadeTime );

#endif
