// ======================================
// ======================================

#include <stdint.h>
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "recbinventory.h"
#include "documents.h"
#include "hudcontrol.h"
#include "../game_shared/vgui_loadtga.h"

extern intptr_t iWheelchair;

bool g_bInventoryFadeOut;

void InventorySound( char *pszSound, float flVolume )
{
	gEngfuncs.pfnPlaySoundByName( pszSound, flVolume );
}

CRECBInventory::CRECBInventory( int iTrans, int iRemoveMe, int x, int y, int wide, int tall ) : CMenuPanel( iTrans, iRemoveMe, x, y, wide, tall )
{
	int iWide, iTall;
	int iX, iY;
	int i;

	m_pBackgroundPanel = new Panel( 0, 0, ScreenWidth, ScreenHeight );
	m_pBackgroundPanel->setParent( this );
	m_pBackgroundPanel->setBgColor( 0, 0, 0, 0 );

	m_iThree = 0;

	m_pBackgroundImage = vgui_LoadTGA( "gfx/vgui/backgroundtoall.tga" );
	if ( m_pBackgroundImage )
		m_pBackgroundImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/backgroundtoall.tga!\n" );

	m_pBackgroundImage->getSize( iWide, iTall );
	iX = ScreenWidth / 2 - iWide / 2;
	iY = ScreenHeight / 2 - iTall / 2;

	m_pBackground = new ImagePanel( m_pBackgroundImage );
	m_pBackground->setParent( this );
	m_pBackground->setPos( iX, iY );
	m_pBackground->setVisible( true );

	iX = ScreenWidth / 2 - 374;
	iY = ScreenHeight / 2 - 240;

	m_pInventoryImage = vgui_LoadTGA( "gfx/vgui/640_inventory.tga" );
	if ( m_pInventoryImage )
		m_pInventoryImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_inventory.tga!\n" );

	m_pInventory = new ImagePanel( m_pInventoryImage );
	m_pInventory->setParent( this );
	m_pInventory->setPos( iX, iY );
	m_pInventory->setVisible( true );

	m_pItemImages[0] = new CImageLabel( NULL, 31, 68 );
	m_pItemImages[0]->setVisible( true );
	m_pItemImages[0]->setParent( m_pInventory );
	m_pItemImages[0]->setImage( NULL );

	m_pItemImages[1] = new CImageLabel( NULL, 133, 68 );
	m_pItemImages[1]->setVisible( true );
	m_pItemImages[1]->setParent( m_pInventory );
	m_pItemImages[1]->setImage( NULL );

	m_pItemImages[2] = new CImageLabel( NULL, 235, 68 );
	m_pItemImages[2]->setVisible( true );
	m_pItemImages[2]->setParent( m_pInventory );
	m_pItemImages[2]->setImage( NULL );

	m_pItemImages[3] = new CImageLabel( NULL, 31, 227 );
	m_pItemImages[3]->setVisible( true );
	m_pItemImages[3]->setParent( m_pInventory );
	m_pItemImages[3]->setImage( NULL );

	m_pItemImages[4] = new CImageLabel( NULL, 133, 227 );
	m_pItemImages[4]->setVisible( true );
	m_pItemImages[4]->setParent( m_pInventory );
	m_pItemImages[4]->setImage( NULL );

	m_pItemImages[5] = new CImageLabel( NULL, 235, 227 );
	m_pItemImages[5]->setVisible( true );
	m_pItemImages[5]->setParent( m_pInventory );
	m_pItemImages[5]->setImage( NULL );

	m_pItemImages[6] = new CImageLabel( NULL, 398, 68 );
	m_pItemImages[6]->setVisible( true );
	m_pItemImages[6]->setParent( m_pInventory );
	m_pItemImages[6]->setImage( NULL );

	m_pItemImages[7] = new CImageLabel( NULL, 500, 68 );
	m_pItemImages[7]->setVisible( true );
	m_pItemImages[7]->setParent( m_pInventory );
	m_pItemImages[7]->setImage( NULL );

	m_pItemImages[8] = new CImageLabel( NULL, 31, 348 );
	m_pItemImages[8]->setVisible( true );
	m_pItemImages[8]->setParent( m_pInventory );
	m_pItemImages[8]->setImage( NULL );

	m_pItemImages[9] = new CImageLabel( NULL, 602, 68 );
	m_pItemImages[9]->setVisible( true );
	m_pItemImages[9]->setParent( m_pInventory );
	m_pItemImages[9]->setImage( NULL );

	m_pItemButtons[0] = new ColorButton( "", 36, 73, 99, 99, false, false );
	m_pItemButtons[0]->setParent( m_pInventory );
	m_pItemButtons[0]->addActionSignal( new CRECBInventoryHandler_Command( this, 1 ) );

	m_pItemButtons[1] = new ColorButton( "", 138, 73, 99, 99, false, false );
	m_pItemButtons[1]->setParent( m_pInventory );
	m_pItemButtons[1]->addActionSignal( new CRECBInventoryHandler_Command( this, 2 ) );

	m_pItemButtons[2] = new ColorButton( "", 240, 73, 99, 99, false, false );
	m_pItemButtons[2]->setParent( m_pInventory );
	m_pItemButtons[2]->addActionSignal( new CRECBInventoryHandler_Command( this, 3 ) );

	m_pItemButtons[3] = new ColorButton( "", 36, 232, 99, 99, false, false );
	m_pItemButtons[3]->setParent( m_pInventory );
	m_pItemButtons[3]->addActionSignal( new CRECBInventoryHandler_Command( this, 4 ) );

	m_pItemButtons[4] = new ColorButton( "", 138, 232, 99, 99, false, false );
	m_pItemButtons[4]->setParent( m_pInventory );
	m_pItemButtons[4]->addActionSignal( new CRECBInventoryHandler_Command( this, 5 ) );

	m_pItemButtons[5] = new ColorButton( "", 240, 232, 99, 99, false, false );
	m_pItemButtons[5]->setParent( m_pInventory );
	m_pItemButtons[5]->addActionSignal( new CRECBInventoryHandler_Command( this, 6 ) );

	m_pOptionButtons[0] = new ColorButton( "", 373, 226, 363, 47, false, false );
	m_pOptionButtons[0]->setParent( m_pInventory );
	m_pOptionButtons[0]->addActionSignal( new CRECBInventoryHandler_Command( this, 7 ) );
	m_pOptionButtons[0]->setVisible( false );

	m_pOptionButtons[1] = new ColorButton( "", 373, 276, 363, 47, false, false );
	m_pOptionButtons[1]->setParent( m_pInventory );
	m_pOptionButtons[1]->addActionSignal( new CRECBInventoryHandler_Command( this, 8 ) );
	m_pOptionButtons[1]->setVisible( false );

	m_pOptionButtons[2] = new ColorButton( "", 373, 325, 363, 47, false, false );
	m_pOptionButtons[2]->setParent( m_pInventory );
	m_pOptionButtons[2]->addActionSignal( new CRECBInventoryHandler_Command( this, 10 ) );
	m_pOptionButtons[2]->setVisible( false );

	m_pOptionButtons[3] = new ColorButton( "", 373, 376, 363, 47, false, false );
	m_pOptionButtons[3]->setParent( m_pInventory );
	m_pOptionButtons[3]->addActionSignal( new CRECBInventoryHandler_Command( this, 15 ) );
	m_pOptionButtons[3]->setVisible( false );

	m_pOptionButtons[4] = new ColorButton( "", 373, 226, 363, 97, false, false );
	m_pOptionButtons[4]->setParent( m_pInventory );
	m_pOptionButtons[4]->addActionSignal( new CRECBInventoryHandler_Command( this, 16 ) );

	for ( i = 0; i < 6; i++ )
	{
		m_pItemButtons[i]->setUnArmedBorderColor( 0, 0, 0, 255 );
		m_pItemButtons[i]->setArmedBorderColor( 255, 255, 255, 128 );
		m_pItemButtons[i]->setUnArmedColor( 0, 0, 0, 255 );
		m_pItemButtons[i]->setArmedColor( 0, 0, 0, 255 );
	}

	for ( i = 0; i < 5; i++ )
	{
		m_pOptionButtons[i]->setUnArmedBorderColor( 0, 0, 0, 255 );
		m_pOptionButtons[i]->setArmedBorderColor( 255, 255, 255, 128 );
		m_pOptionButtons[i]->setUnArmedColor( 0, 0, 0, 255 );
		m_pOptionButtons[i]->setArmedColor( 0, 0, 0, 255 );
	}

	m_pQuickSlot1Button = new ColorButton( "", 403, 73, 99, 99, false, false );
	m_pQuickSlot1Button->setParent( m_pInventory );
	m_pQuickSlot1Button->addActionSignal( new CRECBInventoryHandler_Command( this, 12 ) );
	m_pQuickSlot1Button->setUnArmedBorderColor( 0, 0, 0, 255 );
	m_pQuickSlot1Button->setArmedBorderColor( 255, 255, 255, 255 );
	m_pQuickSlot1Button->setUnArmedColor( 0, 0, 0, 255 );
	m_pQuickSlot1Button->setArmedColor( 0, 0, 0, 255 );

	m_pQuickSlot2Button = new ColorButton( "", 505, 73, 99, 99, false, false );
	m_pQuickSlot2Button->setParent( m_pInventory );
	m_pQuickSlot2Button->addActionSignal( new CRECBInventoryHandler_Command( this, 13 ) );
	m_pQuickSlot2Button->setUnArmedBorderColor( 0, 0, 0, 255 );
	m_pQuickSlot2Button->setArmedBorderColor( 255, 255, 255, 255 );
	m_pQuickSlot2Button->setUnArmedColor( 0, 0, 0, 255 );
	m_pQuickSlot2Button->setArmedColor( 0, 0, 0, 255 );

	m_pQuickSlot3Button = new ColorButton( "", 607, 73, 99, 99, false, false );
	m_pQuickSlot3Button->setParent( m_pInventory );
	m_pQuickSlot3Button->addActionSignal( new CRECBInventoryHandler_Command( this, 14 ) );
	m_pQuickSlot3Button->setUnArmedBorderColor( 0, 0, 0, 255 );
	m_pQuickSlot3Button->setArmedBorderColor( 255, 255, 255, 255 );
	m_pQuickSlot3Button->setUnArmedColor( 0, 0, 0, 255 );
	m_pQuickSlot3Button->setArmedColor( 0, 0, 0, 255 );

	m_pPopupImage = new CImageLabel( NULL, 0, 0 );
	m_pPopupImage->setVisible( false );
	m_pPopupImage->setParent( this );
	m_pPopupImage->setImage( NULL );

	m_pMenuImage = new CImageLabel( NULL, -5, -5 );
	m_pMenuImage->setVisible( false );
	m_pMenuImage->setParent( m_pInventory );
	m_pMenuImage->setImage( vgui_LoadTGA( "gfx/vgui/inventory_weapons.tga" ) );

	m_pMenuButtons[0] = new ColorButton( "", 0, 0, 126, 42, false, false );
	m_pMenuButtons[0]->setParent( m_pInventory );
	m_pMenuButtons[0]->addActionSignal( new CRECBInventoryHandler_Command( this, 50 ) );
	m_pMenuButtons[0]->setVisible( false );
	m_pMenuButtons[0]->setContentAlignment( Label::a_center );

	m_pMenuButtons[1] = new ColorButton( "", 0, 42, 126, 42, false, false );
	m_pMenuButtons[1]->setParent( m_pInventory );
	m_pMenuButtons[1]->addActionSignal( new CRECBInventoryHandler_Command( this, 51 ) );
	m_pMenuButtons[1]->setVisible( false );
	m_pMenuButtons[1]->setContentAlignment( Label::a_center );

	m_pMenuButtons[2] = new ColorButton( "", 0, 84, 126, 42, false, false );
	m_pMenuButtons[2]->setParent( m_pInventory );
	m_pMenuButtons[2]->addActionSignal( new CRECBInventoryHandler_Command( this, 52 ) );
	m_pMenuButtons[2]->setVisible( false );
	m_pMenuButtons[2]->setContentAlignment( Label::a_center );

	m_pMenuButtons[3] = new ColorButton( "", 0, 126, 126, 42, false, false );
	m_pMenuButtons[3]->setParent( m_pInventory );
	m_pMenuButtons[3]->addActionSignal( new CRECBInventoryHandler_Command( this, 53 ) );
	m_pMenuButtons[3]->setVisible( false );
	m_pMenuButtons[3]->setContentAlignment( Label::a_center );

	m_pMenuButtons[4] = new ColorButton( "", 0, 126, 42, 42, false, false );
	m_pMenuButtons[4]->setParent( m_pInventory );
	m_pMenuButtons[4]->addActionSignal( new CRECBInventoryHandler_Command( this, 54 ) );
	m_pMenuButtons[4]->setVisible( false );
	m_pMenuButtons[4]->setContentAlignment( Label::a_center );

	m_pMenuButtons[5] = new ColorButton( "", 42, 126, 42, 42, false, false );
	m_pMenuButtons[5]->setParent( m_pInventory );
	m_pMenuButtons[5]->addActionSignal( new CRECBInventoryHandler_Command( this, 55 ) );
	m_pMenuButtons[5]->setVisible( false );
	m_pMenuButtons[5]->setContentAlignment( Label::a_center );

	m_pMenuButtons[6] = new ColorButton( "", 84, 126, 42, 42, false, false );
	m_pMenuButtons[6]->setParent( m_pInventory );
	m_pMenuButtons[6]->addActionSignal( new CRECBInventoryHandler_Command( this, 56 ) );
	m_pMenuButtons[6]->setVisible( false );
	m_pMenuButtons[6]->setContentAlignment( Label::a_center );

	for ( i = 0; i < 7; i++ )
	{
		m_pMenuButtons[i]->setUnArmedBorderColor( 255, 255, 255, 128 );
		m_pMenuButtons[i]->setArmedBorderColor( 255, 255, 255, 0 );
		m_pMenuButtons[i]->setUnArmedColor( 255, 255, 255, 128 );
		m_pMenuButtons[i]->setArmedColor( 255, 255, 255, 0 );
	}

	m_pSelection = new Panel( 36, 73, 99, 99 );
	m_pSelection->setParent( m_pInventory );
	m_pSelection->setBgColor( 0, 0, 0, 255 );
	m_pSelection->setVisible( false );
	m_pSelection->setBorder( new LineBorder( Color( 255, 0, 0, 255 ) ) );
	m_pSelection->setPaintBorderEnabled( true );

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hInventoryScheme = pSchemes->getSchemeHandle( "inventory" );
	Font *pInventoryFont = pSchemes->getFont( hInventoryScheme );

	m_pObjectiveLabel = new Label( "", 404, 354, 301, 97 );
	m_pObjectiveLabel->setParent( m_pInventory );
	m_pObjectiveLabel->setBgColor( 0, 0, 0, 255 );
	m_pObjectiveLabel->setFgColor( 255, 255, 255, 0 );
	m_pObjectiveLabel->setFont( pInventoryFont );
	m_pObjectiveLabel->setContentAlignment( Label::a_center );
	m_pObjectiveLabel->setVisible( true );
}

