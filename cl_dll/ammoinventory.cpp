// ======================================
// ======================================

#include "hud.h"
#include "client_port.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "ammoinventory.h"
#include "../game_shared/vgui_loadtga.h"

CAmmoInventory::CAmmoInventory() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	Panel *pPanel;

	pPanel = new Panel( 0, 0, ScreenWidth, ScreenHeight );
	pPanel->setParent( this );
	pPanel->setSize( ScreenWidth, ScreenHeight );
	pPanel->setPos( 0, 0 );
	pPanel->setBgColor( 0, 0, 0, 0 );
	pPanel->setVisible( true );

	m_pBackgroundImage = vgui_LoadTGA( "gfx/vgui/backgroundtoall.tga" );
	if ( m_pBackgroundImage )
		m_pBackgroundImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/backgroundtoall.tga!\n" );

	m_pBackgroundImage->getSize( wide, tall );
	x = ScreenWidth / 2 - wide / 2;
	y = ScreenHeight / 2 - tall / 2;

	m_pBackground = new ImagePanel( m_pBackgroundImage );
	m_pBackground->setParent( this );
	m_pBackground->setPos( x, y );
	m_pBackground->setVisible( true );

	m_pInventoryImage = vgui_LoadTGA( "gfx/vgui/ammo/ammo_inventory.tga" );
	if ( m_pInventoryImage )
		m_pInventoryImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/ammo/ammo_inventory.tga!\n" );

	m_pInventoryImage->getSize( wide, tall );
	x = ScreenWidth / 2 - wide / 2;
	y = ScreenHeight / 2 - tall / 2;

	m_pInventory = new ImagePanel( m_pInventoryImage );
	m_pInventory->setParent( this );
	m_pInventory->setPos( x, y );
	m_pInventory->setVisible( true );

	m_pSlotImage[1] = new CImageLabel( NULL, 31, 45 );
	m_pSlotImage[1]->setVisible( true );
	m_pSlotImage[1]->setParent( m_pInventory );
	m_pSlotImage[1]->setImage( NULL );

	m_pSlotImage[2] = new CImageLabel( NULL, 133, 45 );
	m_pSlotImage[2]->setVisible( true );
	m_pSlotImage[2]->setParent( m_pInventory );
	m_pSlotImage[2]->setImage( NULL );

	m_pSlotImage[3] = new CImageLabel( NULL, 235, 45 );
	m_pSlotImage[3]->setVisible( true );
	m_pSlotImage[3]->setParent( m_pInventory );
	m_pSlotImage[3]->setImage( NULL );

	m_pSlotImage[4] = new CImageLabel( NULL, 31, 167 );
	m_pSlotImage[4]->setVisible( true );
	m_pSlotImage[4]->setParent( m_pInventory );
	m_pSlotImage[4]->setImage( NULL );

	m_pSlotImage[5] = new CImageLabel( NULL, 133, 167 );
	m_pSlotImage[5]->setVisible( true );
	m_pSlotImage[5]->setParent( m_pInventory );
	m_pSlotImage[5]->setImage( NULL );

	m_pSlotImage[6] = new CImageLabel( NULL, 235, 167 );
	m_pSlotImage[6]->setVisible( true );
	m_pSlotImage[6]->setParent( m_pInventory );
	m_pSlotImage[6]->setImage( NULL );

	m_pCountLabel[1] = new Label( "", 36, 50, 99, 99 );
	m_pCountLabel[1]->setVisible( true );
	m_pCountLabel[1]->setParent( m_pInventory );
	m_pCountLabel[1]->setBgColor( 0, 0, 0, 255 );
	m_pCountLabel[1]->setFgColor( 255, 255, 255, 0 );

	m_pCountLabel[2] = new Label( "", 138, 50, 99, 99 );
	m_pCountLabel[2]->setVisible( true );
	m_pCountLabel[2]->setParent( m_pInventory );
	m_pCountLabel[2]->setBgColor( 0, 0, 0, 255 );
	m_pCountLabel[2]->setFgColor( 255, 255, 255, 0 );

	m_pCountLabel[3] = new Label( "", 240, 50, 99, 99 );
	m_pCountLabel[3]->setVisible( true );
	m_pCountLabel[3]->setParent( m_pInventory );
	m_pCountLabel[3]->setBgColor( 0, 0, 0, 255 );
	m_pCountLabel[3]->setFgColor( 255, 255, 255, 0 );

	m_pCountLabel[4] = new Label( "", 36, 172, 99, 99 );
	m_pCountLabel[4]->setVisible( true );
	m_pCountLabel[4]->setParent( m_pInventory );
	m_pCountLabel[4]->setBgColor( 0, 0, 0, 255 );
	m_pCountLabel[4]->setFgColor( 255, 255, 255, 0 );

	m_pCountLabel[5] = new Label( "", 138, 172, 99, 99 );
	m_pCountLabel[5]->setVisible( true );
	m_pCountLabel[5]->setParent( m_pInventory );
	m_pCountLabel[5]->setBgColor( 0, 0, 0, 255 );
	m_pCountLabel[5]->setFgColor( 255, 255, 255, 0 );

	m_pCountLabel[6] = new Label( "", 240, 172, 99, 99 );
	m_pCountLabel[6]->setVisible( true );
	m_pCountLabel[6]->setParent( m_pInventory );
	m_pCountLabel[6]->setBgColor( 0, 0, 0, 255 );
	m_pCountLabel[6]->setFgColor( 255, 255, 255, 0 );

	m_pSelectedImage = new CImageLabel( NULL, 31, 289 );
	m_pSelectedImage->setVisible( true );
	m_pSelectedImage->setParent( m_pInventory );
	m_pSelectedImage->setImage( NULL );

	m_pButton[1] = new ColorButton( "", 36, 50, 99, 99, false, false );
	m_pButton[1]->setParent( m_pInventory );
	m_pButton[1]->addActionSignal( new CAmmoInventoryHandler_Command( this, 1 ) );

	m_pButton[2] = new ColorButton( "", 138, 50, 99, 99, false, false );
	m_pButton[2]->setParent( m_pInventory );
	m_pButton[2]->addActionSignal( new CAmmoInventoryHandler_Command( this, 2 ) );

	m_pButton[3] = new ColorButton( "", 240, 50, 99, 99, false, false );
	m_pButton[3]->setParent( m_pInventory );
	m_pButton[3]->addActionSignal( new CAmmoInventoryHandler_Command( this, 3 ) );

	m_pButton[4] = new ColorButton( "", 36, 172, 99, 99, false, false );
	m_pButton[4]->setParent( m_pInventory );
	m_pButton[4]->addActionSignal( new CAmmoInventoryHandler_Command( this, 4 ) );

	m_pButton[5] = new ColorButton( "", 138, 172, 99, 99, false, false );
	m_pButton[5]->setParent( m_pInventory );
	m_pButton[5]->addActionSignal( new CAmmoInventoryHandler_Command( this, 5 ) );

	m_pButton[6] = new ColorButton( "", 240, 172, 99, 99, false, false );
	m_pButton[6]->setParent( m_pInventory );
	m_pButton[6]->addActionSignal( new CAmmoInventoryHandler_Command( this, 6 ) );

	m_pDropButton = new ColorButton( "", 7, 415, 360, 50, false, false );
	m_pDropButton->setParent( m_pInventory );
	m_pDropButton->addActionSignal( new CAmmoInventoryHandler_Command( this, 7 ) );

	setBgColor( 0, 0, 0, 0 );

	Initialize();
}

