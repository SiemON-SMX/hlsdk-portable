// ======================================
// ======================================

#ifndef _PADLOCK_H
#define _PADLOCK_H
using namespace vgui;

class CPadlock : public Panel
{
public:
	CPadlock();
	~CPadlock( void );
	void Initialize();
	void ActionSignal( int cmd );

public:
	Image	*m_pButtonsImage;
	ImagePanel	*m_pButtons;
};

class CPadlockHandler_Command : public ActionSignal
{
private:
	CPadlock	*m_pFather;
	int			m_cmd;

public:
	CPadlockHandler_Command( CPadlock *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void PadlockInit();

#endif
