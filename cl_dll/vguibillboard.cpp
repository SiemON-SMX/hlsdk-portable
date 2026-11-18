// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "vguibillboard.h"
#include "hudcontrol.h"
#include "../game_shared/vgui_loadtga.h"

CVGUIBillboard::CVGUIBillboard() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	Panel *pPanel;
	CommandButton *pButton;

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

	if ( !vgui_LoadTGA( "gfx/vgui/640_gustavdahlwelcome.tga" ) )
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_gustavdahlwelcome.tga!\n" );

	x = ScreenWidth / 2 - 320;
	y = ScreenHeight / 2 - 240;

	m_pImage = new ImagePanel( vgui_LoadTGA( "gfx/vgui/640_gustavdahlwelcome.tga" ) );
	m_pImage->setParent( this );
	m_pImage->setPos( x, y );
	m_pImage->setVisible( true );

	pButton = new CommandButton( "", x, y, 640, 480, true, true );
	pButton->setParent( this );
	pButton->addActionSignal( new CBillboardHandler_Command( this, 1 ) );

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	wide = 640;
	tall = 480;
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "credits" );
	SchemeHandle_t hChapterScheme = pSchemes->getSchemeHandle( "chaptertitle" );
	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "credits" );
	m_pFont = pSchemes->getFont( hTitleScheme );

	m_pLabel = new Label( " ", 0, 0, ScreenWidth, 64 );
	m_pLabel->setParent( this );
	m_pLabel->setFont( m_pFont );
	m_pLabel->setFgColor( 0, 0, 0, 0 );
	m_pLabel->setBgColor( 0, 0, 0, 255 );
	m_pLabel->setText( " " );
	m_pLabel->setContentAlignment( Label::a_center );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

void CVGUIBillboard::ActionSignal( int cmd )
{
	switch ( cmd )
	{
	case 0:
		break;
	case 1:
		setVisible( false );
		gViewPort->UpdateCursorState();
		break;
	default:
		gEngfuncs.Con_DPrintf( "Unknown Billboard ActionSignal %i.\n", cmd );
		break;
	}
}

void CVGUIBillboard::Initialize()
{
	m_pLabel->setText( "" );
	setVisible( false );
}

int CVGUIBillboard::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( pszCurrentBinding && !strcmp( pszCurrentBinding, "toggleconsole" ) )
		return 1;

	return 0;
}

int MsgBoard( const char *pszName, int iSize, void *pbuf )
{
	char szImage[128];

	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pVGUIBillboard )
	{
		sprintf( szImage, "%s", READ_STRING() );

		if ( strlen( szImage ) > 1 )
		{
			if ( vgui_LoadTGA( szImage ) )
				gViewPort->m_pVGUIBillboard->m_pImage->setImage( vgui_LoadTGA( szImage ) );
			else
				gEngfuncs.Con_Printf( "Cannot load %s!\n", szImage );
		}
		else
		{
			gEngfuncs.Con_Printf( "No message sent\n" );
		}

		if ( !gViewPort->m_pVGUIBillboard->isVisible() )
		{
			gViewPort->m_pVGUIBillboard->setVisible( true );
			gViewPort->UpdateCursorState();
		}
	}

	return 1;
}

void VGUI_BoardInit()
{
	gEngfuncs.pfnHookUserMsg( "Board", MsgBoard );
}

void CVGUIBillboard::paint()
{
	int x, y;
	int wide, tall;
	int r, g, b, a;

	if ( gViewPort && gViewPort->m_pHUDControl && gViewPort->m_pHUDControl->m_szMessage[0] )
	{
		if ( strlen( gViewPort->m_pHUDControl->m_szMessage ) > 1 )
		{
			gViewPort->m_pHUDControl->m_pMessageLabel->getPos( x, y );
			gViewPort->m_pHUDControl->m_pMessageLabel->getSize( wide, tall );
			gViewPort->m_pHUDControl->m_pMessageLabel->getFgColor( r, g, b, a );

			m_pLabel->setSize( wide, tall );
			m_pLabel->setText( gViewPort->m_pHUDControl->m_szMessage );
			m_pLabel->setPos( x, y );
			m_pLabel->setContentAlignment( Label::a_center );
			m_pLabel->setFgColor( r, g, b, a );
		}
	}

	Panel::paint();
}

void CVGUIBillboard::paintBackground()
{
	Panel::paintBackground();
}

void CVGUIBillboard::setVisible( bool state )
{
	Panel::setVisible( state );
}

CVGUIBillboard::~CVGUIBillboard( void )
{
	delete m_pBackgroundImage;
}
