// ======================================
// ======================================

#ifndef _CUTSCENESKIP_H
#define _CUTSCENESKIP_H
using namespace vgui;

class CCutsceneSkip : public Panel
{
public:
	CCutsceneSkip();
	~CCutsceneSkip( void );
	void Initialize();

public:
	Image	*m_pDotsImage;
	ImagePanel	*m_pDotsPanel;
};

#endif
