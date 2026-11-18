// ======================================
// ======================================

#ifndef _USERDOCUMENT_H
#define _USERDOCUMENT_H
using namespace vgui;

class CUserDocument : public CMenuPanel
{
public:
	CUserDocument( int iTrans, int iRemoveMe, int x, int y, int wide, int tall );
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	const char *GetPassword( int index );
	const char *GetUsername( int index );

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
	CImageLabel	*m_pCancelLabel;
	Font	*m_pFont;
	Image	*m_pDocumentImage;
	ImagePanel	*m_pDocument;
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	int		m_iType;
	int		m_iIndex;
};

class CUserDocumentHandler_Command : public ActionSignal
{
private:
	CUserDocument	*m_pFather;
	int				m_cmd;

public:
	CUserDocumentHandler_Command( CUserDocument *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void UserDocInit();

#endif
