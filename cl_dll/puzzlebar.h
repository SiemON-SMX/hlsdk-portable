// ======================================
// ======================================

#ifndef _PUZZLEBAR_H
#define _PUZZLEBAR_H
using namespace vgui;

class CPuzzleBar : public Panel
{
public:
	CPuzzleBar();
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	virtual void setVisible( bool state );

public:
	Image	*m_pImage;
	ImagePanel	*m_pImagePanel;
};

class CPuzzleBarHandler_Command : public ActionSignal
{
private:
	CPuzzleBar	*m_pFather;
	int			m_cmd;

public:
	CPuzzleBarHandler_Command( CPuzzleBar *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void PuzzlebarInit();

#endif
