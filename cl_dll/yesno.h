// ======================================
// ======================================

#ifndef _YESNO_H
#define _YESNO_H
using namespace vgui;

class CYesno : public Panel
{
public:
	CYesno();
	~CYesno( void );
	void Initialize();
	void ActionSignal( int cmd );

public:
	int		m_iActivate;
	Image	*m_pImage;
	ImagePanel	*m_pImagePanel;
	CommandButton	*m_pYesButton;
	CommandButton	*m_pNoButton;
};

class CYesnoHandler_Command : public ActionSignal
{
private:
	CYesno	*m_pFather;
	int		m_cmd;

public:
	CYesnoHandler_Command( CYesno *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void YesNoInit();

#endif
