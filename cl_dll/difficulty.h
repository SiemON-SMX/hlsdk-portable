// ======================================
// ======================================

#ifndef _DIFFICULTY_H
#define _DIFFICULTY_H
using namespace vgui;

class CDifficulty : public Panel, public ICheckButton2Handler
{
public:
	CDifficulty();
	~CDifficulty();
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	virtual void setVisible( bool state );

	virtual void StateChanged( CCheckButton2 *pButton );

public:
	Image	*m_pImage;
	ImagePanel	*m_pImagePanel;
	ColorButton	*m_pSkill4Button;
	CCheckButton2	m_CommentaryButton;
};

class CDifficultyHandler_Command : public ActionSignal
{
private:
	CDifficulty	*m_pFather;
	int			m_cmd;

public:
	CDifficultyHandler_Command( CDifficulty *panel, int cmd )
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
