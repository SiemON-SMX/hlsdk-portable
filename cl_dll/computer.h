// ======================================
// ======================================

#ifndef _COMPUTER_H
#define _COMPUTER_H
using namespace vgui;

class CComputer : public Panel
{
public:
	CComputer();
	~CComputer( void );
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	virtual void setVisible( bool state );

protected:
	virtual void solve();

public:
	float	m_flNextKeyTime;
	int		m_iLoggedOn;
	int		m_iUserLength;
	int		m_iPasswordLength;
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	TextEntry	*m_pUser;
	TextEntry	*m_pPassword;
	Image	*m_pScreenImage;
	ImagePanel	*m_pScreen;
	TextPanel	*m_pText;
	int		m_iUnusedE8[4];
};

class CComputerHandler_Command : public ActionSignal
{
private:
	CComputer	*m_pFather;
	int			m_cmd;

public:
	CComputerHandler_Command( CComputer *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void VGUI_ComputerInit();

#endif
