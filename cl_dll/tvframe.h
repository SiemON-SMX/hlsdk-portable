// ======================================
// ======================================

#ifndef _TVFRAME_H
#define _TVFRAME_H
using namespace vgui;

class CTVFrame : public Panel
{
public:
	CTVFrame();
	~CTVFrame( void );
	void Initialize();

public:
	int		m_iTVFrame;
	int		m_iUnusedC0[8];
};

void TVFrameInit();

#endif
