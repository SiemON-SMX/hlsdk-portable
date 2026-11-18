// ======================================
// ======================================

#include <fstream>
#include <string>
#include <vector>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "credits.h"
#include "coopstats.h"
#include <stdarg.h>

extern int iMenuMode;
extern int g_iHeadShieldType;
extern cl_entity_t g_HeadShieldEnt;
extern int g_HeadShieldState;

#define SHIELD_ON	0
extern cvar_t *cl_fastcredits;
extern int iUnknown101902FC;

extern bool g_bHoodieLoaded;
extern bool g_bJeansLoaded;
extern bool g_bFaceLoaded;

void IrrKlangPlayMP3( const char *pszFile );
void IrrKlangSetLoop( bool bLoop );
void IrrKlangStop( float flFadeTime );

bool g_bUnlockShown;
int g_iUnlockClothes;
int g_iCreditsLetterboxHeight;
int g_iCoopCredits;
int iUnknown101B27C8;

CCredits::CCredits() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int i;
	int y;

	setBgColor( 255, 255, 255, 0 );

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hCreditsScheme = pSchemes->getSchemeHandle( "credits" );
	SchemeHandle_t hCreditsTitleScheme = pSchemes->getSchemeHandle( "creditstitle" );
	m_pCreditsFont = pSchemes->getFont( hCreditsScheme );
	m_pCreditsTitleFont = pSchemes->getFont( hCreditsTitleScheme );

	m_bScrollUp = true;

	for ( i = 1; i <= MAX_CREDIT_LINES; i++ )
	{
		y = ScreenHeight;
		m_pCreditLabel[i] = new Label( " ", 0, y + 64 + i * 32, ScreenWidth, 32 );
		m_pCreditLabel[i]->setParent( this );
		m_pCreditLabel[i]->setFont( m_pCreditsFont );
		m_pCreditLabel[i]->setFgColor( 0, 0, 0, 255 );
		m_pCreditLabel[i]->setBgColor( 0, 0, 0, 255 );
		m_pCreditLabel[i]->setText( " " );
		m_pCreditLabel[i]->setContentAlignment( Label::a_center );
	}

	y = ScreenHeight / 2;

	for ( i = 1; i <= 14; i++ )
	{
		m_pStatLabel[i] = new Label( " ", 0, y - 256 + i * 32, ScreenWidth, 32 );
		m_pStatLabel[i]->setParent( this );
		m_pStatLabel[i]->setFont( m_pCreditsFont );
		m_pStatLabel[i]->setFgColor( 0, 0, 0, 255 );
		m_pStatLabel[i]->setBgColor( 0, 0, 0, 255 );
		m_pStatLabel[i]->setText( " " );
		m_pStatLabel[i]->setContentAlignment( Label::a_center );
		m_pStatLabel[i]->setVisible( false );
	}

	m_pGradeLabel = new Label( " ", 0, y + 256, ScreenWidth, 42 );
	m_pGradeLabel->setParent( this );
	m_pGradeLabel->setFont( pSchemes->getFont( pSchemes->getSchemeHandle( "chaptertitle" ) ) );
	m_pGradeLabel->setFgColor( 0, 0, 0, 255 );
	m_pGradeLabel->setBgColor( 0, 0, 0, 255 );
	m_pGradeLabel->setText( " " );
	m_pGradeLabel->setContentAlignment( Label::a_center );
	m_pGradeLabel->setVisible( false );

	m_pGradeShadowLabel = new Label( " ", 0, y + 256, ScreenWidth, 42 );
	m_pGradeShadowLabel->setParent( this );
	m_pGradeShadowLabel->setFont( pSchemes->getFont( pSchemes->getSchemeHandle( "chaptertitle" ) ) );
	m_pGradeShadowLabel->setFgColor( 0, 0, 0, 255 );
	m_pGradeShadowLabel->setBgColor( 0, 0, 0, 255 );
	m_pGradeShadowLabel->setText( " " );
	m_pGradeShadowLabel->setContentAlignment( Label::a_center );
	m_pGradeShadowLabel->setVisible( false );

	m_pStatsTitleLabel = new Label( "Statistics and grade", 0, y - 266, ScreenWidth, 42 );
	m_pStatsTitleLabel->setParent( this );
	m_pStatsTitleLabel->setFont( m_pCreditsTitleFont );
	m_pStatsTitleLabel->setFgColor( 0, 0, 0, 255 );
	m_pStatsTitleLabel->setBgColor( 0, 0, 0, 255 );
	m_pStatsTitleLabel->setText( "Statistics and grade" );
	m_pStatsTitleLabel->setContentAlignment( Label::a_center );
	m_pStatsTitleLabel->setVisible( false );

	m_pUnlockLabel = new Label( "Unlocks", 0, y - 266, ScreenWidth, 42 );
	m_pUnlockLabel->setParent( this );
	m_pUnlockLabel->setFont( m_pCreditsTitleFont );
	m_pUnlockLabel->setFgColor( 0, 0, 0, 255 );
	m_pUnlockLabel->setBgColor( 0, 0, 0, 255 );
	m_pUnlockLabel->setText( "Unlocks" );
	m_pUnlockLabel->setContentAlignment( Label::a_center );
	m_pUnlockLabel->setVisible( false );

	g_iCreditsLetterboxHeight = ScreenHeight / 480.0f * 25.0f + 0.5f;

	m_pLetterboxTop = new Panel( 0, 0, ScreenWidth, g_iCreditsLetterboxHeight );
	m_pLetterboxTop->setParent( this );
	m_pLetterboxTop->setSize( ScreenWidth, g_iCreditsLetterboxHeight );
	m_pLetterboxTop->setBgColor( 0, 0, 0, 0 );
	m_pLetterboxTop->setVisible( true );

	m_pLetterboxBottom = new Panel( 0, ScreenHeight - g_iCreditsLetterboxHeight, ScreenWidth, g_iCreditsLetterboxHeight );
	m_pLetterboxBottom->setParent( this );
	m_pLetterboxBottom->setSize( ScreenWidth, g_iCreditsLetterboxHeight );
	m_pLetterboxBottom->setBgColor( 0, 0, 0, 0 );
	m_pLetterboxBottom->setVisible( true );

	Initialize();
}

