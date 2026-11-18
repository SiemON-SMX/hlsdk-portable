// ======================================
// ======================================

#include <fstream>
#include <string>
#include <vector>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "notedocument.h"
#include "../game_shared/vgui_loadtga.h"

CNoteDocument::CNoteDocument( int iTrans, int iRemoveMe, int x, int y, int wide, int tall ) : CMenuPanel( iTrans, iRemoveMe, x, y, wide, tall )
{
	int iXPos, iYPos;
	int iWide, iTall;
	int iX, iY;
	Panel *pPanel;
	CommandButton *pButton;

	m_pLineLabel = NULL;
	Initialize();

	iXPos = ScreenWidth / 2 - 320;
	iYPos = ScreenHeight / 2 - 240;

	setBgColor( Color( 0, 0, 0, 0 ) );

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

	pPanel = new Panel( 0, 0, 0, 0 );
	pPanel->setParent( this );
	pPanel->setSize( ScreenWidth, ScreenHeight );
	pPanel->setBgColor( 0, 0, 0, 128 );
	pPanel->setVisible( true );

	m_pPanel = new CTransparentPanel( 0, iXPos, iYPos, 640, 480 );
	m_pPanel->setParent( this );
	m_pPanel->setPaintBorderEnabled( false );
	m_pPanel->setBgColor( Color( 0, 0, 0, 255 ) );

	m_pImageLabel = new CImageLabel( m_szImage1, -5, -5 );
	m_pImageLabel->setVisible( true );
	m_pImageLabel->setParent( m_pPanel );

	m_pCancelLabel = new CImageLabel( "notecancel", -5, -5 );
	m_pCancelLabel->setVisible( true );
	m_pCancelLabel->setParent( m_pPanel );

	m_pCloseButton = new CommandButton( "", 0, 0, 20, 20, true, true );
	m_pCloseButton->setParent( m_pPanel );
	m_pCloseButton->addActionSignal( new CNoteDocumentHandler_Command( this, 3 ) );

	pButton = new CommandButton( "", 38, 411, 33, 33, true, true );
	pButton->setParent( m_pPanel );
	pButton->addActionSignal( new CNoteDocumentHandler_Command( this, 1 ) );

	pButton = new CommandButton( "", 38, 447, 33, 33, true, true );
	pButton->setParent( m_pPanel );
	pButton->addActionSignal( new CNoteDocumentHandler_Command( this, 2 ) );

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hCreditsScheme = pSchemes->getSchemeHandle( "credits" );
	m_pFont = pSchemes->getFont( hCreditsScheme );

	m_pLineLabel = new Label( "", 0, 0, ScreenWidth, 64 );
	m_pLineLabel->setParent( this );
	m_pLineLabel->setFont( m_pFont );
	m_pLineLabel->setFgColor( 255, 255, 255, 0 );
	m_pLineLabel->setBgColor( 0, 0, 0, 255 );
	m_pLineLabel->setText( "" );
	m_pLineLabel->setContentAlignment( Label::a_center );
}

void CNoteDocument::ActionSignal( int cmd )
{
	switch ( cmd )
	{
	case 1:
		PrevLine();
		break;
	case 2:
		NextLine();
		break;
	case 3:
		gViewPort->HideTopMenu();
		gEngfuncs.pfnServerCmd( "closedocument" );
		break;
	}
}

void CNoteDocument::ClearString( char *str )
{
	for ( int i = 0; i < strlen( str ); i++ )
		str[i] = 0;
}

int CNoteDocument::CountLines()
{
	char szPath[256];

	strcpy( szPath, gEngfuncs.pfnGetGameDirectory() );
	strcat( szPath, "/notes/" );

	switch ( m_iPage )
	{
	case 1:
		strcat( szPath, m_szText1 );
		break;
	case 2:
		strcat( szPath, m_szText2 );
		break;
	case 3:
		strcat( szPath, m_szText3 );
		break;
	case 4:
		strcat( szPath, m_szText4 );
		break;
	}

	std::ifstream file( szPath );
	std::string line;
	std::vector<std::string> lines;

	while ( std::getline( file, line ) )
		lines.push_back( line );

	return lines.size();
}

