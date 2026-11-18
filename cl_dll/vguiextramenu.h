// ======================================
// ======================================

#ifndef _VGUIEXTRAMENU_H
#define _VGUIEXTRAMENU_H
using namespace vgui;

class CVGUIExtraMenu : public Panel
{
public:
	CVGUIExtraMenu();
	~CVGUIExtraMenu();
	void Initialize();
	void ActionSignal( int cmd );

	virtual void setVisible( bool state );

protected:
	virtual void solve();

public:
	Image	*m_pDonateImage;
	ImagePanel	*m_pDonate;
	Image	*m_pSoundtrack1Image;
	ImagePanel	*m_pSoundtrack1;
	Image	*m_pSoundtrack2Image;
	ImagePanel	*m_pSoundtrack2;
	Image	*m_pClothesImage;
	ImagePanel	*m_pClothes;
	Image	*m_pAoMDCImage;
	ImagePanel	*m_pAoMDC;
	Image	*m_pFacebookImage;
	ImagePanel	*m_pFacebook;
	Image	*m_pCancelImage;
	ImagePanel	*m_pCancel;
	ColorButton	*m_pDonateButton;
	ColorButton	*m_pSoundtrack1Button;
	ColorButton	*m_pSoundtrack2Button;
	ColorButton	*m_pClothesButton;
	ColorButton	*m_pAoMDCButton;
	ColorButton	*m_pFacebookButton;
	ColorButton	*m_pCancelButton;
	bool	m_bDonateArmed;
	bool	m_bSoundtrack1Armed;
	bool	m_bSoundtrack2Armed;
	bool	m_bClothesArmed;
	bool	m_bAoMDCArmed;
	bool	m_bFacebookArmed;
	bool	m_bCancelArmed;
};

class CVGUIExtraMenuHandler_Command : public ActionSignal
{
private:
	CVGUIExtraMenu	*m_pFather;
	int			m_cmd;

public:
	CVGUIExtraMenuHandler_Command( CVGUIExtraMenu *panel, int cmd )
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
