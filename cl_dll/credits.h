// ======================================
// ======================================

#ifndef _CREDITS_H
#define _CREDITS_H
using namespace vgui;

#define MAX_CREDIT_LINES	255

class CCredits : public Panel
{
public:
	CCredits();
	~CCredits( void );
	void Initialize();

	int CountLines();
	void SetLine( int iUnused, int iLine );
	void ShowUnlock( int iUnlock, const char *pszText, int iDistance, char *pszModel, int iBody );

protected:
	virtual void solve();

public:
	bool	m_bShowStats;
	bool	m_bTextOnly;
	bool	m_bContinue;
	int		m_iUnknownC0;
	Panel	*m_pLetterboxTop;
	Panel	*m_pLetterboxBottom;
	char	m_szUnlockText[256];
	bool	m_bUnlocks;
	int		m_iUnlocks;
	int		m_iScrollSpeed;
	int		m_iUnknown1D8[4];
	int		m_iShotsFired;
	int		m_iShotsHit;
	int		m_iSyringesUsed;
	int		m_iShotsMissed;
	float	m_flAccuracy;
	int		m_iTimesSaved;
	int		m_iWeaponsFound;
	int		m_iDamageTaken;
	float	m_flDistanceDecimal;
	float	m_flDistance;
	int		m_iSeconds;
	int		m_iMinutes;
	int		m_iHours;
	int		m_iMeleeKills;
	int		m_iFirearmKills;
	int		m_iDifficulty;
	Label	*m_pStatsTitleLabel;
	Label	*m_pGradeLabel;
	Label	*m_pGradeShadowLabel;
	Label	*m_pStatLabel[15];
	int		m_iUnknown270[17];
	Label	*m_pUnlockLabel;
	bool	m_bStatsTimer;
	float	m_flStatsTime;
	char	m_szFile[256];
	Label	*m_pCreditLabel[MAX_CREDIT_LINES + 1];
	int		m_iNumLines;
	bool	m_bStarted;
	bool	m_bScrollUp;
	int		m_iBrightness;
	bool	m_bFadeIn;
	bool	m_bFadeDone;
	Font	*m_pCreditsFont;
	Font	*m_pCreditsTitleFont;
	float	m_flStartTime;
};

extern bool g_bUnlockShown;

char *UTIL_VarArgs( const char *format, ... );

#endif