void CNoteDocument::Initialize()
{
	if ( m_pLineLabel )
		m_pLineLabel->setText( "" );

	m_iLine = 0;
	m_iPage = 0;

	strncpy( m_szImage1, "", 16 );
	strncpy( m_szImage2, "", 16 );
	strncpy( m_szImage3, "", 16 );
	strncpy( m_szImage4, "", 16 );
	strncpy( m_szSound, "", 48 );
	strncpy( m_szText1, "", 16 );
	strncpy( m_szText2, "", 16 );
	strncpy( m_szText3, "", 16 );
	strncpy( m_szText4, "", 16 );
	strncpy( m_szLine, "", 128 );

	m_bTitle = false;
}

int CNoteDocument::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( pszCurrentBinding && !strcmp( pszCurrentBinding, "toggleconsole" ) )
		return 1;

	return 0;
}

int MsgNoteDoc( const char *pszName, int iSize, void *pbuf )
{
	char szText1[256];
	char szText2[256];
	char szText3[256];
	char szText4[256];

	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pNoteDocument )
	{
		gViewPort->m_pNoteDocument->ClearString( gViewPort->m_pNoteDocument->m_szImage1 );
		gViewPort->m_pNoteDocument->ClearString( gViewPort->m_pNoteDocument->m_szImage2 );
		gViewPort->m_pNoteDocument->ClearString( gViewPort->m_pNoteDocument->m_szImage3 );
		gViewPort->m_pNoteDocument->ClearString( gViewPort->m_pNoteDocument->m_szImage4 );
		gViewPort->m_pNoteDocument->ClearString( gViewPort->m_pNoteDocument->m_szText1 );
		gViewPort->m_pNoteDocument->ClearString( gViewPort->m_pNoteDocument->m_szSound );

		strcpy( gViewPort->m_pNoteDocument->m_szImage1, READ_STRING() );
		strcpy( gViewPort->m_pNoteDocument->m_szImage2, READ_STRING() );
		strcpy( gViewPort->m_pNoteDocument->m_szImage3, READ_STRING() );
		strcpy( gViewPort->m_pNoteDocument->m_szImage4, READ_STRING() );
		strcpy( szText1, READ_STRING() );
		strcpy( szText2, READ_STRING() );
		strcpy( szText3, READ_STRING() );
		strcpy( szText4, READ_STRING() );
		strcpy( gViewPort->m_pNoteDocument->m_szSound, READ_STRING() );

		int iLanguage = CVAR_GET_FLOAT( "cof_subtitlelanguage" );

		if ( strstr( szText1, "apnote" ) || strstr( szText1, "magazine" ) || strstr( szText1, "purnellnote" ) || strstr( szText1, "sewernote" ) || strstr( szText1, "subnote" ) )
		{
			switch ( iLanguage )
			{
			case 0:
			case 1:
				strcpy( gViewPort->m_pNoteDocument->m_szText1, szText1 );
				break;
			case 2:
				strcpy( gViewPort->m_pNoteDocument->m_szText1, "languages/dutch/" );
				strcat( gViewPort->m_pNoteDocument->m_szText1, szText1 );
				break;
			case 3:
				strcpy( gViewPort->m_pNoteDocument->m_szText1, "languages/french/" );
				strcat( gViewPort->m_pNoteDocument->m_szText1, szText1 );
				break;
			case 4:
				strcpy( gViewPort->m_pNoteDocument->m_szText1, "languages/german/" );
				strcat( gViewPort->m_pNoteDocument->m_szText1, szText1 );
				break;
			case 5:
				strcpy( gViewPort->m_pNoteDocument->m_szText1, "languages/norwegian/" );
				strcat( gViewPort->m_pNoteDocument->m_szText1, szText1 );
				break;
			case 6:
				strcpy( gViewPort->m_pNoteDocument->m_szText1, "languages/spanish/" );
				strcat( gViewPort->m_pNoteDocument->m_szText1, szText1 );
				break;
			case 7:
				strcpy( gViewPort->m_pNoteDocument->m_szText1, "languages/swedish/" );
				strcat( gViewPort->m_pNoteDocument->m_szText1, szText1 );
				break;
			}
		}

		if ( strstr( szText2, "apnote" ) || strstr( szText2, "magazine" ) || strstr( szText2, "purnellnote" ) || strstr( szText2, "sewernote" ) || strstr( szText2, "subnote" ) )
		{
			switch ( iLanguage )
			{
			case 0:
			case 1:
				strcpy( gViewPort->m_pNoteDocument->m_szText2, szText2 );
				break;
			case 2:
				strcpy( gViewPort->m_pNoteDocument->m_szText2, "languages/dutch/" );
				strcat( gViewPort->m_pNoteDocument->m_szText2, szText2 );
				break;
			case 3:
				strcpy( gViewPort->m_pNoteDocument->m_szText2, "languages/french/" );
				strcat( gViewPort->m_pNoteDocument->m_szText2, szText2 );
				break;
			case 4:
				strcpy( gViewPort->m_pNoteDocument->m_szText2, "languages/german/" );
				strcat( gViewPort->m_pNoteDocument->m_szText2, szText2 );
				break;
			case 5:
				strcpy( gViewPort->m_pNoteDocument->m_szText2, "languages/norwegian/" );
				strcat( gViewPort->m_pNoteDocument->m_szText2, szText2 );
				break;
			case 6:
				strcpy( gViewPort->m_pNoteDocument->m_szText2, "languages/spanish/" );
				strcat( gViewPort->m_pNoteDocument->m_szText2, szText2 );
				break;
			case 7:
				strcpy( gViewPort->m_pNoteDocument->m_szText2, "languages/swedish/" );
				strcat( gViewPort->m_pNoteDocument->m_szText2, szText2 );
				break;
			}
		}

		if ( strstr( szText3, "apnote" ) || strstr( szText3, "magazine" ) || strstr( szText3, "purnellnote" ) || strstr( szText3, "sewernote" ) || strstr( szText3, "subnote" ) )
		{
			switch ( iLanguage )
			{
			case 0:
			case 1:
				strcpy( gViewPort->m_pNoteDocument->m_szText3, szText3 );
				break;
			case 2:
				strcpy( gViewPort->m_pNoteDocument->m_szText3, "languages/dutch/" );
				strcat( gViewPort->m_pNoteDocument->m_szText3, szText3 );
				break;
			case 3:
				strcpy( gViewPort->m_pNoteDocument->m_szText3, "languages/french/" );
				strcat( gViewPort->m_pNoteDocument->m_szText3, szText3 );
				break;
			case 4:
				strcpy( gViewPort->m_pNoteDocument->m_szText3, "languages/german/" );
				strcat( gViewPort->m_pNoteDocument->m_szText3, szText3 );
				break;
			case 5:
				strcpy( gViewPort->m_pNoteDocument->m_szText3, "languages/norwegian/" );
				strcat( gViewPort->m_pNoteDocument->m_szText3, szText3 );
				break;
			case 6:
				strcpy( gViewPort->m_pNoteDocument->m_szText3, "languages/spanish/" );
				strcat( gViewPort->m_pNoteDocument->m_szText3, szText3 );
				break;
			case 7:
				strcpy( gViewPort->m_pNoteDocument->m_szText3, "languages/swedish/" );
				strcat( gViewPort->m_pNoteDocument->m_szText3, szText3 );
				break;
			}
		}

		if ( strstr( szText4, "apnote" ) || strstr( szText4, "magazine" ) || strstr( szText4, "purnellnote" ) || strstr( szText4, "sewernote" ) || strstr( szText4, "subnote" ) )
		{
			switch ( iLanguage )
			{
			case 0:
			case 1:
				strcpy( gViewPort->m_pNoteDocument->m_szText4, szText4 );
				break;
			case 2:
				strcpy( gViewPort->m_pNoteDocument->m_szText4, "languages/dutch/" );
				strcat( gViewPort->m_pNoteDocument->m_szText4, szText4 );
				break;
			case 3:
				strcpy( gViewPort->m_pNoteDocument->m_szText4, "languages/french/" );
				strcat( gViewPort->m_pNoteDocument->m_szText4, szText4 );
				break;
			case 4:
				strcpy( gViewPort->m_pNoteDocument->m_szText4, "languages/german/" );
				strcat( gViewPort->m_pNoteDocument->m_szText4, szText4 );
				break;
			case 5:
				strcpy( gViewPort->m_pNoteDocument->m_szText4, "languages/norwegian/" );
				strcat( gViewPort->m_pNoteDocument->m_szText4, szText4 );
				break;
			case 6:
				strcpy( gViewPort->m_pNoteDocument->m_szText4, "languages/spanish/" );
				strcat( gViewPort->m_pNoteDocument->m_szText4, szText4 );
				break;
			case 7:
				strcpy( gViewPort->m_pNoteDocument->m_szText4, "languages/swedish/" );
				strcat( gViewPort->m_pNoteDocument->m_szText4, szText4 );
				break;
			}
		}

		gViewPort->m_pNoteDocument->m_iPage = 1;
		gViewPort->m_pNoteDocument->SetPage( 1 );
	}

	gViewPort->ShowVGUIMenu( 31 );
	return 1;
}

