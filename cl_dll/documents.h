// ======================================
// ======================================

#ifndef _DOCUMENTS_H
#define _DOCUMENTS_H
using namespace vgui;

#define MAX_DOCUMENTS	16

class CDocuments : public Panel
{
public:
	CDocuments();
	~CDocuments();
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	virtual void setVisible( bool state );

public:
	int		m_iHidden[MAX_DOCUMENTS + 1];
	CommandButton	*m_pButton[MAX_DOCUMENTS + 1];
	Image	*m_pDocumentsImage;
	ImagePanel	*m_pDocuments;
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	ImagePanel	*m_pCodeDigit1[MAX_DOCUMENTS + 1];
	ImagePanel	*m_pCodeDigit2[MAX_DOCUMENTS + 1];
	ImagePanel	*m_pCodeDigit3[MAX_DOCUMENTS + 1];
	ImagePanel	*m_pCodeDigit4[MAX_DOCUMENTS + 1];
	Image	*m_pHiddenNoteImage[MAX_DOCUMENTS + 1];
	ImagePanel	*m_pNote[MAX_DOCUMENTS + 1];
};

class CDocumentsHandler_Command : public ActionSignal
{
private:
	CDocuments	*m_pFather;
	int			m_cmd;

public:
	CDocumentsHandler_Command( CDocuments *panel, int cmd )
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
