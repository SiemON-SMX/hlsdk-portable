// ======================================
// ======================================

#ifndef _TELEPHONE_H
#define _TELEPHONE_H
using namespace vgui;

class CTelephone : public Panel
{
public:
	CTelephone();
	~CTelephone( void );
	void Initialize();
	void ActionSignal( int cmd );

	void AddDigit( int digit );
	void Dial();

	virtual void setVisible( bool state );

protected:
	virtual void paint();

public:
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	bool	m_bLocked;
	float	m_flFade;
	char	m_szNumber[16];
	char	m_szDialed[16];
	Image	*m_pTelephoneImage;
	ImagePanel	*m_pTelephone;
};

class CTelephoneHandler_Command : public ActionSignal
{
private:
	CTelephone	*m_pFather;
	int			m_cmd;

public:
	CTelephoneHandler_Command( CTelephone *panel, int cmd )
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
