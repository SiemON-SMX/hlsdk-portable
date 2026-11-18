// ======================================
// ======================================

#ifndef _COOPSTATS_H
#define _COOPSTATS_H
using namespace vgui;

#define MAX_COOP_PLAYERS	4

class CCoopStats : public Panel
{
public:
	CCoopStats();
	~CCoopStats( void );
	void Initialize();

public:
	int		m_iUnknownBC;
	Label	*m_pTitleLabel;
	Label	*m_pGradeLabel;
	Label	*m_pStatLabel[MAX_COOP_PLAYERS + 1][10];
};

void SetEndingOrUnlockables( int type, bool hasntgot, bool ending6, bool ending7 );

#endif
