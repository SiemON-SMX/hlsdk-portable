// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "bossbar.h"
#include "../game_shared/vgui_loadtga.h"

CBossBar::CBossBar() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x;

	m_pFullImage = vgui_LoadTGA( "gfx/vgui/boss/doctor_health_full.tga" );
	if ( m_pFullImage )
		m_pFullImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/boss/doctor_health_full.tga!\n" );

	m_pEmptyImage = vgui_LoadTGA( "gfx/vgui/boss/doctor_health_empty.tga" );
	if ( m_pEmptyImage )
		m_pEmptyImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/boss/doctor_health_empty.tga!\n" );

	m_pFull = new ImagePanel( m_pFullImage );
	m_pFull->setParent( this );
	wide = m_pFull->getWide();
	tall = m_pFull->getTall();
	x = ScreenWidth / 2 - wide / 2;
	m_pFull->setPos( x, tall );
	m_pFull->setVisible( true );

	m_pEmpty = new ImagePanel( m_pEmptyImage );
	m_pEmpty->setParent( this );
	m_pEmpty->setPos( x, tall );
	m_pEmpty->setVisible( true );

	m_pBar = new ImagePanel( m_pFullImage );
	m_pBar->setParent( this );
	m_pBar->setPos( x, tall );
	m_pBar->setVisible( true );

	setBgColor( 0, 0, 0, 255 );
	m_iUnused1C8 = 0;

	Initialize();
}

CBossBar::~CBossBar()
{
	delete m_pFullImage;
	delete m_pEmptyImage;
}

void CBossBar::Initialize()
{
	m_iUnused1C8 = 0;
	setVisible( false );
}

int MsgBossBar( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pBossBar )
	{
		gViewPort->m_pBossBar->m_iUnused1C8 = 0;
		gViewPort->m_pBossBar->m_iState = READ_BYTE();
		sprintf( gViewPort->m_pBossBar->m_szEmptyImage, "%s", READ_STRING() );
		sprintf( gViewPort->m_pBossBar->m_szFullImage, "%s", READ_STRING() );

		gViewPort->m_pBossBar->m_pFull->setImage( vgui_LoadTGA( gViewPort->m_pBossBar->m_szFullImage ) );
		gViewPort->m_pBossBar->m_pBar->setImage( vgui_LoadTGA( gViewPort->m_pBossBar->m_szFullImage ) );
		gViewPort->m_pBossBar->m_pEmpty->setImage( vgui_LoadTGA( gViewPort->m_pBossBar->m_szEmptyImage ) );

		int iHealth = READ_SHORT();
		if ( iHealth < 0 )
			iHealth = 0;

		if ( gViewPort->m_pBossBar->m_iState == 1 )
			gViewPort->m_pBossBar->m_iHealth = iHealth;
		else
			gViewPort->m_pBossBar->m_iHealth = 0;

		gViewPort->m_pBossBar->m_iMaxHealth = READ_SHORT();

		if ( gViewPort->m_pBossBar->m_iState == 1 )
			gViewPort->m_pBossBar->setVisible( true );
		else if ( gViewPort->m_pBossBar->m_iState == 0 )
			gViewPort->m_pBossBar->setVisible( false );
	}

	return 1;
}

void VGUI_BossBarInit()
{
	gEngfuncs.pfnHookUserMsg( "BossBar", MsgBossBar );
}

void CBossBar::solve()
{
	int tall = m_pEmpty->getTall();
	int wide = m_pEmpty->getWide();

	if ( m_iState == 2 )
		setVisible( false );

	int width = (int)( (float)m_iHealth / (float)m_iMaxHealth * (float)wide );
	if ( width < 1 )
		width = 0;

	m_pBar->setSize( width, tall );

	Panel::solve();
}
