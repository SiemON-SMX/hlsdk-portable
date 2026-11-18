// ======================================
// ======================================

#ifndef _AMMOINVENTORY_H
#define _AMMOINVENTORY_H
using namespace vgui;

#define MAX_AMMO_SLOTS	6

class CAmmoInventory : public Panel
{
public:
	CAmmoInventory();
	~CAmmoInventory( void );
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	virtual void setVisible( bool state );

protected:
	virtual void solve();

public:
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	int		m_iDropMode;
	int		m_iSelected;
	Label	*m_pCountLabel[MAX_AMMO_SLOTS + 1];
	int		m_iAmmoType[MAX_AMMO_SLOTS + 1];
	CImageLabel	*m_pSlotImage[MAX_AMMO_SLOTS + 1];
	CImageLabel	*m_pSelectedImage;
	ColorButton	*m_pButton[MAX_AMMO_SLOTS + 1];
	ColorButton	*m_pDropButton;
	Image	*m_pInventoryImage;
	ImagePanel	*m_pInventory;
};

class CAmmoInventoryHandler_Command : public ActionSignal
{
private:
	CAmmoInventory	*m_pFather;
	int				m_cmd;

public:
	CAmmoInventoryHandler_Command( CAmmoInventory *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void VGUI_AmmoInvInit();

#endif