void CRECBInventory::ActionSignal( int cmd )
{
	char szCommand[256];
	char szCmd[32];
	int i;

	gEngfuncs.Con_DPrintf( "Inventory: action %i (selected %i)\n", cmd, m_iSelected );

	const char *pszLevel = gEngfuncs.pfnGetLevelName();

	switch ( cmd )
	{
	case 0:
		break;
	case 1:
		SelectItem( 1 );
		break;
	case 2:
		SelectItem( 2 );
		break;
	case 3:
		SelectItem( 3 );
		break;
	case 4:
		SelectItem( 4 );
		break;
	case 5:
		SelectItem( 5 );
		break;
	case 6:
		SelectItem( 6 );
		break;
	case 7:
		SetAction( 1 );
		break;
	case 8:
		SetAction( 2 );
		break;
	case 9:
		SetAction( 3 );
		break;
	case 10:
		SetAction( 4 );
		break;
	case 11:
		gViewPort->HideTopMenu();
		break;
	case 12:
		break;
	case 13:
		break;
	case 14:
		break;
	case 15:
		if ( !strstr( pszLevel, "cof_" ) )
			SetAction( 7 );
		break;
	case 16:
		if ( !strstr( pszLevel, "cof_" ) && gViewPort && gViewPort->m_pDocuments )
		{
			gViewPort->HideTopMenu();
			gViewPort->m_pDocuments->setVisible( true );
			gViewPort->UpdateCursorState();
		}
		break;
	case 50:
		sprintf( szCmd, "inventoryequip %i\n", m_iSelected );
		gEngfuncs.pfnServerCmd( szCmd );
		m_pMenuImage->setVisible( false );
		for ( i = 0; i < 7; i++ )
			m_pMenuButtons[i]->setVisible( false );
		break;
	case 51:
		{
			int x, y;

			m_iCombine = m_iSelected;
			m_pItemButtons[m_iSelected - 1]->getPos( x, y );
			m_iSelectionAlpha = 255;
			g_bInventoryFadeOut = true;
			m_pMenuImage->setVisible( false );
			m_pSelection->setPos( x, y );
			m_pSelection->setVisible( true );
			for ( i = 0; i < 7; i++ )
				m_pMenuButtons[i]->setVisible( false );
		}
		break;
	case 52:
		sprintf( szCmd, "inventorydrop %i\n", m_iSelected );
		gEngfuncs.pfnServerCmd( szCmd );
		m_pMenuImage->setVisible( false );
		for ( i = 0; i < 7; i++ )
			m_pMenuButtons[i]->setVisible( false );
		break;
	case 53:
		{
			int x, y;

			m_iDualWield = m_iSelected;
			m_pItemButtons[m_iSelected - 1]->getPos( x, y );
			m_iSelectionAlpha = 255;
			g_bInventoryFadeOut = true;
			m_pSelection->setPos( x, y );
			m_pSelection->setVisible( true );
			m_pMenuImage->setVisible( false );
			for ( i = 0; i < 7; i++ )
				m_pMenuButtons[i]->setVisible( false );
		}
		break;
	case 54:
		if ( strlen( gHUD.GetSlotItem( m_iSelected ) ) && strstr( gHUD.GetSlotItem( m_iSelected ), "weapon_" ) )
		{
			m_pItemImages[6]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.GetSlotItem( m_iSelected ) ) ) );
			strncpy( gHUD.m_szInventoryWeapon1, gHUD.GetSlotItem( m_iSelected ), 128 );
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			sprintf( szCommand, "quickselset1 %i\n", m_iSelected );
			gEngfuncs.pfnServerCmd( szCommand );
			m_pMenuImage->setVisible( false );
			for ( i = 0; i < 7; i++ )
				m_pMenuButtons[i]->setVisible( false );
		}
		else
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );

		m_iAction = 0;
		break;
	case 55:
		if ( strlen( gHUD.GetSlotItem( m_iSelected ) ) && strstr( gHUD.GetSlotItem( m_iSelected ), "weapon_" ) )
		{
			m_pItemImages[7]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.GetSlotItem( m_iSelected ) ) ) );
			strncpy( gHUD.m_szInventoryWeapon2, gHUD.GetSlotItem( m_iSelected ), 128 );
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			sprintf( szCommand, "quickselset2 %i\n", m_iSelected );
			gEngfuncs.pfnServerCmd( szCommand );
			m_pMenuImage->setVisible( false );
			for ( i = 0; i < 7; i++ )
				m_pMenuButtons[i]->setVisible( false );
		}
		else
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );

		m_iAction = 0;
		break;
	case 56:
		if ( strlen( gHUD.GetSlotItem( m_iSelected ) ) && strstr( gHUD.GetSlotItem( m_iSelected ), "weapon_" ) )
		{
			m_pItemImages[9]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.GetSlotItem( m_iSelected ) ) ) );
			strncpy( gHUD.m_szInventoryWeapon3, gHUD.GetSlotItem( m_iSelected ), 128 );
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			sprintf( szCommand, "quickselset3 %i\n", m_iSelected );
			gEngfuncs.pfnServerCmd( szCommand );
			m_pMenuImage->setVisible( false );
			for ( i = 0; i < 7; i++ )
				m_pMenuButtons[i]->setVisible( false );
		}
		else
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );

		m_iAction = 0;
		break;
	default:
		gEngfuncs.Con_DPrintf( "Unknown Inventory ActionSignal %i.\n", cmd );
		break;
	}
}

