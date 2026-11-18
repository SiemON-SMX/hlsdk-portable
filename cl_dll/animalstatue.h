// ======================================
// ======================================

#ifndef _ANIMALSTATUE_H
#define _ANIMALSTATUE_H
using namespace vgui;

class CAnimalStatue : public Panel
{
public:
	CAnimalStatue();
	~CAnimalStatue();
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

public:
	Panel	*m_pBlackPanel;
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	int		m_iStatue;
	Image	*m_pButtonsImage;
	ImagePanel	*m_pButtons;
};

class CAnimalStatueHandler_Command : public ActionSignal
{
private:
	CAnimalStatue	*m_pFather;
	int				m_cmd;

public:
	CAnimalStatueHandler_Command( CAnimalStatue *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void StatueInit();

#endif
