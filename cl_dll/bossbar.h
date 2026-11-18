// ======================================
// ======================================

#ifndef _BOSSBAR_H
#define _BOSSBAR_H
using namespace vgui;

class CBossBar : public Panel
{
public:
	CBossBar();
	~CBossBar();
	void Initialize();

	virtual void solve();

public:
	int		m_iState;
	char	m_szEmptyImage[128];
	char	m_szFullImage[128];
	int		m_iHealth;
	int		m_iMaxHealth;
	int		m_iUnused1C8;
	Image	*m_pFullImage;
	Image	*m_pEmptyImage;
	ImagePanel	*m_pFull;
	ImagePanel	*m_pEmpty;
	ImagePanel	*m_pBar;
};

void VGUI_BossBarInit();

#endif
