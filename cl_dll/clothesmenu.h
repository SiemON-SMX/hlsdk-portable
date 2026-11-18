// ======================================
// ======================================

#ifndef _CLOTHESMENU_H
#define _CLOTHESMENU_H
using namespace vgui;

#define MAX_CLOTHES	13

class CClothesMenu : public Panel
{
public:
	CClothesMenu();
	~CClothesMenu( void );
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	void UpdateButtons();

public:
	float	m_flNextClick;
	ColorButton	*m_pClothesButton[MAX_CLOTHES];
	int		m_iUnlocked;
};

class CClothesMenuHandler_Command : public ActionSignal
{
private:
	CClothesMenu	*m_pFather;
	int			m_cmd;

public:
	CClothesMenuHandler_Command( CClothesMenu *panel, int cmd )
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
