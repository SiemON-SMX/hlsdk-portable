// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "event_api.h"
#include "vgui_TeamFortressViewport.h"
#include "documents.h"
#include "../game_shared/vgui_loadtga.h"

CDocuments::CDocuments() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	int i;
	Panel *pPanel;
	CommandButton *pButton;

	pPanel = new Panel( 0, 0, ScreenWidth, ScreenHeight );
	pPanel->setParent( this );
	pPanel->setSize( ScreenWidth, ScreenHeight );
	pPanel->setPos( 0, 0 );
	pPanel->setBgColor( 0, 0, 0, 0 );
	pPanel->setVisible( true );

	m_pDocumentsImage = vgui_LoadTGA( "gfx/vgui/640_documents.tga" );
	if ( m_pDocumentsImage )
		m_pDocumentsImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_documents.tga!\n" );

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

	m_pDocumentsImage->getSize( wide, tall );
	x = ScreenWidth / 2 - wide / 2;
	y = ScreenHeight / 2 - tall / 2;

	m_pDocuments = new ImagePanel( m_pDocumentsImage );
	m_pDocuments->setParent( this );
	m_pDocuments->setPos( x, y );
	m_pDocuments->setVisible( true );

	y = 76;

	for ( i = 1; i <= MAX_DOCUMENTS; i++ )
	{
		m_pHiddenNoteImage[i] = vgui_LoadTGA( "gfx/vgui/notes/hiddennote1.tga" );
		if ( m_pHiddenNoteImage[i] )
			m_pHiddenNoteImage[i]->setColor( Color( 255, 255, 255, 1 ) );
		else
			gEngfuncs.Con_Printf( "Cannot load gfx/vgui/notes/hiddennote1.tga!\n" );

		m_pNote[i] = new ImagePanel( m_pHiddenNoteImage[i] );
		m_pNote[i]->setParent( m_pDocuments );
		m_pNote[i]->setPos( 230, y );
		m_pNote[i]->setVisible( false );

		m_pCodeDigit1[i] = new ImagePanel( NULL );
		m_pCodeDigit1[i]->setParent( m_pDocuments );
		m_pCodeDigit1[i]->setPos( 230, y );
		m_pCodeDigit1[i]->setVisible( false );

		m_pCodeDigit2[i] = new ImagePanel( NULL );
		m_pCodeDigit2[i]->setParent( m_pDocuments );
		m_pCodeDigit2[i]->setPos( 230, y );
		m_pCodeDigit2[i]->setVisible( false );

		m_pCodeDigit3[i] = new ImagePanel( NULL );
		m_pCodeDigit3[i]->setParent( m_pDocuments );
		m_pCodeDigit3[i]->setPos( 230, y );
		m_pCodeDigit3[i]->setVisible( false );

		m_pCodeDigit4[i] = new ImagePanel( NULL );
		m_pCodeDigit4[i]->setParent( m_pDocuments );
		m_pCodeDigit4[i]->setPos( 230, y );
		m_pCodeDigit4[i]->setVisible( false );

		m_pButton[i] = new CommandButton( "", 230, y, 422, 30, true, true );
		m_pButton[i]->setParent( m_pDocuments );
		m_pButton[i]->addActionSignal( new CDocumentsHandler_Command( this, i ) );

		y += 30;
	}

	pButton = new CommandButton( "", 0, 0, 20, 20, true, true );
	pButton->setParent( m_pDocuments );
	pButton->addActionSignal( new CDocumentsHandler_Command( this, 20 ) );

	pButton = new CommandButton( "", 0, 34, 33, 33, true, true );
	pButton->setParent( m_pDocuments );
	pButton->addActionSignal( new CDocumentsHandler_Command( this, 25 ) );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

CDocuments::~CDocuments()
{
	delete m_pDocumentsImage;
	delete m_pBackgroundImage;

	for ( int i = 1; i <= MAX_DOCUMENTS; i++ )
		delete m_pHiddenNoteImage[i];
}

void CDocuments::ActionSignal( int cmd )
{
	char szCommand[64];

	if ( cmd != 20 )
	{
		if ( cmd == 25 )
		{
			setVisible( false );
			gViewPort->UpdateCursorState();
			gViewPort->ShowVGUIMenu( 30 );
		}
		else if ( cmd >= 1 && cmd <= MAX_DOCUMENTS )
		{
			if ( strlen( gHUD.m_Notes[cmd].szLine[4] ) > 1 && !m_iHidden[cmd] )
			{
				setVisible( false );
				gViewPort->UpdateCursorState();
				sprintf( szCommand, "recalldocument %i", cmd );
				gEngfuncs.pfnServerCmd( szCommand );
			}
			else
			{
				gEngfuncs.pfnPlaySoundByName( "inventory/inventory_no.wav", 1 );
			}
		}
	}
	else
	{
		setVisible( false );
		gViewPort->UpdateCursorState();
	}
}

void CDocuments::Initialize()
{
	setVisible( false );
}

int CDocuments::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( pszCurrentBinding && !strcmp( pszCurrentBinding, "toggleconsole" ) )
		return 1;

	return 0;
}

void CDocuments::setVisible( bool state )
{
	if ( state )
	{
		for ( int i = 1; i <= MAX_DOCUMENTS; i++ )
		{
			if ( strlen( gHUD.m_Notes[i].szLine[4] ) > 1 )
				m_pNote[i]->setVisible( true );
			else
				m_pNote[i]->setVisible( false );
		}

		cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
		if ( pPlayer )
			gEngfuncs.pEventAPI->EV_PlaySound( 0, pPlayer->origin, CHAN_AUTO, "notes/note_turn1.wav", 1.0, 0.8, 0, 80 );
	}

	Panel::setVisible( state );
}
