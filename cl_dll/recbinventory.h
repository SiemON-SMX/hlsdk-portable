// ======================================
// ======================================

#ifndef _RECBINVENTORY_H
#define _RECBINVENTORY_H
using namespace vgui;

class CRECBInventory : public CMenuPanel
{
public:
	CRECBInventory( int iTrans, int iRemoveMe, int x, int y, int wide, int tall );
	void Initialize();
	void ActionSignal( int cmd );
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

	void ClearItems();
	void DoAction( int slot );
	bool MouseOver( int x, int y, int wide, int tall );
	void SelectItem( int slot );
	void SetAction( int action );
	void Update();

	virtual void setVisible( bool state );

protected:
	virtual void solve();

public:
	Image	*m_pBackgroundImage;
	ImagePanel	*m_pBackground;
	CImageLabel	*m_pMenuImage;
	bool	m_bUnknownE0;
	int		m_iUnknownE4;
	int		m_iDualWield;
	int		m_iCombine;
	int		m_iSelected;
	int		m_iSelectionAlpha;
	Panel	*m_pSelection;
	int		m_iUnusedFC;
	ColorButton	*m_pMenuButtons[7];
	int		m_iUnused11C;
	CImageLabel	*m_pItemImages[10];
	Panel	*m_pBackgroundPanel;
	Image	*m_pInventoryImage;
	ImagePanel	*m_pInventory;
	CImageLabel	*m_pPopupImage;
	int		m_iUnused158;
	ColorButton	*m_pItemButtons[6];
	int		m_iUnused174[3];
	ColorButton	*m_pOptionButtons[5];
	ColorButton	*m_pQuickSlot1Button;
	ColorButton	*m_pQuickSlot2Button;
	ColorButton	*m_pQuickSlot3Button;
	Label	*m_pObjectiveLabel;
	int		m_iThree;
	int		m_iUnused1A8[2];
	int		m_iAction;
	int		m_iUnused1B4;
	int		m_iActionItem1;
	int		m_iActionItem2;
	int		m_iHoverItem;
};

class CRECBInventoryHandler_Command : public ActionSignal
{
private:
	CRECBInventory	*m_pFather;
	int				m_cmd;

public:
	CRECBInventoryHandler_Command( CRECBInventory *panel, int cmd )
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed( Panel *panel )
	{
		m_pFather->ActionSignal( m_cmd );
	}
};

void VGUI_QSlotInit();

#endif