void CRECBInventory::ClearItems()
{
	for ( int i = 0; i < 10; i++ )
		m_pItemImages[i]->setImage( NULL );

	m_iAction = 0;
	m_iActionItem2 = 0;
	m_iActionItem1 = 0;
	m_iHoverItem = 0;
}

void CRECBInventory::DoAction( int slot )
{
	char szCommand[256];
	char szCmd[32];

	if ( m_iAction <= 0 )
		return;

	const char *pszLevel = gEngfuncs.pfnGetLevelName();

	switch ( m_iAction )
	{
	case 1:
		sprintf( szCmd, "inventoryequip %i\n", slot );
		gEngfuncs.pfnServerCmd( szCmd );
		break;
	case 2:
		if ( !m_iActionItem1 || !m_iActionItem2 )
			return;

		if ( m_iActionItem1 == m_iActionItem2 )
		{
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
		}
		else
		{
			if ( strlen( gHUD.CheckItem( "combined_result", gHUD.GetSlotItem( m_iActionItem1 ) ) ) <= 1 )
				break;

			sprintf( szCommand, "inventorycombine %i %i %s\n", m_iActionItem1, m_iActionItem2, gHUD.CheckItem( "combined_result", gHUD.GetSlotItem( m_iActionItem1 ) ) );
			gEngfuncs.pfnServerCmd( szCommand );
		}

		m_iActionItem1 = 0;
		m_iActionItem2 = 0;
		break;
	case 3:
		sprintf( szCmd, "inventoryuse %i\n", slot );
		gEngfuncs.pfnServerCmd( szCmd );
		break;
	case 4:
		sprintf( szCmd, "inventorydrop %i\n", slot );
		gEngfuncs.pfnServerCmd( szCmd );
		break;
	case 5:
		if ( strlen( gHUD.GetSlotItem( slot ) ) && strstr( gHUD.GetSlotItem( slot ), "weapon_" ) )
		{
			m_pItemImages[6]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.GetSlotItem( slot ) ) ) );
			strncpy( gHUD.m_szInventoryWeapon1, gHUD.GetSlotItem( slot ), 128 );
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			sprintf( szCmd, "quickselset1 %i\n", slot );
			gEngfuncs.pfnServerCmd( szCmd );
		}
		else
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
		break;
	case 6:
		if ( strlen( gHUD.GetSlotItem( slot ) ) && strstr( gHUD.GetSlotItem( slot ), "weapon_" ) )
		{
			m_pItemImages[7]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.GetSlotItem( slot ) ) ) );
			strncpy( gHUD.m_szInventoryWeapon2, gHUD.GetSlotItem( slot ), 128 );
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			sprintf( szCmd, "quickselset2 %i\n", slot );
			gEngfuncs.pfnServerCmd( szCmd );
		}
		else
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
		break;
	case 7:
		if ( !strstr( pszLevel, "cof_" ) && m_iActionItem1 && m_iActionItem2 )
		{
			if ( m_iActionItem1 == m_iActionItem2 )
			{
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
				m_iActionItem2 = 0;
				m_iActionItem1 = 0;
				return;
			}

			if ( strlen( gHUD.CheckItem( "classname", gHUD.GetSlotItem( m_iActionItem1 ) ) ) > 1 )
			{
				sprintf( szCommand, "inventorydualwield %i %i\n", m_iActionItem1, m_iActionItem2 );
				gEngfuncs.pfnServerCmd( szCommand );
				m_iActionItem2 = 0;
				m_iActionItem1 = 0;
				return;
			}
		}
		return;
	case 8:
		if ( strlen( gHUD.GetSlotItem( slot ) ) && strstr( gHUD.GetSlotItem( slot ), "weapon_" ) )
		{
			m_pItemImages[9]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.GetSlotItem( slot ) ) ) );
			strncpy( gHUD.m_szInventoryWeapon3, gHUD.GetSlotItem( slot ), 128 );
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			sprintf( szCmd, "quickselset3 %i\n", slot );
			gEngfuncs.pfnServerCmd( szCmd );
		}
		else
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
		break;
	default:
		return;
	}

	m_iAction = 0;
}

