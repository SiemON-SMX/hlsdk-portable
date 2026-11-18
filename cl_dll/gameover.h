// ======================================
// ======================================

#ifndef _GAMEOVER_H
#define _GAMEOVER_H
using namespace vgui;

class CGameOver : public Panel
{
public:
	CGameOver();
	~CGameOver();
	void Initialize();
	void ActionSignal( int cmd );

	virtual void setVisible( bool state );

protected:
	virtual void solve();

public:
	Image	*m_pImage;
	ImagePanel	*m_pPanel;
	Image	*m_pLoadImage;
	ImagePanel	*m_pLoad;
	Image	*m_pExitImage;
	ImagePanel	*m_pExit;
	ColorButton	*m_pLoadButton;
	ColorButton	*m_pExitButton;
	char	m_szCustomDoc[256];
	float	m_flAlpha;
	bool	m_bLoadArmed;
	bool	m_bExitArmed;
};

class CGameOverHandler_Command : public ActionSignal
{
private:
	CGameOver	*m_pFather;
	int			m_cmd;

public:
	CGameOverHandler_Command( CGameOver *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void VGUI_CustomDocInit();

#endif
