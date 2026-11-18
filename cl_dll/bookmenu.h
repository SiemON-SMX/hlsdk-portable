// ======================================
// ======================================

#ifndef _BOOKMENU_H
#define _BOOKMENU_H
using namespace vgui;

class CBookMenu : public Panel
{
public:
	CBookMenu();
	~CBookMenu();
	void Initialize();
	void ActionSignal( int cmd );

	virtual void setVisible( bool state );

public:
	Image	*m_pArrowsImage;
	ImagePanel	*m_pArrows;
	Image	*m_pCancelImage;
	ImagePanel	*m_pCancel;
};

class CBookMenuHandler_Command : public ActionSignal
{
private:
	CBookMenu	*m_pFather;
	int			m_cmd;

public:
	CBookMenuHandler_Command( CBookMenu *panel, int cmd )
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