void CRECBInventory::Initialize()
{
	if ( gViewPort && gViewPort->m_pRECBInventory )
		gViewPort->m_pRECBInventory->Update();

	m_pItemImages[6]->setImage( NULL );
	gHUD.m_szInventoryWeapon1[0] = 0;
	if ( strlen( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon1 ) ) > 2 )
		m_pItemImages[6]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon1 ) ) );

	m_pItemImages[7]->setImage( NULL );
	gHUD.m_szInventoryWeapon2[0] = 0;
	if ( strlen( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon2 ) ) > 2 )
		m_pItemImages[7]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon2 ) ) );

	m_pItemImages[9]->setImage( NULL );
	gHUD.m_szInventoryWeapon3[0] = 0;
	if ( strlen( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon3 ) ) > 2 )
		m_pItemImages[9]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon3 ) ) );
}

int CRECBInventory::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( pszCurrentBinding )
	{
		if ( !strcmp( pszCurrentBinding, "toggleconsole" ) )
			return 1;

		if ( !strcmp( pszCurrentBinding, "+inventory" ) && isVisible() )
		{
			gViewPort->HideTopMenu();
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_close.wav", 1 );
		}
	}

	return 0;
}

bool CRECBInventory::MouseOver( int x, int y, int wide, int tall )
{
	int cx, cy;

	if ( !gViewPort )
		return false;

	getApp()->getCursorPos( cx, cy );

	if ( cx >= x && cx <= x + wide && cy >= y && cy <= y + wide )
		return true;

	return false;
}

