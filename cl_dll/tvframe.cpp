// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "tvframe.h"

CTVFrame::CTVFrame() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

void CTVFrame::Initialize()
{
	m_iTVFrame = 0;
	setVisible( false );
}

int MsgTVFrame( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pScope )
		READ_BYTE();

	return 1;
}

void TVFrameInit()
{
	gEngfuncs.pfnHookUserMsg( "TVFrame", MsgTVFrame );
}

CTVFrame::~CTVFrame( void )
{
}