void CNoteDocument::NextLine()
{
	char szPath[256];

	strcpy( szPath, gEngfuncs.pfnGetGameDirectory() );
	strcat( szPath, "/notes/" );

	switch ( m_iPage )
	{
	case 1:
		strcat( szPath, m_szText1 );
		break;
	case 2:
		strcat( szPath, m_szText2 );
		break;
	case 3:
		strcat( szPath, m_szText3 );
		break;
	case 4:
		strcat( szPath, m_szText4 );
		break;
	}

	std::ifstream file( szPath );
	std::string line;
	std::string strUnused;

	m_iLine++;

	if ( m_iLine > CountLines() )
		m_iLine = 1;

	line = m_iLine;

	int i = 1;

	while ( std::getline( file, line ) )
	{
		if ( i == m_iLine )
		{
			char szLine[256];
			strcpy( szLine, line.c_str() );
			szLine[line.length()] = 0;

			m_bTitle = ( i == 1 );
			strcpy( m_szLine, szLine );
		}

		i++;
	}
}

void CNoteDocument::NextPage()
{
	switch ( m_iPage )
	{
	case 1:
		if ( m_szImage2[1] )
		{
			m_iPage = 2;
			SetPage( 2 );
		}
		break;
	case 2:
		if ( m_szImage3[1] )
		{
			m_iPage = 3;
			SetPage( 3 );
			return;
		}

		m_iPage = 1;
		SetPage( 1 );
		break;
	case 3:
		if ( m_szImage4[1] )
		{
			m_iPage = 4;
			SetPage( 4 );
			return;
		}

		m_iPage = 1;
		SetPage( 1 );
		break;
	case 4:
		m_iPage = 1;
		SetPage( 1 );
		break;
	}
}