int MsgInventChck( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if ( gViewPort && gViewPort->m_pRECBInventory )
		gViewPort->m_pRECBInventory->Update();

	return 1;
}

int MsgQSlotClear( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iSlot = READ_BYTE();
	int iOn = READ_BYTE();

	if ( gViewPort->m_pRECBInventory )
	{
		switch ( iSlot )
		{
		case 1:
			if ( !iOn )
			{
				gViewPort->m_pRECBInventory->m_pItemImages[6]->setImage( NULL );
				gHUD.m_szInventoryWeapon1[0] = 0;
			}
			else
			{
				if ( strlen( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon1 ) ) > 2 )
					gViewPort->m_pRECBInventory->m_pItemImages[6]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon1 ) ) );
			}
			break;
		case 2:
			if ( !iOn )
			{
				gViewPort->m_pRECBInventory->m_pItemImages[7]->setImage( NULL );
				gHUD.m_szInventoryWeapon2[0] = 0;
			}
			else
			{
				if ( strlen( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon2 ) ) > 2 )
					gViewPort->m_pRECBInventory->m_pItemImages[7]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon2 ) ) );
			}
			break;
		case 3:
			if ( !iOn )
			{
				gViewPort->m_pRECBInventory->m_pItemImages[9]->setImage( NULL );
				gHUD.m_szInventoryWeapon3[0] = 0;
			}
			else
			{
				if ( strlen( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon3 ) ) > 2 )
					gViewPort->m_pRECBInventory->m_pItemImages[9]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.m_szInventoryWeapon3 ) ) );
			}
			break;
		}
	}

	return 1;
}

