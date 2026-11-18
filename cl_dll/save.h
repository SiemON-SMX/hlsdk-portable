// ======================================
// ======================================

#ifndef _SAVE_H
#define _SAVE_H
using namespace vgui;

class CSave : public Panel
{
public:
	CSave();
	~CSave( void );
	void Initialize();
	void ActionSignal( int cmd );

public:
	Image	*m_pImage;
	ImagePanel	*m_pImagePanel;
};

class CSaveHandler_Command : public ActionSignal
{
private:
	CSave	*m_pFather;
	int		m_cmd;

public:
	CSaveHandler_Command( CSave *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void SaveInit();

#endif
