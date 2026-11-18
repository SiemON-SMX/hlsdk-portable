// ======================================
// ======================================

#ifndef _PHONE_H
#define _PHONE_H
using namespace vgui;

class CPhone : public Panel
{
public:
	CPhone();
	~CPhone( void );
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	void AddDigit( int digit );
	void Clear();
	void PhoneDial();

	virtual void setVisible( bool state )
	{
		Panel::setVisible( state );
	}

public:
	Image	*m_pPhoneImage;
	ImagePanel	*m_pPhone;
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	char	m_szNumber[16];
	int		m_iUnknownDC;
	Label	*m_pNumberLabel;
};

class CPhoneHandler_Command : public ActionSignal
{
private:
	CPhone	*m_pFather;
	int		m_cmd;

public:
	CPhoneHandler_Command( CPhone *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void PhoneInit();

#endif