int MsgQSlotUpdate( const char *pszName, int iSize, void *pbuf )
{
	char szCommand[128];

	BEGIN_READ( pbuf, iSize );
	int iSlot = READ_BYTE();
	int iItem = READ_BYTE();

	if ( gViewPort->m_pRECBInventory )
	{
		switch ( iSlot )
		{
		case 1:
			if ( strlen( gHUD.GetSlotItem( iItem ) ) && strstr( gHUD.GetSlotItem( iItem ), "weapon_" ) )
			{
				gViewPort->m_pRECBInventory->m_pItemImages[6]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.GetSlotItem( iItem ) ) ) );
				strncpy( gHUD.m_szInventoryWeapon1, gHUD.GetSlotItem( iItem ), 128 );
				sprintf( szCommand, "quickselset1 %i\n", iItem );
				gEngfuncs.pfnServerCmd( szCommand );
			}
			break;
		case 2:
			if ( strlen( gHUD.GetSlotItem( iItem ) ) && strstr( gHUD.GetSlotItem( iItem ), "weapon_" ) )
			{
				gViewPort->m_pRECBInventory->m_pItemImages[7]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.GetSlotItem( iItem ) ) ) );
				strncpy( gHUD.m_szInventoryWeapon2, gHUD.GetSlotItem( iItem ), 128 );
				sprintf( szCommand, "quickselset2 %i\n", iItem );
				gEngfuncs.pfnServerCmd( szCommand );
			}
			break;
		case 3:
			if ( strlen( gHUD.GetSlotItem( iItem ) ) && strstr( gHUD.GetSlotItem( iItem ), "weapon_" ) )
			{
				gViewPort->m_pRECBInventory->m_pItemImages[9]->setImage( vgui_LoadTGA( gHUD.CheckItem( "inventory_picture", gHUD.GetSlotItem( iItem ) ) ) );
				strncpy( gHUD.m_szInventoryWeapon3, gHUD.GetSlotItem( iItem ), 128 );
				sprintf( szCommand, "quickselset3 %i\n", iItem );
				gEngfuncs.pfnServerCmd( szCommand );
			}
			break;
		}
	}

	return 1;
}

void CRECBInventory::SelectItem( int slot )
{
	char szCommand[256];
	char szCmd[32];

	if ( m_iDualWield )
	{
		if ( m_iDualWield != slot )
		{
			if ( strlen( gHUD.CheckItem( "classname", gHUD.GetSlotItem( slot ) ) ) > 1 )
			{
				sprintf( szCmd, "inventorydualwield %i %i\n", m_iDualWield, slot );
				gEngfuncs.pfnServerCmd( szCmd );
				m_iDualWield = 0;
				g_bInventoryFadeOut = false;
				m_pSelection->setVisible( false );
				return;
			}

			m_iDualWield = 0;
			g_bInventoryFadeOut = false;
			m_pSelection->setVisible( false );
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
			return;
		}
	}
	else
	{
		if ( m_iCombine )
		{
			if ( m_iCombine != slot )
			{
				if ( strlen( gHUD.CheckItem( "combined_result", gHUD.GetSlotItem( slot ) ) ) > 1 )
				{
					sprintf( szCommand, "inventorycombine %i %i %s\n", m_iCombine, slot, gHUD.CheckItem( "combined_result", gHUD.GetSlotItem( m_iCombine ) ) );
					gEngfuncs.pfnServerCmd( szCommand );
					m_iDualWield = 0;
					g_bInventoryFadeOut = false;
					m_pSelection->setVisible( false );
					return;
				}

				m_iCombine = 0;
				g_bInventoryFadeOut = false;
				m_pSelection->setVisible( false );
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
				return;
			}
		}
		else
		{
			if ( strlen( gHUD.GetSlotItem( slot ) ) > 1 )
			{
				int x, y, wide, tall;
				bool bItem = false;

				m_iSelected = slot;
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_toggle.wav", 1 );

				if ( !strstr( gHUD.GetSlotItem( m_iSelected ), "weapon_" ) )
					bItem = true;

				m_pItemButtons[m_iSelected - 1]->getPos( x, y );
				m_pItemButtons[m_iSelected - 1]->getSize( wide, tall );
				x += wide;
				m_pMenuButtons[0]->getSize( wide, tall );

				m_pMenuImage->setVisible( true );
				m_pMenuImage->setPos( x - 5, y - 5 );

				if ( bItem )
					m_pMenuImage->setImage( vgui_LoadTGA( "gfx/vgui/inventory_items.tga" ) );
				else
					m_pMenuImage->setImage( vgui_LoadTGA( "gfx/vgui/inventory_weapons.tga" ) );

				for ( int i = 1; i <= 7; i++ )
				{
					m_pMenuButtons[i - 1]->setPos( x, y );

					if ( i >= 4 )
					{
						if ( !bItem )
							m_pMenuButtons[i - 1]->setVisible( true );
						else
							m_pMenuButtons[i - 1]->setVisible( false );
					}
					else
					{
						m_pMenuButtons[i - 1]->setVisible( true );
					}

					if ( i <= 4 )
						y += tall;

					if ( i >= 5 )
						x += 42;
				}
			}
			return;
		}
	}

	gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
}

void CRECBInventory::SetAction( int action )
{
	gEngfuncs.pfnPlaySoundByName( "inventory/inventory_toggle.wav", 1 );

	m_iAction = action;
	m_iActionItem2 = 0;
	m_iActionItem1 = 0;
}

