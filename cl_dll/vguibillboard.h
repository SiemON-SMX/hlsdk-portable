// ======================================
// ======================================

#ifndef _VGUIBILLBOARD_H
#define _VGUIBILLBOARD_H
using namespace vgui;

class CVGUIBillboard : public Panel
{
public:
	CVGUIBillboard();
	~CVGUIBillboard( void );
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	virtual void setVisible( bool state );

protected:
	virtual void paintBackground();
	virtual void paint();

public:
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	ImagePanel	*m_pImage;
	Label	*m_pLabel;
	Font	*m_pFont;
};

class CBillboardHandler_Command : public ActionSignal
{
private:
	CVGUIBillboard	*m_pFather;
	int				m_cmd;

public:
	CBillboardHandler_Command( CVGUIBillboard *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void VGUI_BoardInit();

#endif
