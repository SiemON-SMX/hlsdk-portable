// ======================================
// ======================================

#ifndef _LANGUAGE_H
#define _LANGUAGE_H
using namespace vgui;

class CLanguage : public Panel
{
public:
	CLanguage();
	~CLanguage( void );
	void Initialize();
	void ActionSignal( int cmd );

public:
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
};

class CLanguageHandler_Command : public ActionSignal
{
private:
	CLanguage	*m_pFather;
	int			m_cmd;

public:
	CLanguageHandler_Command( CLanguage *panel, int cmd )
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