void CRECBInventory::Update()
{
	const char *pszLevel = gEngfuncs.pfnGetLevelName();

	if ( strstr( pszLevel, "cof_" ) )
	{
		m_pBackground->setVisible( true );
		m_pInventory->setImage( vgui_LoadTGA( "gfx/vgui/640_inventory_p.tga" ) );

		m_pItemButtons[0]->setPos( 36, 91 );
		m_pItemImages[0]->setPos( 31, 86 );
		m_pItemButtons[1]->setPos( 138, 91 );
		m_pItemImages[1]->setPos( 133, 86 );
		m_pItemButtons[2]->setPos( 240, 91 );
		m_pItemImages[2]->setPos( 235, 86 );
		m_pItemButtons[3]->setPos( 36, 213 );
		m_pItemImages[3]->setPos( 31, 208 );
		m_pItemButtons[4]->setPos( 138, 213 );
		m_pItemImages[4]->setPos( 133, 208 );
		m_pItemButtons[5]->setPos( 240, 213 );
		m_pItemImages[5]->setPos( 235, 208 );
		m_pItemImages[8]->setPos( 31, 329 );
		m_pQuickSlot1Button->setPos( 403, 90 );
		m_pQuickSlot2Button->setPos( 505, 90 );
		m_pQuickSlot3Button->setPos( 607, 90 );
		m_pItemImages[6]->setPos( 398, 85 );
		m_pItemImages[7]->setPos( 500, 85 );
		m_pItemImages[9]->setPos( 602, 85 );

		m_pOptionButtons[0]->setSize( 363, 42 );
		m_pOptionButtons[0]->setPos( 373, 243 );
		m_pOptionButtons[1]->setSize( 363, 42 );
		m_pOptionButtons[1]->setPos( 373, 285 );
		m_pOptionButtons[2]->setSize( 363, 42 );
		m_pOptionButtons[2]->setPos( 373, 327 );
		m_pOptionButtons[3]->setVisible( false );
		m_pOptionButtons[4]->setVisible( false );

		setBgColor( 255, 0, 0, 255 );
	}
	else
	{
		m_pBackground->setVisible( true );

		if ( m_iThree )
			m_pInventory->setImage( vgui_LoadTGA( "gfx/vgui/640_inventory_three.tga" ) );
		else
			m_pInventory->setImage( vgui_LoadTGA( "gfx/vgui/640_inventory.tga" ) );


		m_pItemButtons[0]->setPos( 36, 73 );
		m_pItemImages[0]->setPos( 31, 68 );
		m_pItemButtons[1]->setPos( 138, 73 );
		m_pItemImages[1]->setPos( 133, 68 );
		m_pItemButtons[2]->setPos( 240, 73 );
		m_pItemImages[2]->setPos( 235, 68 );
		m_pItemButtons[3]->setPos( 36, 232 );
		m_pItemImages[3]->setPos( 31, 227 );
		m_pItemButtons[4]->setPos( 138, 232 );
		m_pItemImages[4]->setPos( 133, 227 );
		m_pItemButtons[5]->setPos( 240, 232 );
		m_pItemImages[5]->setPos( 235, 227 );
		m_pItemImages[8]->setPos( 31, 348 );
		m_pQuickSlot1Button->setPos( 403, 73 );
		m_pQuickSlot2Button->setPos( 505, 73 );
		m_pQuickSlot3Button->setPos( 607, 73 );
		m_pItemImages[6]->setPos( 398, 68 );
		m_pItemImages[7]->setPos( 500, 68 );
		m_pItemImages[9]->setPos( 602, 68 );

		m_pOptionButtons[0]->setSize( 363, 47 );
		m_pOptionButtons[0]->setPos( 373, 226 );
		m_pOptionButtons[1]->setSize( 363, 47 );
		m_pOptionButtons[1]->setPos( 373, 276 );
		m_pOptionButtons[2]->setSize( 363, 47 );
		m_pOptionButtons[2]->setPos( 373, 325 );
		m_pOptionButtons[3]->setVisible( false );
		m_pOptionButtons[4]->setVisible( true );

		setBgColor( 0, 0, 0, 255 );
	}

	m_pBackgroundPanel->setBgColor( 0, 0, 0, 0 );
}

void VGUI_QSlotInit()
{
	gEngfuncs.pfnHookUserMsg( "QSlotClear", MsgQSlotClear );
	gEngfuncs.pfnHookUserMsg( "QSlotUpdate", MsgQSlotUpdate );
	gEngfuncs.pfnHookUserMsg( "InventChck", MsgInventChck );
}

void CRECBInventory::setVisible( bool state )
{
	int i;

	for ( i = 0; i < 7; i++ )
		m_pMenuButtons[i]->setVisible( false );

	m_pMenuImage->setVisible( false );
	m_pSelection->setVisible( false );

	m_iSelectionAlpha = 0;
	m_iSelected = 0;
	m_iCombine = 0;
	m_iDualWield = 0;
	m_iAction = 0;
	m_iActionItem2 = 0;
	m_iActionItem1 = 0;
	m_iHoverItem = 0;

	if ( state )
	{
		if ( m_pItemImages[8] )
			m_pItemImages[8]->setImage( NULL );

		if ( m_pPopupImage )
			m_pPopupImage->setImage( NULL );

		m_bUnknownE0 = false;
		m_iUnknownE4 = 0;

		if ( strlen( gHUD.m_szObjectiveTitle ) > 2 && strlen( gHUD.m_szObjectiveText ) > 2 )
		{
			ParseHintTokens( gHUD.m_szObjectiveTitle, gHUD.m_szObjectiveText );

			if ( strlen( gViewPort->m_pHUDControl->m_szHintLine1 ) > 2 )
			{
				char szText[256];
				unsigned int j;

				for ( i = 0, j = 0; j < strlen( gViewPort->m_pHUDControl->m_szHintLine1 ); j++ )
				{
					i++;

					if ( gViewPort->m_pHUDControl->m_szHintLine1[j] == '@' )
						szText[j] = '\n';
					else
						szText[j] = gViewPort->m_pHUDControl->m_szHintLine1[j];
				}

				szText[i] = 0;
				m_pObjectiveLabel->setText( szText );
			}

			m_pObjectiveLabel->setVisible( true );
		}
		else
			m_pObjectiveLabel->setVisible( false );
	}
	else
	{
		if ( m_pPopupImage )
			m_pPopupImage->setImage( NULL );

		m_bUnknownE0 = false;
		m_iUnknownE4 = 0;

		if ( gEngfuncs.pfnGetLevelName() )
		{
			for ( i = 0; i < 5; i++ )
			{
				if ( m_pOptionButtons[i] && m_pOptionButtons[i]->isVisible() )
					m_pOptionButtons[i]->cursorExited();
			}

			for ( i = 0; i < 6; i++ )
			{
				if ( m_pItemButtons[i] && m_pItemButtons[i]->isVisible() )
					m_pItemButtons[i]->cursorExited();
			}

			m_pQuickSlot1Button->cursorExited();
			m_pQuickSlot2Button->cursorExited();
			m_pQuickSlot3Button->cursorExited();
		}

		m_pItemImages[8]->setImage( NULL );
	}

	Panel::setVisible( state );
}

