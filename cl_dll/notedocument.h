// ======================================
// ======================================

#ifndef _NOTEDOCUMENT_H
#define _NOTEDOCUMENT_H
using namespace vgui;

class CNoteDocument : public CMenuPanel
{
public:
	CNoteDocument( int iTrans, int iRemoveMe, int x, int y, int wide, int tall );
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	void ClearString( char *str );
	int CountLines();
	void NextLine();
	void NextPage();
	void PrevLine();
	void PrevPage();
	void SetPage( int page );
	void SetText();

	virtual void setVisible( bool state )
	{
		Panel::setVisible( state );
	}

protected:
	virtual void paintBackground()
	{
		Panel::paintBackground();
	}

	virtual void paint();

public:
	CommandButton	*m_pCloseButton;
	CTransparentPanel	*m_pPanel;
	CImageLabel	*m_pImageLabel;
	CImageLabel	*m_pCancelLabel;
	int		m_iLine;
	int		m_iPage;
	char	m_szImage1[16];
	char	m_szImage2[16];
	char	m_szImage3[16];
	char	m_szImage4[16];
	char	m_szSound[48];
	char	m_szText1[16];
	char	m_szText2[16];
	char	m_szText3[16];
	char	m_szText4[16];
	char	m_szLine[128];
	bool	m_bTitle;
	Label	*m_pLineLabel;
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	Font	*m_pFont;
};

class CNoteDocumentHandler_Command : public ActionSignal
{
private:
	CNoteDocument	*m_pFather;
	int				m_cmd;

public:
	CNoteDocumentHandler_Command( CNoteDocument *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void NoteDocInit();

#endif