void CAmmoInventory::ActionSignal( int cmd )
{
	switch ( cmd )
	{
	case 1:
		if ( m_iDropMode )
		{
			if ( m_iAmmoType[1] )
			{
				gEngfuncs.pfnServerCmd( "dropammo 1" );
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			}
			else
			{
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
			}
		}
		break;
	case 2:
		if ( m_iDropMode )
		{
			if ( m_iAmmoType[2] )
			{
				gEngfuncs.pfnServerCmd( "dropammo 2" );
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			}
			else
			{
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
			}
		}
		break;
	case 3:
		if ( m_iDropMode )
		{
			if ( m_iAmmoType[3] )
			{
				gEngfuncs.pfnServerCmd( "dropammo 3" );
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			}
			else
			{
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
			}
		}
		break;
	case 4:
		if ( m_iDropMode )
		{
			if ( m_iAmmoType[4] )
			{
				gEngfuncs.pfnServerCmd( "dropammo 4" );
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			}
			else
			{
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
			}
		}
		break;
	case 5:
		if ( m_iDropMode )
		{
			if ( m_iAmmoType[5] )
			{
				gEngfuncs.pfnServerCmd( "dropammo 5" );
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			}
			else
			{
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
			}
		}
		break;
	case 6:
		if ( m_iDropMode )
		{
			if ( m_iAmmoType[6] )
			{
				gEngfuncs.pfnServerCmd( "dropammo 6" );
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_click.wav", 1 );
			}
			else
			{
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
			}
		}
		break;
	case 7:
		m_iDropMode = 1;
		gEngfuncs.pfnPlaySoundByName( "inventory/inventory_toggle.wav", 1 );
		break;
	}
}