int CCredits::CountLines()
{
	std::ifstream file( m_szFile );
	std::string line;
	std::vector<std::string> lines;

	while ( std::getline( file, line ) )
		lines.push_back( line );

	return lines.size();
}

void CCredits::Initialize()
{
	g_iUnlockClothes = 0;
	setVisible( false );
	m_bShowStats = false;
	m_bUnlocks = false;
	m_iUnlocks = 0;
}

int MsgCredits( const char *pszName, int iSize, void *pbuf )
{
	char szPath[256];
	char szFile[256];
	char szEnding[256];

	BEGIN_READ( pbuf, iSize );

	if ( !gViewPort->m_pCredits )
		return 1;

	gViewPort->m_pCredits->m_szUnlockText[0] = '\0';
	g_iUnlockClothes = 0;
	gViewPort->m_pCredits->m_bUnlocks = false;
	gViewPort->m_pCredits->m_iUnlocks = 0;
	gViewPort->m_pCredits->m_pStatsTitleLabel->setVisible( false );
	gViewPort->m_pCredits->m_pGradeLabel->setVisible( false );
	gViewPort->m_pCredits->m_pGradeShadowLabel->setVisible( false );
	gViewPort->m_pCredits->setVisible( true );

	int iTextOnly = READ_BYTE();
	bool bConclusion = false;
	int iLanguage;

	if ( iTextOnly )
	{
		strcpy( szFile, READ_STRING() );

		if ( strstr( szFile, "coconc.txt" ) || strstr( szFile, "conclusion1.txt" ) || strstr( szFile, "conclusion2.txt" ) ||
			strstr( szFile, "conclusion3.txt" ) || strstr( szFile, "conclusion4.txt" ) || strstr( szFile, "docconc.txt" ) )
		{
			bConclusion = true;
			strcpy( gViewPort->m_pCredits->m_szFile, szFile );
		}
		else if ( !strstr( szFile, gEngfuncs.pfnGetGameDirectory() ) )
		{
			strcpy( szPath, gEngfuncs.pfnGetGameDirectory() );
			strcat( szPath, szFile );
			strcpy( gViewPort->m_pCredits->m_szFile, szPath );
		}
		else
		{
			strcpy( gViewPort->m_pCredits->m_szFile, szFile );
		}
	}
	else
	{
		strcpy( gViewPort->m_pCredits->m_szFile, READ_STRING() );
	}

	iLanguage = gEngfuncs.pfnGetCvarFloat( "cof_subtitlelanguage" );

	if ( !iTextOnly || bConclusion == true )
	{
		switch ( iLanguage )
		{
		case 2:
			if ( strstr( gViewPort->m_pCredits->m_szFile, "coconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/dutch/docconc.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion1.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/dutch/conclusion1.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion2.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/dutch/conclusion2.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion3.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/dutch/conclusion3.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion4.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/dutch/conclusion4.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "docconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/dutch/docconc.txt" );
			}
			break;
		case 3:
			if ( strstr( gViewPort->m_pCredits->m_szFile, "coconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/french/docconc.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion1.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/french/conclusion1.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion2.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/french/conclusion2.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion3.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/french/conclusion3.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion4.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/french/conclusion4.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "docconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/french/docconc.txt" );
			}
			break;
		case 4:
			if ( strstr( gViewPort->m_pCredits->m_szFile, "coconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/german/docconc.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion1.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/german/conclusion1.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion2.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/german/conclusion2.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion3.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/german/conclusion3.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion4.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/german/conclusion4.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "docconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/german/docconc.txt" );
			}
			break;
		case 5:
			if ( strstr( gViewPort->m_pCredits->m_szFile, "coconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/norwegian/docconc.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion1.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/norwegian/conclusion1.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion2.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/norwegian/conclusion2.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion3.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/norwegian/conclusion3.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion4.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/norwegian/conclusion4.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "docconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/norwegian/docconc.txt" );
			}
			break;
		case 6:
			if ( strstr( gViewPort->m_pCredits->m_szFile, "coconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/spanish/docconc.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion1.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/spanish/conclusion1.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion2.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/spanish/conclusion2.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion3.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/spanish/conclusion3.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion4.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/spanish/conclusion4.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "docconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/spanish/docconc.txt" );
			}
			break;
		case 7:
			if ( strstr( gViewPort->m_pCredits->m_szFile, "coconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/swedish/docconc.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion1.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/swedish/conclusion1.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion2.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/swedish/conclusion2.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion3.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/swedish/conclusion3.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "conclusion4.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/swedish/conclusion4.txt" );
			}
			else if ( strstr( gViewPort->m_pCredits->m_szFile, "docconc.txt" ) )
			{
				strcpy( gViewPort->m_pCredits->m_szFile, gEngfuncs.pfnGetGameDirectory() );
				strcat( gViewPort->m_pCredits->m_szFile, "/txtfiles/languages/swedish/docconc.txt" );
			}
			break;
		}
	}

	gViewPort->m_pCredits->m_iNumLines = gViewPort->m_pCredits->CountLines();
	gViewPort->m_pCredits->m_bStarted = true;

	int iStartY = 0;
	if ( !gViewPort->m_pCredits->m_bScrollUp )
		iStartY = ScreenHeight;

	gViewPort->m_pCredits->m_iBrightness = 0;
	gViewPort->m_pCredits->m_bFadeIn = false;
	gViewPort->m_pCredits->m_bFadeDone = false;
	gViewPort->m_pCredits->m_bStatsTimer = false;
	gViewPort->m_pCredits->m_flStatsTime = 0;

	int i;

	for ( i = 1; i <= gViewPort->m_pCredits->m_iNumLines; i++ )
	{
		gViewPort->m_pCredits->SetLine( i, i );

		int iOffset = i * 32;
		if ( gViewPort->m_pCredits->m_bScrollUp )
			iOffset = -iOffset;

		gViewPort->m_pCredits->m_pCreditLabel[i]->setPos( 0, iStartY + iOffset );
		gViewPort->m_pCredits->m_pCreditLabel[i]->setVisible( true );
	}

	gViewPort->m_pCredits->m_iShotsFired = READ_SHORT();
	gViewPort->m_pCredits->m_iShotsHit = READ_SHORT();
	gViewPort->m_pCredits->m_iSyringesUsed = READ_SHORT();
	gViewPort->m_pCredits->m_iShotsMissed = READ_SHORT();
	gViewPort->m_pCredits->m_flAccuracy = READ_COORD();
	gViewPort->m_pCredits->m_iTimesSaved = READ_SHORT();
	gViewPort->m_pCredits->m_iWeaponsFound = READ_SHORT();
	gViewPort->m_pCredits->m_iDamageTaken = READ_SHORT();
	gViewPort->m_pCredits->m_flDistanceDecimal = READ_SHORT();
	gViewPort->m_pCredits->m_flDistance = READ_SHORT();
	gViewPort->m_pCredits->m_iSeconds = READ_SHORT();
	gViewPort->m_pCredits->m_iMinutes = READ_SHORT();
	gViewPort->m_pCredits->m_iHours = READ_SHORT();
	gViewPort->m_pCredits->m_iMeleeKills = READ_SHORT();
	gViewPort->m_pCredits->m_iFirearmKills = READ_SHORT();
	READ_BYTE();
	gViewPort->m_pCredits->m_iScrollSpeed = READ_BYTE();
	int iHide = READ_BYTE();
	gViewPort->m_pCredits->m_iDifficulty = READ_BYTE();
	gViewPort->m_pCredits->m_iUnlocks = READ_LONG();
	int iEnding = READ_BYTE();
	int iGrade = READ_BYTE();
	int iCoop = READ_BYTE();
	int iStatsTimer = READ_BYTE();
	g_iCoopCredits = READ_BYTE();
	int iEnding6 = READ_BYTE();
	int iEnding7 = READ_BYTE();
	READ_BYTE();
	READ_BYTE();
	int iShowGrade = READ_BYTE();

	if ( gViewPort->m_pCredits->m_iDifficulty < 1 || gViewPort->m_pCredits->m_iDifficulty > 4 )
		gViewPort->m_pCredits->m_iDifficulty = 1;

	if ( iHide )
		gViewPort->m_pCredits->setVisible( false );

	if ( iTextOnly )
	{
		gViewPort->m_pCredits->m_bTextOnly = true;
		gViewPort->m_pCredits->m_bScrollUp = false;

		for ( i = 1; i <= MAX_CREDIT_LINES; i++ )
			gViewPort->m_pCredits->m_pCreditLabel[i]->setPos( 0, ScreenHeight + i * 32 );
	}
	else
	{
		gViewPort->m_pCredits->m_bScrollUp = true;
		gViewPort->m_pCredits->m_bTextOnly = false;
	}

	gViewPort->m_pCredits->m_bShowStats = false;
	gViewPort->m_pCredits->m_flStartTime = gEngfuncs.GetClientTime();

	gViewPort->m_pCredits->m_pStatLabel[1]->setText( UTIL_VarArgs( "Shots fired: %i", gViewPort->m_pCredits->m_iShotsFired ) );
	gViewPort->m_pCredits->m_pStatLabel[2]->setText( UTIL_VarArgs( "Shots hit: %i", gViewPort->m_pCredits->m_iShotsHit ) );
	gViewPort->m_pCredits->m_pStatLabel[3]->setText( UTIL_VarArgs( "Shots missed: %i", gViewPort->m_pCredits->m_iShotsMissed ) );
	gViewPort->m_pCredits->m_pStatLabel[4]->setText( UTIL_VarArgs( "Overall accuracy: %i percent", (int)gViewPort->m_pCredits->m_flAccuracy ) );

	if ( iCoop )
	{
		gViewPort->m_pCredits->m_pStatLabel[5]->setText( UTIL_VarArgs( "Overall damage taken: %i", gViewPort->m_pCredits->m_iDamageTaken ) );
		gViewPort->m_pCredits->m_pStatLabel[6]->setText( UTIL_VarArgs( "Distance travelled on foot: %i.%i km", (int)gViewPort->m_pCredits->m_flDistance, (int)gViewPort->m_pCredits->m_flDistanceDecimal ) );
		gViewPort->m_pCredits->m_pStatLabel[7]->setText( UTIL_VarArgs( "Time taken: %i:%i:%i", gViewPort->m_pCredits->m_iHours, gViewPort->m_pCredits->m_iMinutes, gViewPort->m_pCredits->m_iSeconds ) );
		gViewPort->m_pCredits->m_pStatLabel[8]->setText( UTIL_VarArgs( "Enemies killed by melee: %i", gViewPort->m_pCredits->m_iMeleeKills ) );
		gViewPort->m_pCredits->m_pStatLabel[9]->setText( UTIL_VarArgs( "Enemies killed by firearms: %i", gViewPort->m_pCredits->m_iFirearmKills ) );
		gViewPort->m_pCredits->m_pStatLabel[10]->setText( "" );
		gViewPort->m_pCredits->m_pStatLabel[11]->setText( "" );
		gViewPort->m_pCredits->m_pStatLabel[12]->setText( "" );
	}
	else
	{
		gViewPort->m_pCredits->m_pStatLabel[5]->setText( UTIL_VarArgs( "Syringes used: %i", gViewPort->m_pCredits->m_iSyringesUsed ) );
		gViewPort->m_pCredits->m_pStatLabel[6]->setText( UTIL_VarArgs( "Times saved: %i", gViewPort->m_pCredits->m_iTimesSaved ) );
		gViewPort->m_pCredits->m_pStatLabel[7]->setText( UTIL_VarArgs( "Weapons found: %i", gViewPort->m_pCredits->m_iWeaponsFound ) );
		gViewPort->m_pCredits->m_pStatLabel[8]->setText( UTIL_VarArgs( "Overall damage taken: %i", gViewPort->m_pCredits->m_iDamageTaken ) );
		gViewPort->m_pCredits->m_pStatLabel[9]->setText( UTIL_VarArgs( "Distance travelled on foot: %i.%i km", (int)gViewPort->m_pCredits->m_flDistance, (int)gViewPort->m_pCredits->m_flDistanceDecimal ) );
		gViewPort->m_pCredits->m_pStatLabel[10]->setText( UTIL_VarArgs( "Time taken: %i:%i:%i", gViewPort->m_pCredits->m_iHours, gViewPort->m_pCredits->m_iMinutes, gViewPort->m_pCredits->m_iSeconds ) );
		gViewPort->m_pCredits->m_pStatLabel[11]->setText( UTIL_VarArgs( "Enemies killed by melee: %i", gViewPort->m_pCredits->m_iMeleeKills ) );
		gViewPort->m_pCredits->m_pStatLabel[12]->setText( UTIL_VarArgs( "Enemies killed by firearms: %i", gViewPort->m_pCredits->m_iFirearmKills ) );
	}

	switch ( gViewPort->m_pCredits->m_iDifficulty )
	{
	case 1:
		gViewPort->m_pCredits->m_pStatLabel[13]->setText( UTIL_VarArgs( "Difficulty chosen: Easy" ) );
		break;
	case 2:
		gViewPort->m_pCredits->m_pStatLabel[13]->setText( UTIL_VarArgs( "Difficulty chosen: Normal" ) );
		break;
	case 3:
		gViewPort->m_pCredits->m_pStatLabel[13]->setText( UTIL_VarArgs( "Difficulty chosen: Hard" ) );
		break;
	case 4:
		gViewPort->m_pCredits->m_pStatLabel[13]->setText( UTIL_VarArgs( "Difficulty chosen: Nightmare" ) );
		break;
	default:
		gViewPort->m_pCredits->m_pStatLabel[13]->setText( UTIL_VarArgs( "Difficulty chosen: Easy" ) );
		break;
	}

	if ( iEnding6 && iCoop )
		iEnding = 6;

	if ( iEnding7 && g_iCoopCredits )
	{
		strcpy( szEnding, "Ending seen: " );
		strcat( szEnding, "7" );
	}
	else
	{
		strcpy( szEnding, "Ending seen: " );

		switch ( iEnding )
		{
		case 1:
			strcat( szEnding, "1" );
			break;
		case 2:
			strcat( szEnding, "2" );
			break;
		case 3:
			strcat( szEnding, "3" );
			break;
		case 4:
			strcat( szEnding, "4" );
			break;
		case 5:
			strcat( szEnding, "5" );
			break;
		case 6:
			strcat( szEnding, "6" );
			break;
		case 7:
			strcat( szEnding, "7" );
			break;
		}
	}

	gViewPort->m_pCredits->m_pStatLabel[14]->setText( szEnding );

	for ( i = 1; i <= 14; i++ )
	{
		gViewPort->m_pCredits->m_pStatLabel[i]->setContentAlignment( Label::a_center );
		gViewPort->m_pCredits->m_pStatLabel[i]->setVisible( false );
	}

	switch ( iGrade )
	{
	case 1:
		gViewPort->m_pCredits->m_pGradeLabel->setText( "S" );
		gViewPort->m_pCredits->m_pGradeShadowLabel->setText( "S" );
		break;
	case 2:
		gViewPort->m_pCredits->m_pGradeLabel->setText( "A" );
		gViewPort->m_pCredits->m_pGradeShadowLabel->setText( "A" );
		break;
	case 3:
		gViewPort->m_pCredits->m_pGradeLabel->setText( "B" );
		gViewPort->m_pCredits->m_pGradeShadowLabel->setText( "B" );
		break;
	case 4:
		gViewPort->m_pCredits->m_pGradeLabel->setText( "C" );
		gViewPort->m_pCredits->m_pGradeShadowLabel->setText( "C" );
		break;
	case 5:
		gViewPort->m_pCredits->m_pGradeLabel->setText( "D" );
		gViewPort->m_pCredits->m_pGradeShadowLabel->setText( "D" );
		break;
	case 6:
		gViewPort->m_pCredits->m_pGradeLabel->setText( "E" );
		gViewPort->m_pCredits->m_pGradeShadowLabel->setText( "E" );
		break;
	default:
		gViewPort->m_pCredits->m_pGradeLabel->setText( "F" );
		gViewPort->m_pCredits->m_pGradeShadowLabel->setText( "F" );
		break;
	}

	if ( !iShowGrade )
	{
		gViewPort->m_pCredits->m_pGradeLabel->setText( " " );
		gViewPort->m_pCredits->m_pGradeShadowLabel->setText( " " );
		gViewPort->m_pCredits->m_pStatLabel[14]->setText( " " );
	}

	if ( iStatsTimer )
	{
		gViewPort->m_pCredits->m_bStatsTimer = true;
		gViewPort->m_pCredits->m_flStatsTime = gEngfuncs.GetClientTime() + 0.2;
	}

	return 1;
}

void CCredits::SetLine( int iUnused, int iLine )
{
	std::ifstream file( m_szFile );
	std::string line;
	std::string line2;
	line = iLine;
	int i = 1;
	char szLine[256];

	while ( std::getline( file, line ) )
	{
		if ( i == iLine )
		{
			strcpy( szLine, line.c_str() );
			szLine[line.length()] = 0;

			char *pszText = szLine;

			if ( szLine[0] == '#' )
			{
				pszText = &szLine[1];
				m_pCreditLabel[i]->setFont( m_pCreditsTitleFont );
			}
			else
			{
				m_pCreditLabel[i]->setFont( m_pCreditsFont );
			}

			m_pCreditLabel[i]->setText( pszText );
			m_pCreditLabel[i]->setContentAlignment( Label::a_center );
		}

		i++;
	}
}

void CCredits::ShowUnlock( int iUnlock, const char *pszText, int iDistance, char *pszModel, int iBody )
{
	int i;

	iUnknown101902FC = iDistance;
	m_iUnlocks &= ~iUnlock;
	m_bContinue = false;

	for ( i = 1; i <= 14; i++ )
	{
		if ( m_pStatLabel[i]->isVisible() )
			m_pStatLabel[i]->setVisible( false );
	}

	for ( i = 1; i <= m_iNumLines; i++ )
	{
		if ( m_pCreditLabel[i]->isVisible() )
			m_pCreditLabel[i]->setVisible( false );
	}

	if ( m_pGradeLabel->isVisible() )
		m_pGradeLabel->setVisible( false );
	if ( m_pGradeShadowLabel->isVisible() )
		m_pGradeShadowLabel->setVisible( false );
	if ( m_pStatsTitleLabel->isVisible() )
		m_pStatsTitleLabel->setVisible( false );

	strcpy( m_szUnlockText, pszText );

	if ( g_iHeadShieldType != 6 )
		g_iHeadShieldType = 6;

	iUnknown101902FC = 50;

	if ( !strcmp( pszModel, "models/items/generic_note.mdl" ) || !strcmp( pszModel, "models/weapons/axe/w_axe.mdl" ) ||
		!strcmp( pszModel, "models/weapons/famas/w_famas.mdl" ) || !strcmp( pszModel, "models/items/package.mdl" ) )
	{
		iUnknown101B27C8 = 90;
	}
	else if ( !strcmp( pszModel, "models/items/cassette_tape.mdl" ) )
	{
		iUnknown101902FC = 20;
		iUnknown101B27C8 = 90;
	}
	else
	{
		if ( !strcmp( pszModel, "models/doctor_unlock.mdl" ) )
			iUnknown101902FC = 80;

		iUnknown101B27C8 = 0;
	}

	gHUD.m_pszHeadShieldSound = pszModel;
	gHUD.m_iHeadShieldSoundFlags = iBody;
	g_HeadShieldState = SHIELD_ON;
}

char *UTIL_VarArgs( const char *format, ... )
{
	va_list argptr;
	static char string[1024];

	va_start( argptr, format );
	vsprintf( string, format, argptr );
	va_end( argptr );

	return string;
}

void VGUI_CreditsInit()
{
	gEngfuncs.pfnHookUserMsg( "Credits", MsgCredits );
}

void CCredits::solve()
{
	static bool s_bLetterboxFrame = false;
	static bool s_bFadeFrame = true;
	int i;

	int iSpeed = g_iCreditsLetterboxHeight / 15;
	if ( iSpeed <= 0 )
		iSpeed = 1;

	if ( gHUD.m_BlackBar.m_iOn )
	{
		s_bLetterboxFrame = !s_bLetterboxFrame;
		if ( s_bLetterboxFrame )
		{
			int x, y, x2, y2;

			m_pLetterboxTop->getPos( x, y );
			m_pLetterboxBottom->getPos( x2, y2 );

			if ( y < 0 )
				y += iSpeed;
			if ( y > 0 )
				y = 0;
			if ( y2 > ScreenHeight - g_iCreditsLetterboxHeight )
				y2 -= iSpeed;
			if ( y2 < ScreenHeight - g_iCreditsLetterboxHeight )
				y2 = ScreenHeight - g_iCreditsLetterboxHeight;

			m_pLetterboxTop->setPos( x, y );
			m_pLetterboxBottom->setPos( x2, y2 );
		}
	}
	else
	{
		s_bLetterboxFrame = !s_bLetterboxFrame;
		if ( s_bLetterboxFrame )
		{
			int x, y, x2, y2;

			m_pLetterboxTop->getPos( x, y );
			m_pLetterboxBottom->getPos( x2, y2 );

			if ( y > -g_iCreditsLetterboxHeight )
				y -= iSpeed;
			if ( y < -g_iCreditsLetterboxHeight )
				y = -g_iCreditsLetterboxHeight;
			if ( y2 < ScreenHeight )
				y2 += iSpeed;
			if ( y2 > ScreenHeight )
				y2 = ScreenHeight;

			m_pLetterboxTop->setPos( x, y );
			m_pLetterboxBottom->setPos( x2, y2 );
		}
	}

	if ( m_bStatsTimer == true && !m_bTextOnly )
	{
		float flTime = gEngfuncs.GetClientTime();

		if ( flTime > m_flStatsTime && !m_bShowStats )
		{
			IrrKlangSetLoop( true );
			IrrKlangPlayMP3( "stats.mp3" );
			m_bShowStats = true;
			m_pLetterboxBottom->setVisible( false );
			m_pLetterboxTop->setVisible( false );

			if ( g_iCoopCredits && gViewPort->m_pCoopStats )
			{
				setVisible( false );
				gViewPort->m_pCoopStats->setVisible( true );
			}
		}
	}

	s_bFadeFrame = !s_bFadeFrame;
	if ( !s_bFadeFrame )
	{
		Panel::solve();
		return;
	}

	if ( strlen( m_szUnlockText ) > 2 )
	{
		int r, g, b, a;

		m_pUnlockLabel->getFgColor( r, g, b, a );
		if ( a > 0 )
			a -= 15;
		if ( a < 0 )
			a = 0;

		m_pUnlockLabel->setText( m_szUnlockText );
		m_pUnlockLabel->setVisible( true );
		m_pUnlockLabel->setPos( 0, ScreenWidth / 2 );
		m_pUnlockLabel->setFgColor( 255, 255, 255, a );
	}

	if ( m_bUnlocks )
	{
		if ( m_iUnlocks & 0x40 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x40, "Unlocked: FAMAS with infinite ammo", 50, "models/weapons/famas/w_famas.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x80 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x80, "Unlocked: David Leatherhoff suit", 50, "models/costumes/unfolded_clothes.mdl", 1 );
				g_iUnlockClothes = 1;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x100 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x100, "Unlocked: ModDB hoodie", 50, "models/costumes/unfolded_clothes.mdl", 0 );
				g_iUnlockClothes = 2;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x200 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x200, "Unlocked: Hello Kitty suit", 50, "models/costumes/unfolded_clothes.mdl", 1 );
				g_iUnlockClothes = 3;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x400 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x400, "Unlocked: Afraid of Monsters suit", 50, "models/costumes/unfolded_clothes.mdl", 1 );
				g_iUnlockClothes = 4;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x800 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x800, "Unlocked: Camouflage hoodie", 50, "models/costumes/unfolded_clothes.mdl", 0 );
				g_iUnlockClothes = 5;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x1000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x1000, "Unlocked: Half Life Creations hoodie", 50, "models/costumes/unfolded_clothes.mdl", 0 );
				g_iUnlockClothes = 6;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x2000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x2000, "Unlocked: Black Metal suit", 50, "models/costumes/unfolded_clothes.mdl", 1 );
				g_iUnlockClothes = 7;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x4000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x4000, "Unlocked: Team Psykskallar hoodie", 50, "models/costumes/unfolded_clothes.mdl", 0 );
				g_iUnlockClothes = 8;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x8000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x8000, "Unlocked: Fuck Anime suit", 50, "models/costumes/unfolded_clothes.mdl", 1 );
				g_iUnlockClothes = 9;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x10000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x10000, "Unlocked: Sick Simon suit", 50, "models/costumes/unfolded_clothes.mdl", 1 );
				g_iUnlockClothes = 10;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x20000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x20000, "Unlocked: AoM Twitcher suit", 50, "models/costumes/unfolded_clothes.mdl", 1 );
				g_iUnlockClothes = 11;
				g_bHoodieLoaded = false;
				g_bJeansLoaded = false;
				g_bFaceLoaded = false;
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x40000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x40000, "Unlocked: Custom hoodies", 50, "models/costumes/unfolded_clothes.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x80000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x80000, "Unlocked: AoMDC axe", 50, "models/weapons/axe/w_axe.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x100000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x100000, "Unlocked: Camera of Death", 50, "models/weapons/camera/w_camera.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x200000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x200000, "Unlocked: Simon\'s Book", 50, "models/weapons/booklaser/w_booklaser.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x400000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x400000, "Unlocked: Developer commentaries", 50, "models/developer_commentary.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x800000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x800000, "Unlocked: Secret package", 50, "models/items/package.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x20 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x20, "Unlocked: Nightmare mode", 50, "models/items/cassette_tape.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x1000000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x1000000, "Unlocked: Nightvision", 50, "models/items/w_nightvision.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x2000000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x2000000, "Unlocked: Book page 1", 50, "models/items/generic_note.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x4000000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x4000000, "Unlocked: Book page 2", 50, "models/items/generic_note.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x8000000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x8000000, "Unlocked: Book page 3", 50, "models/items/generic_note.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x10000000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x10000000, "Unlocked: Book page 4", 50, "models/items/generic_note.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x20000000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x20000000, "Unlocked: Book page 5", 50, "models/items/generic_note.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x40000000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x40000000, "Unlocked: Book page 6", 50, "models/items/generic_note.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_iUnlocks & 0x80000000 )
		{
			if ( !g_bUnlockShown )
			{
				ShowUnlock( 0x80000000, "Unlocked: Doctor mode", 50, "models/doctor_unlock.mdl", 0 );
				g_bUnlockShown = true;
			}
		}
		else if ( m_bContinue )
		{
			m_bContinue = false;
			setVisible( false );
			gViewPort->UpdateCursorState();
			g_iUnlockClothes = 0;

			if ( iMenuMode )
			{
				gEngfuncs.pfnClientCmd( "map c_game_menu1" );
				gViewPort->UpdateCursorState();
			}
			else
			{
				gEngfuncs.pfnServerCmd( "closegame" );
				gViewPort->UpdateCursorState();
			}
		}
	}
	else
	{
		if ( m_pUnlockLabel->isVisible() )
			m_pUnlockLabel->setVisible( false );

		if ( !m_bShowStats || m_bTextOnly || g_iCoopCredits )
		{
			if ( m_bFadeIn )
			{
				if ( m_iBrightness < 255 )
					m_iBrightness += 15;
			}
			else if ( m_iBrightness > 0 )
			{
				m_iBrightness -= 15;
			}

			if ( m_iBrightness > 255 )
				m_iBrightness = 255;
			else if ( m_iBrightness < 0 )
				m_iBrightness = 0;

			if ( m_bTextOnly )
				m_iBrightness = 255;

			for ( i = 1; i <= m_iNumLines; i++ )
			{
				int x, y;
				int r, g, b, a;

				m_pCreditLabel[i]->getPos( x, y );

				int iStep = ScreenHeight / m_iScrollSpeed / 20;
				if ( cl_fastcredits->value )
					iStep = 20;
				else if ( iStep <= 0 )
					iStep = 1;

				if ( m_bScrollUp )
					y += iStep;
				else
					y -= iStep;

				m_pCreditLabel[i]->getFgColor( r, g, b, a );

				bool bScrollUp = m_bScrollUp;

				if ( bScrollUp )
				{
					if ( y > 0 && y < ScreenHeight - ScreenHeight * 0.25 )
						a -= 5;
					if ( y > ScreenHeight - ScreenHeight * 0.25 )
						a += 5;
				}
				else
				{
					if ( y > ScreenHeight - ScreenHeight * 0.25 )
						a -= 5;
					if ( y < ScreenHeight * 0.25 )
						a += 5;
				}

				if ( i == m_iNumLines / 2 && y > ScreenHeight / 2 - 16 && !m_bFadeIn && !m_bFadeDone && !m_bTextOnly )
					m_bFadeIn = true;

				if ( a < 0 )
					a = 0;
				else if ( a > 255 )
					a = 255;

				if ( y > ScreenHeight )
					a = 255;

				if ( bScrollUp )
				{
					if ( i == m_iNumLines && y > ScreenHeight / 2 - 16 && !m_bTextOnly )
					{
						m_bFadeIn = false;
						m_bFadeDone = true;
						y = ScreenHeight / 2 - 16;

						if ( !m_bStatsTimer )
						{
							m_bStatsTimer = true;
							m_flStatsTime = gEngfuncs.GetClientTime() + 3.0;
							IrrKlangStop( 2.0 );
						}
					}
				}
				else if ( i == m_iNumLines && y < ScreenHeight / 2 - 16 && !m_bTextOnly )
				{
					m_bFadeIn = false;
					m_bFadeDone = true;
					y = ScreenHeight / 2 - 16;

					if ( !m_bStatsTimer )
					{
						m_bStatsTimer = true;
						m_flStatsTime = gEngfuncs.GetClientTime() + 3.0;
					}
				}

				m_pCreditLabel[i]->setFgColor( m_iBrightness, m_iBrightness, m_iBrightness, a );
				m_pCreditLabel[i]->setPos( x, y );
			}
		}
		else
		{
			int x, y;
			int r, g, b, a;

			if ( m_bContinue )
			{
				m_bContinue = false;
				m_bUnlocks = true;
			}

			setBgColor( 255, 255, 255, 255 );

			for ( i = 1; i <= m_iNumLines; i++ )
			{
				m_pCreditLabel[i]->getFgColor( r, g, b, a );
				a += 15;
				if ( a > 255 )
					a = 255;
				m_pCreditLabel[i]->setFgColor( m_iBrightness, m_iBrightness, m_iBrightness, a );
			}

			m_bFadeIn = true;

			if ( m_iBrightness < 255 )
				m_iBrightness += 15;

			if ( m_iBrightness > 255 )
				m_iBrightness = 255;
			else if ( m_iBrightness < 0 )
				m_iBrightness = 0;

			if ( !m_pGradeLabel->isVisible() )
				m_pGradeLabel->setVisible( true );
			m_pGradeLabel->getFgColor( r, g, b, a );
			if ( a > 0 )
				a -= 5;
			if ( a < 0 )
				a = 0;
			m_pGradeLabel->setFgColor( m_iBrightness, m_iBrightness, m_iBrightness, a );

			if ( !m_pGradeShadowLabel->isVisible() )
				m_pGradeShadowLabel->setVisible( true );
			m_pGradeLabel->getFgColor( r, g, b, a );
			if ( a > 0 )
				a -= 1;
			if ( a < 0 )
				a = 0;
			m_pGradeShadowLabel->setFgColor( 255, 0, 0, a );

			if ( !m_pStatsTitleLabel->isVisible() )
				m_pStatsTitleLabel->setVisible( true );
			m_pStatsTitleLabel->getFgColor( r, g, b, a );
			if ( a > 0 )
				a -= 5;
			if ( a < 0 )
				a = 0;
			m_pStatsTitleLabel->setFgColor( m_iBrightness, m_iBrightness, m_iBrightness, a );

			for ( i = 1; i <= 14; i++ )
			{
				if ( !m_pStatLabel[i]->isVisible() )
					m_pStatLabel[i]->setVisible( true );
				m_pStatLabel[i]->getFgColor( r, g, b, a );
				if ( a > 0 )
					a -= 5;
				if ( a < 0 )
					a = 0;
				m_pStatLabel[i]->setVisible( true );
				m_pStatLabel[i]->getPos( x, y );
				m_pStatLabel[i]->setFgColor( m_iBrightness, m_iBrightness, m_iBrightness, a );
				m_pStatLabel[i]->setPos( x, y );
			}
		}

		if ( m_bShowStats && !m_bTextOnly && !g_iCoopCredits )
		{
			int r, g, b, a;

			getBgColor( r, g, b, a );
			if ( a < 255 )
				a += 5;
			if ( a > 255 )
				a = 255;
			setBgColor( 255 - m_iBrightness, 255 - m_iBrightness, 255 - m_iBrightness, a );
		}
		else if ( !m_bTextOnly )
		{
			setBgColor( 255 - m_iBrightness, 255 - m_iBrightness, 255 - m_iBrightness, 0 );
		}

		if ( !m_bTextOnly )
		{
			Panel::solve();
			return;
		}
	}

	setBgColor( 255, 255, 255, 255 );
	Panel::solve();
}

CCredits::~CCredits( void )
{
}
