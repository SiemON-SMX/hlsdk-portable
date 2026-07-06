#ifndef AIM_ENTITY_H
#define AIM_ENTITY_H

#include <string>

void AimEntity_Init();

int AimEntity_OnAimNew( const char *pszName, int iSize, void *pbuf );
int AimEntity_OnAimUpd( const char *pszName, int iSize, void *pbuf );
int AimEntity_OnAimClear( const char *pszName, int iSize, void *pbuf );

extern bool        aimingAtEntity;
extern std::string entityClassName;
extern std::string entityTarget;
extern std::string entityTargetName;
extern int         entityModelIndex;
extern Vector      entityPos;
extern Vector      entityAngle;
extern Vector      entityVelocity;
extern Vector      entityMins;
extern Vector      entityMaxs;
extern float       entityHealth;
extern float       entityHealthMax;
extern int         entityFlags;
extern int         entityDeadFlags;
extern int         entitySpawnFlags;

#endif // AIM_ENTITY_H