void CAmmoInventory::Initialize()
{
	m_iDropMode = 0;
	m_iSelected = 0;
	setVisible( false );

	m_iAmmoType[1] = 0;
	m_iAmmoType[2] = 0;
	m_iAmmoType[3] = 0;
	m_iAmmoType[4] = 0;
	m_iAmmoType[5] = 0;
	m_iAmmoType[6] = 0;
}

int CAmmoInventory::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( pszCurrentBinding )
	{
		if ( !strcmp( pszCurrentBinding, "toggleconsole" ) )
			return 1;

		if ( !strcmp( pszCurrentBinding, "+inventory" ) && isVisible() )
		{
			setVisible( false );
			gViewPort->UpdateCursorState();
			gEngfuncs.pfnPlaySoundByName( "inventory/inventory_close.wav", 1 );
		}
	}

	return 0;
}

int MsgAmmoInv( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pAmmoInventory )
	{
		int iSlot = READ_BYTE();
		int iType = READ_BYTE();
		int iCount = READ_BYTE();
		char szCount[4];

		itoa( iCount, szCount, 10 );

		gViewPort->m_pAmmoInventory->m_iAmmoType[iSlot] = iType;
		gViewPort->m_pAmmoInventory->m_pCountLabel[iSlot]->setText( szCount );
		gViewPort->m_pAmmoInventory->m_pCountLabel[iSlot]->setContentAlignment( Label::a_southwest );

		if ( iType )
		{
			switch ( iType )
			{
			case 1:
				if ( gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot] )
					gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot]->setImage( vgui_LoadTGA( "gfx/vgui/ammo/glock_slot.tga" ) );
				break;
			case 2:
				if ( gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot] )
					gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot]->setImage( vgui_LoadTGA( "gfx/vgui/ammo/m16_slot.tga" ) );
				break;
			case 3:
				if ( gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot] )
					gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot]->setImage( vgui_LoadTGA( "gfx/vgui/ammo/p345_slot.tga" ) );
				break;
			case 4:
				if ( gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot] )
					gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot]->setImage( vgui_LoadTGA( "gfx/vgui/ammo/shotgun_slot.tga" ) );
				break;
			case 5:
				if ( gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot] )
					gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot]->setImage( vgui_LoadTGA( "gfx/vgui/ammo/tmp_slot.tga" ) );
				break;
			}
		}
		else
		{
			if ( gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot] )
			{
				gViewPort->m_pAmmoInventory->m_pSlotImage[iSlot]->setImage( NULL );

				if ( gViewPort->m_pAmmoInventory->m_iSelected == iSlot && gViewPort->m_pAmmoInventory->m_pSelectedImage )
					gViewPort->m_pAmmoInventory->m_pSelectedImage->setImage( NULL );
			}
		}
	}

	return 1;
}

