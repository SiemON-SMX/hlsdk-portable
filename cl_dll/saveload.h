// ======================================
// ======================================

#ifndef _SAVELOAD_H
#define _SAVELOAD_H
using namespace vgui;

class CSaveLoad : public Panel
{
public:
	CSaveLoad();
	~CSaveLoad( void );
	void Initialize();
	void ActionSignal( int cmd );

	void ReadAllSlots();
	void ReadSlot( int slot );
	void WriteSaveInfo( const char *pszFile );

protected:
	virtual void solve()
	{
		Panel::solve();
	}

public:
	Label	*m_pSlot1Label;
	Label	*m_pSlot2Label;
	Label	*m_pSlot3Label;
	Label	*m_pSlot4Label;
	Label	*m_pSlot5Label;
	ColorButton	*m_pSlot1Button;
	ColorButton	*m_pSlot2Button;
	ColorButton	*m_pSlot3Button;
	ColorButton	*m_pSlot4Button;
	ColorButton	*m_pSlot5Button;
	float	m_flSlot1ClickTime;
	float	m_flSlot2ClickTime;
	float	m_flSlot3ClickTime;
	float	m_flSlot4ClickTime;
	float	m_flSlot5ClickTime;
	char	m_szSaveName[128];
	int		m_iMode;
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
};

class CSaveLoadHandler_Command : public ActionSignal
{
private:
	CSaveLoad	*m_pFather;
	int			m_cmd;

public:
	CSaveLoadHandler_Command( CSaveLoad *panel, int cmd )
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