void CRECBInventory::solve()
{
	int i;

	if ( isVisible() )
	{
		if ( m_pSelection->isVisible() )
		{
			if ( g_bInventoryFadeOut )
			{
				if ( m_iSelectionAlpha > 0 )
					m_iSelectionAlpha -= 15;

				if ( m_iSelectionAlpha <= 0 )
				{
					g_bInventoryFadeOut = false;
					m_iSelectionAlpha = 0;
				}
			}
			else
			{
				if ( m_iSelectionAlpha < 255 )
					m_iSelectionAlpha += 15;

				if ( m_iSelectionAlpha >= 255 )
				{
					g_bInventoryFadeOut = true;
					m_iSelectionAlpha = 255;
				}
			}

			m_pSelection->setBorder( new LineBorder( Color( 255, 0, 0, m_iSelectionAlpha ) ) );
		}

		if ( iWheelchair && isVisible() )
		{
			gViewPort->HideTopMenu();
			return;
		}

		if ( getApp()->isMouseDown( MOUSE_LEFT, NULL ) )
		{
			bool bArmed = false;
			int iCursorX, iCursorY;

			getApp()->getCursorPos( iCursorX, iCursorY );

			for ( i = 0; i < 7; i++ )
			{
				if ( m_pMenuButtons[i]->isArmed() )
					bArmed = true;
				else if ( m_pMenuButtons[i]->isVisible() && m_pMenuButtons[i]->isWithin( iCursorX, iCursorY ) )
					bArmed = true;
			}

			if ( !bArmed )
			{
				if ( m_pMenuImage->isVisible() )
					gEngfuncs.Con_DPrintf( "Inventory: menu closed by press at %i %i\n", iCursorX, iCursorY );

				for ( i = 0; i < 7; i++ )
					m_pMenuButtons[i]->setVisible( false );

				m_pMenuImage->setVisible( false );
			}
		}

		if ( m_pPopupImage->isVisible() )
			m_pPopupImage->setVisible( false );

		for ( i = 1; i <= 6; i++ )
		{
			if ( m_pItemButtons[i - 1]->isArmed() && strlen( gHUD.GetSlotItem( i ) ) > 1 )
			{
				m_pItemImages[8]->setImage( vgui_LoadTGA( gHUD.CheckItem( "check_picture", gHUD.GetSlotItem( i ) ) ) );
				m_iHoverItem = i;
			}
		}

		if ( m_pQuickSlot1Button->isArmed() && m_iHoverItem != 7 && strlen( gHUD.m_szInventoryWeapon1 ) > 1 )
		{
			m_iHoverItem = 7;
			m_pItemImages[8]->setImage( vgui_LoadTGA( gHUD.CheckItem( "check_picture", gHUD.m_szInventoryWeapon1 ) ) );
		}

		if ( m_pQuickSlot2Button->isArmed() && m_iHoverItem != 8 && strlen( gHUD.m_szInventoryWeapon2 ) > 1 )
		{
			m_iHoverItem = 8;
			m_pItemImages[8]->setImage( vgui_LoadTGA( gHUD.CheckItem( "check_picture", gHUD.m_szInventoryWeapon2 ) ) );
		}

		if ( m_pQuickSlot3Button->isArmed() && m_iHoverItem != 9 && strlen( gHUD.m_szInventoryWeapon3 ) > 1 )
		{
			m_iHoverItem = 9;
			m_pItemImages[8]->setImage( vgui_LoadTGA( gHUD.CheckItem( "check_picture", gHUD.m_szInventoryWeapon3 ) ) );
		}

		if ( m_iHoverItem >= 1 && m_iHoverItem <= 6 && !m_pItemButtons[m_iHoverItem - 1]->isArmed() )
		{
			m_pItemImages[8]->setImage( NULL );
			m_iHoverItem = 0;
		}

		if ( m_iHoverItem == 7 && !m_pQuickSlot1Button->isArmed() )
		{
			m_pItemImages[8]->setImage( NULL );
			m_iHoverItem = 0;
		}

		if ( m_iHoverItem == 8 && !m_pQuickSlot2Button->isArmed() )
		{
			m_pItemImages[8]->setImage( NULL );
			m_iHoverItem = 0;
		}

		if ( m_iHoverItem == 9 && !m_pQuickSlot3Button->isArmed() )
		{
			m_pItemImages[8]->setImage( NULL );
			m_iHoverItem = 0;
		}

		if ( m_pQuickSlot1Button->isArmed() && m_iHoverItem != 7 )
			m_iHoverItem = 7;

		if ( m_pQuickSlot2Button->isArmed() && m_iHoverItem != 8 )
			m_iHoverItem = 8;

		if ( m_pQuickSlot3Button->isArmed() && m_iHoverItem != 9 )
			m_iHoverItem = 9;
	}

	Panel::solve();
}
