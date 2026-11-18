// ======================================
// ======================================

#ifndef _SCOPE_H
#define _SCOPE_H
using namespace vgui;

class CScope : public Panel
{
public:
	CScope();
	~CScope( void );
	void Initialize();

public:
	int		m_iScope;
	Image	*m_pScopeImage;
	ImagePanel	*m_pScopePanel;
};

void ScopeInit();

#endif
