// ======================================
// ======================================

#ifndef _HUDCONTROL_H
#define _HUDCONTROL_H
using namespace vgui;

class CHUDControl : public Panel
{
public:
	CHUDControl();
	~CHUDControl();
	void Initialize();

	void HandleHUDVisibility();
	void LogoUpdate();
	void RenderMessages();
	void ShowSMSIcon( bool bShow );
	void UpdateOnPlayerInfo();

protected:
	virtual void solve();
	virtual void paintBackground();
	virtual void paint();

public:
	int		m_iMessageType;
	int		m_iMessageTime;
	int		m_iMessageC4;
	int		m_iMessageC8;
	int		m_iMessageCC;
	int		m_iMessageD0;
	char	m_szMessage[256];
	bool	m_bShowLogo;
	char	m_szMessage2[256];
	int		m_iDevDonator[5];
	Label	*m_pTimerPlusLabel[32];
	float	m_flTimerPlusTime;
	int		m_iTimerLimit;
	bool	m_bTimerPaused;
	Panel	*m_pLetterboxTop;
	Panel	*m_pLetterboxBottom;
	Panel	*m_pOverlay;
	Label	*m_pVoteLabel;
	Label	*m_pLobbyLabel[4];
	Label	*m_pVoteKickLabel;
	bool	m_bLobby;
	bool	m_bPlayerReady[4];
	Label	*m_pTimerLabel;
	Label	*m_pMessageLabel;
	Label	*m_pMessageLabel2;
	Label	*m_pHintLabel;
	Label	*m_pSayTextLabel[5];
	Label	*m_pUnused3C8;
	Label	*m_pLookingAtLabel;
	Label	*m_pPlayerLabel[4];
	Label	*m_pDeathLabel;
	float	m_flLookingAtTime;
	ImagePanel	*m_pLobbyBackground;
	int		m_iDeathAlpha;
	bool	m_bTimer;
	float	m_flTimerNextSecond;
	int		m_iTimerSeconds;
	int		m_iTimerMinutes;
	int		m_iTimerHours;
	char	m_szHintLine1[256];
	char	m_szHintLine2[256];
	char	m_szHintLine3[256];
	char	m_szHintNoButton[256];
	char	m_szHintCfgButton[64];
	int		m_iHintLine;
	bool	m_bHint;
	int		m_iHintAlpha;
	float	m_flHintTime;
	int		m_iUnused854;
	int		m_iUnused858;
	int		m_iSayTextX;
	int		m_iSayTextY;
	char	m_szUnused864[6][256];
	ImagePanel	*m_pIntroduction;
	Image	*m_pIntroductionImage;
	bool	m_bIntroductionFadeIn;
	int		m_iIntroductionAlpha;
	ImagePanel	*m_pHintBar;
	Image	*m_pHintBarImage;
	ImagePanel	*m_pVignetteTL;
	ImagePanel	*m_pVignetteTR;
	ImagePanel	*m_pVignetteBL;
	ImagePanel	*m_pVignetteBR;
	Panel	*m_pBlackScreen;
	Font	*m_pPagerFont;
	Font	*m_pChapterTitleFont;
	char	m_szChapter[128];
	float	m_flMessageAlpha;
	bool	m_bMessageFadeOut;
	bool	m_bMessageHold;
	bool	m_bChapterF1E;
	float	m_flMessageHoldTime;
	float	m_flMessageF24;
	int		m_iMessageChars;
	float	m_flChapterAlpha;
	bool	m_bChapterFadeOut;
	bool	m_bChapterHold;
	float	m_flChapterHoldTime;
	float	m_flChapterNextChar;
	int		m_iChapterChars;
	int		m_iChapterType;
	float	m_flHintSlideInTime;
	float	m_flHintSlideInLength;
	float	m_flHintSlideOutTime;
	float	m_flHintSlideOutLength;
	Font	*m_pCreditsFont;
	int		m_iLogoAlpha;
	bool	m_bLogoF5C;
	bool	m_bLogoF5D;
	float	m_flLogoTime;
	int		m_iLogo;
	Image	*m_pFollowIcon[4];
	ImagePanel	*m_pFollowIconPanel[4];
	Image	*m_pSyringeIcon[4];
	ImagePanel	*m_pSyringeIconPanel[4];
	int		m_iUnusedFA8[5];
	float	m_flZoomFOV;
	float	m_flZoomLength;
	float	m_flZoomTime;
	ImagePanel	*m_pHealthBar;
	ImagePanel	*m_pHealthBarFull;
	ImagePanel	*m_pHealthBarEmpty;
	ImagePanel	*m_pLogo;
	ImagePanel	*m_pStaminaBar;
	ImagePanel	*m_pStaminaBarFull;
	ImagePanel	*m_pStaminaBarEmpty;
	float	m_flStaminaFlashTime;
	Panel	*m_pStaminaFlash;
	Image	*m_pLogoImage;
	ImagePanel	*m_pSMSIcon;
	bool	m_bSMSIcon;
	float	m_flSMSIconTime;
	int		m_iSMSIconAlpha;
	int		m_iStamina;
	ImagePanel	*m_pInventoryHint;
	bool	m_bInventoryHint;
	float	m_flInventoryHintTime;
	int		m_iUnused1010;
	int		m_iInventoryHintY;
	Image	*m_pSMSIconImage;
	Image	*m_pInventoryHintImage;
	Image	*m_pCrosshairImage;
	Image	*m_pHealthBarImage;
	Image	*m_pHealthBarFullImage;
	Image	*m_pHealthBarEmptyImage;
	Image	*m_pStaminaBarImage;
	Image	*m_pStaminaBarFullImage;
	Image	*m_pStaminaBarEmptyImage;
	int		m_iUnused103C[4];
};

void HUDControlInit();
void ParseHintTokens( char *pszName, char *pszFile );

#endif