void NoteDocInit()
{
	gEngfuncs.pfnHookUserMsg( "NoteDoc", MsgNoteDoc );
}

void CNoteDocument::PrevLine()
{
	char szPath[256];

	strcpy( szPath, gEngfuncs.pfnGetGameDirectory() );
	strcat( szPath, "/notes/" );

	switch ( m_iPage )
	{
	case 1:
		strcat( szPath, m_szText1 );
		break;
	case 2:
		strcat( szPath, m_szText2 );
		break;
	case 3:
		strcat( szPath, m_szText3 );
		break;
	case 4:
		strcat( szPath, m_szText4 );
		break;
	}

	std::ifstream file( szPath );
	std::string line;
	std::string strUnused;

	m_iLine--;

	if ( m_iLine <= 0 )
		m_iLine = CountLines();

	line = m_iLine;

	int i = 1;

	while ( std::getline( file, line ) )
	{
		if ( i == m_iLine )
		{
			char szLine[256];
			strcpy( szLine, line.c_str() );
			szLine[line.length()] = 0;

			m_bTitle = ( i == 1 );
			strcpy( m_szLine, szLine );
		}

		i++;
	}
}

void CNoteDocument::PrevPage()
{
	switch ( m_iPage )
	{
	case 1:
		if ( m_szImage4[1] )
		{
			m_iPage = 4;
			SetPage( 4 );
			return;
		}
		if ( m_szImage3[1] )
		{
			m_iPage = 3;
			SetPage( 3 );
			return;
		}
		if ( m_szImage2[1] )
		{
			m_iPage = 2;
			SetPage( 2 );
			return;
		}
	case 2:
		if ( m_szImage1[1] && m_iPage != 1 )
		{
			m_iPage = 1;
			SetPage( 1 );
			return;
		}
		if ( m_szImage4[1] )
		{
			m_iPage = 4;
			SetPage( 4 );
			return;
		}
		if ( m_szImage3[1] )
		{
			m_iPage = 3;
			SetPage( 3 );
			return;
		}
	case 3:
		if ( m_szImage2[1] )
		{
			m_iPage = 2;
			SetPage( 2 );
			return;
		}
		if ( m_szImage1[1] )
		{
			m_iPage = 1;
			SetPage( 1 );
			return;
		}
		if ( m_szImage4[1] )
		{
			m_iPage = 4;
			SetPage( 4 );
			return;
		}
	case 4:
		if ( m_szImage3[1] )
		{
			m_iPage = 3;
			SetPage( 3 );
			return;
		}
		if ( m_szImage2[1] )
		{
			m_iPage = 2;
			SetPage( 2 );
			return;
		}
		if ( m_szImage1[1] )
		{
			m_iPage = 1;
			SetPage( 1 );
			return;
		}
	}
}

