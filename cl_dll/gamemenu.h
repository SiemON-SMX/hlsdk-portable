// ======================================
// ======================================

#ifndef _GAMEMENU_H
#define _GAMEMENU_H
using namespace vgui;

#define MAX_CAMPAIGNS	256

typedef struct
{
	char	szFirstMap[64];
	char	szDescription[64];
	char	szAuthor[64];
	bool	bValid;
} campaign_t;

class CCampaignSlider : public Slider
{
public:
	CCampaignSlider( int x, int y, int wide, int tall, bool vertical ) : Slider( x, y, wide, tall, vertical )
	{
	}

	virtual void paintBackground();
};

class CGameMenu : public Panel
{
public:
	CGameMenu();
	~CGameMenu( void );
	void Initialize();
	void ActionSignal( int cmd );

	void ParseCampaign( char *pszFile, int index );
	void ReadCampaigns();
	void ResetButtonPositions();

	virtual void setVisible( bool state );

protected:
	virtual void solve();

public:
	Label	*m_pVersionLabel;
	bool	m_bUnknownC0;
	campaign_t	m_Campaigns[MAX_CAMPAIGNS + 1];
	ColorButton	*m_pCampaignButton[MAX_CAMPAIGNS];
	int		m_iUnknownC684;
	int		m_iUnknownC688;
	Label	*m_pAuthorLabel;
	Font	*m_pCreditsFont;
	int		m_iArmedCampaign;
	bool	m_bCampaignArmed;
	bool	m_bNewGameArmed;
	bool	m_bLoadGameArmed;
	bool	m_bCustomArmed;
	bool	m_bJoinServerArmed;
	bool	m_bOptionsArmed;
	bool	m_bLanguageArmed;
	bool	m_bUnlockablesArmed;
	bool	m_bExtrasArmed;
	bool	m_bHostServerArmed;
	bool	m_bQuitArmed;
	char	*m_pszCampaign;
	int		m_iArmedX;
	int		m_iArmedY;
	CTFScrollPanel	*m_pCampaignList;
	ColorButton	*m_pNewGameButton;
	ColorButton	*m_pLoadGameButton;
	ColorButton	*m_pCustomButton;
	ColorButton	*m_pJoinServerButton;
	ColorButton	*m_pOptionsButton;
	ColorButton	*m_pLanguageButton;
	ColorButton	*m_pUnlockablesButton;
	ColorButton	*m_pExtrasButton;
	ColorButton	*m_pHostServerButton;
	ColorButton	*m_pQuitButton;
	ColorButton	*m_pCancelButton;
	Image	*m_pLogoImage;
	ImagePanel	*m_pLogo;
	Image	*m_pLogoTSImage;
	ImagePanel	*m_pLogoTS;
	Image	*m_pNewGameImage;
	ImagePanel	*m_pNewGame;
	Image	*m_pLoadGameImage;
	ImagePanel	*m_pLoadGame;
	Image	*m_pCustomImage;
	ImagePanel	*m_pCustom;
	Image	*m_pJoinServerImage;
	ImagePanel	*m_pJoinServer;
	Image	*m_pOptionsImage;
	ImagePanel	*m_pOptions;
	Image	*m_pLanguageImage;
	ImagePanel	*m_pLanguage;
	Image	*m_pUnlockablesImage;
	ImagePanel	*m_pUnlockables;
	Image	*m_pExtrasImage;
	ImagePanel	*m_pExtras;
	Image	*m_pHostServerImage;
	ImagePanel	*m_pHostServer;
	Image	*m_pQuitImage;
	ImagePanel	*m_pQuit;
	int		m_iUnknownC740;
	int		m_iUnknownC744;
};

class CGameMenuHandler_Command : public ActionSignal
{
private:
	CGameMenu	*m_pFather;
	int			m_cmd;

public:
	CGameMenuHandler_Command( CGameMenu *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

#endif
