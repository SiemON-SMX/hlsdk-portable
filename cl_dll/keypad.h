// ======================================
// ======================================

#ifndef _KEYPAD_H
#define _KEYPAD_H
using namespace vgui;

class CKeypad : public Panel
{
public:
	CKeypad();
	~CKeypad();
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	void Activate();
	void EnterKey( int key );
	void Submit();
	void UpdateLayout();

protected:
	virtual void paintBackground()
	{
		Panel::paintBackground();
	}

public:
	Panel	*m_pBackgroundPanel;
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	int		m_iActivated;
	int		m_iType;
	char	m_szCode[8];
	Font	*m_pFont;
	Label	*m_pCodeLabel;
	CommandButton	*m_pKey1;
	CommandButton	*m_pKey2;
	CommandButton	*m_pKey3;
	CommandButton	*m_pKey4;
	CommandButton	*m_pKey5;
	CommandButton	*m_pKey6;
	CommandButton	*m_pKey7;
	CommandButton	*m_pKey8;
	CommandButton	*m_pKey9;
	Image	*m_pPanelBackImage;
	ImagePanel	*m_pPanelBack;
	Image	*m_pPanel2BackImage;
	ImagePanel	*m_pPanel2Back;
	Image	*m_pPanelOffImage;
	ImagePanel	*m_pPanelOff;
	Image	*m_pPanelOnImage;
	ImagePanel	*m_pPanelOn;
};

class CKeypadHandler_Command : public ActionSignal
{
private:
	CKeypad	*m_pFather;
	int		m_cmd;

public:
	CKeypadHandler_Command( CKeypad *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void VGUI_KeypadInit();

#endif
