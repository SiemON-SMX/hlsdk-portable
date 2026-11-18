// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "save.h"
#include "../game_shared/vgui_loadtga.h"

CSave::CSave() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int x, y;
	CommandButton *pButton;

	m_pImage = vgui_LoadTGA( "gfx/vgui/640_savegame.tga" );
	if ( m_pImage )
		m_pImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_panelback.tga!\n" );

	x = ScreenWidth / 2 - 160;
	y = ScreenHeight / 1.25 - 100;

	m_pImagePanel = new ImagePanel( m_pImage );
	m_pImagePanel->setParent( this );
	m_pImagePanel->setPos( x, y );
	m_pImagePanel->setVisible( true );

	pButton = new CommandButton( "", 36, 149, 79, 42, true, true );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CSaveHandler_Command( this, 2 ) );

	pButton = new CommandButton( "", 204, 149, 79, 42, true, true );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CSaveHandler_Command( this, 3 ) );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

void CSave::ActionSignal( int cmd )
{
	switch ( cmd )
	{
	case 0:
		break;
	case 1:
		gEngfuncs.pfnServerCmd( "unfreeze 666" );
		setVisible( false );
		gViewPort->UpdateCursorState();
		break;
	case 2:
		gEngfuncs.pfnServerCmd( "cofsave 19573" );
		gEngfuncs.pfnPlaySoundByName( "save/save_yes.wav", 1 );
		setVisible( false );
		gViewPort->UpdateCursorState();
		break;
	case 3:
		gEngfuncs.pfnServerCmd( "unfreeze 1024" );
		gEngfuncs.pfnPlaySoundByName( "save/save_no.wav", 1 );
		setVisible( false );
		gViewPort->UpdateCursorState();
		break;
	default:
		gEngfuncs.Con_DPrintf( "Unknown Save ActionSignal %i.\n", cmd );
		break;
	}
}

void CSave::Initialize()
{
	setVisible( false );
}

int MsgSave( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pSave )
	{
		if ( READ_BYTE() )
			gViewPort->m_pSave->setVisible( true );
		else
			gViewPort->m_pSave->setVisible( false );

		gViewPort->UpdateCursorState();
	}

	return 1;
}

void SaveInit()
{
	gEngfuncs.pfnHookUserMsg( "Save", MsgSave );
}

CSave::~CSave( void )
{
	delete m_pImage;
}