void VGUI_AmmoInvInit()
{
	gEngfuncs.pfnHookUserMsg( "AmmoInv", MsgAmmoInv );
}

void CAmmoInventory::setVisible( bool state )
{
	int i;

	m_iDropMode = 0;
	m_iSelected = 0;

	if ( m_pSelectedImage )
		m_pSelectedImage->setImage( NULL );

	if ( gEngfuncs.pfnGetLevelName() )
	{
		for ( i = 1; i <= MAX_AMMO_SLOTS; i++ )
		{
			if ( m_pButton[i] && m_pButton[i]->isVisible() )
				m_pButton[i]->cursorExited();
		}

		if ( m_pDropButton )
			m_pDropButton->cursorExited();

		if ( state )
		{
			for ( i = 1; i <= MAX_AMMO_SLOTS; i++ )
			{
				switch ( m_iAmmoType[i] )
				{
				case 1:
					if ( m_pSlotImage[i] )
						m_pSlotImage[i]->setImage( vgui_LoadTGA( "gfx/vgui/ammo/glock_slot.tga" ) );
					break;
				case 2:
					if ( m_pSlotImage[i] )
						m_pSlotImage[i]->setImage( vgui_LoadTGA( "gfx/vgui/ammo/m16_slot.tga" ) );
					break;
				case 3:
					if ( m_pSlotImage[i] )
						m_pSlotImage[i]->setImage( vgui_LoadTGA( "gfx/vgui/ammo/p345_slot.tga" ) );
					break;
				case 4:
					if ( m_pSlotImage[i] )
						m_pSlotImage[i]->setImage( vgui_LoadTGA( "gfx/vgui/ammo/shotgun_slot.tga" ) );
					break;
				case 5:
					if ( m_pSlotImage[i] )
						m_pSlotImage[i]->setImage( vgui_LoadTGA( "gfx/vgui/ammo/tmp_slot.tga" ) );
					break;
				}
			}
		}
	}

	Panel::setVisible( state );
}

void CAmmoInventory::solve()
{
	if ( isVisible() )
	{
		if ( m_iSelected && m_pButton[m_iSelected] && !m_pButton[m_iSelected]->isArmed() )
		{
			m_iSelected = 0;
			m_pSelectedImage->setImage( NULL );
		}

		for ( int i = 1; i <= MAX_AMMO_SLOTS; i++ )
		{
			if ( m_pButton[i] && m_pButton[i]->isArmed() && m_iSelected != i && m_iAmmoType[i] )
			{
				m_iSelected = i;

				switch ( m_iAmmoType[i] )
				{
				case 1:
					m_pSelectedImage->setImage( vgui_LoadTGA( "gfx/vgui/ammo/glock_check.tga" ) );
					break;
				case 2:
					m_pSelectedImage->setImage( vgui_LoadTGA( "gfx/vgui/ammo/m16_check.tga" ) );
					break;
				case 3:
					m_pSelectedImage->setImage( vgui_LoadTGA( "gfx/vgui/ammo/p345_check.tga" ) );
					break;
				case 4:
					m_pSelectedImage->setImage( vgui_LoadTGA( "gfx/vgui/ammo/shotgun_check.tga" ) );
					break;
				case 5:
					m_pSelectedImage->setImage( vgui_LoadTGA( "gfx/vgui/ammo/tmp_check.tga" ) );
					break;
				}
			}
		}
	}

	Panel::solve();
}

CAmmoInventory::~CAmmoInventory( void )
{
	delete m_pInventoryImage;
}
