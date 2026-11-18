// ======================================
// ======================================

#ifndef _UNLOCKABLES_H
#define _UNLOCKABLES_H
using namespace vgui;

#define MAX_UNLOCKABLES	22

class CUnlockables : public Panel
{
public:
	CUnlockables();
	~CUnlockables();
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	virtual void setVisible( bool state );

protected:
	virtual void solve();

public:
	int		m_iUnknownBC;
	int		m_iArmed;
	float	m_flNextClick;
	ColorButton	*m_pItemButton[MAX_UNLOCKABLES];
	Label	*m_pTextLabel;
	ColorButton	*m_pBackButton;
	Image	*m_pLockedImage[MAX_UNLOCKABLES];
	ImagePanel	*m_pLockedPanel[MAX_UNLOCKABLES];
	Image	*m_pMenuImage;
	ImagePanel	*m_pMenu;
};

class CUnlockablesHandler_Command : public ActionSignal
{
private:
	CUnlockables	*m_pFather;
	int			m_cmd;

public:
	CUnlockablesHandler_Command( CUnlockables *panel, int cmd )
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