void CNoteDocument::SetPage( int page )
{
	BitmapTGA *pImage;

	switch ( page )
	{
	case 1:
		pImage = LoadTGAForRes( m_szImage1 );
		break;
	case 2:
		pImage = LoadTGAForRes( m_szImage2 );
		break;
	case 3:
		pImage = LoadTGAForRes( m_szImage3 );
		break;
	case 4:
		pImage = LoadTGAForRes( m_szImage4 );
		break;
	}

	if ( m_szSound[1] )
		gEngfuncs.pfnPlaySoundByName( m_szSound, 1 );

	m_iLine = 1;
	SetText();

	m_pImageLabel->setImage( pImage );
}

void CNoteDocument::SetText()
{
	char szPath[256];

	strcpy( szPath, gEngfuncs.pfnGetGameDirectory() );
	strcat( szPath, "/notes/" );

	switch ( m_iPage )
	{
	case 1:
		strcat( szPath, m_szText1 );
		break;
	case 2:
		strcat( szPath, m_szText2 );
		break;
	case 3:
		strcat( szPath, m_szText3 );
		break;
	case 4:
		strcat( szPath, m_szText4 );
		break;
	}

	std::ifstream file( szPath );
	std::string line;
	std::string strUnused;

	line = m_iLine;

	int i = 1;

	while ( std::getline( file, line ) )
	{
		if ( i == m_iLine )
		{
			char szLine[256];
			strcpy( szLine, line.c_str() );
			szLine[line.length()] = 0;

			m_bTitle = ( i == 1 );
			strcpy( m_szLine, szLine );
		}

		i++;
	}
}

void CNoteDocument::paint()
{
	if ( strlen( m_szLine ) > 1 )
	{
		int wide, tall;

		if ( m_szLine[0] == '*' )
			m_pLineLabel->setFgColor( 255, 255, 0, 0 );
		else if ( m_bTitle )
			m_pLineLabel->setFgColor( 255, 255, 0, 0 );
		else
			m_pLineLabel->setFgColor( 200, 200, 200, 0 );

		m_pFont->getTextSize( m_szLine, wide, tall );
		drawSetTextPos( ScreenWidth / 2 - wide / 2, ScreenHeight - tall * 2 );

		m_pLineLabel->setSize( ScreenWidth, 64 );
		m_pLineLabel->setText( m_szLine );
		m_pLineLabel->setPos( 0, ScreenHeight - tall * 3 );
		m_pLineLabel->setContentAlignment( Label::a_center );
	}

	Panel::paint();
}
