#include "wrect.h"
#include "cl_dll.h"
#include "parsemsg.h"

#include "aim_entity.h"

bool aimingAtEntity = false;
std::string entityClassName = "";
std::string entityTarget = "";
std::string entityTargetName = "";
int entityModelIndex = 0;
Vector entityPos = Vector( 0, 0, 0 );
Vector entityAngle = Vector( 0, 0, 0 );
Vector entityVelocity = Vector( 0, 0, 0 );
Vector entityMins = Vector( 0, 0, 0 );
Vector entityMaxs = Vector( 0, 0, 0 );

float entityHealth = 0.0f;
float entityHealthMax = 0.0f;
int entityFlags = 0;
int entityDeadFlags = 0;
int entitySpawnFlags = 0;

void AimEntity_Init() {
	gEngfuncs.pfnHookUserMsg( "OnAimNew",   AimEntity_OnAimNew );
	gEngfuncs.pfnHookUserMsg( "OnAimUpd",   AimEntity_OnAimUpd );
	gEngfuncs.pfnHookUserMsg( "OnAimClear", AimEntity_OnAimClear );
}

int AimEntity_OnAimNew( const char *pszName, int iSize, void *pbuf ) {
	BEGIN_READ( pbuf, iSize );

	entityClassName = READ_STRING();
	entityTarget    = READ_STRING();
	entityTargetName = READ_STRING();
	entityModelIndex = READ_SHORT();

	entityMins.x = READ_COORD();
	entityMins.y = READ_COORD();
	entityMins.z = READ_COORD();

	entityMaxs.x = READ_COORD();
	entityMaxs.y = READ_COORD();
	entityMaxs.z = READ_COORD();

	entitySpawnFlags = READ_BYTE();

	aimingAtEntity = true;
	return 1;
}

int AimEntity_OnAimUpd( const char *pszName, int iSize, void *pbuf ) {
	BEGIN_READ( pbuf, iSize );

	entityPos.x = READ_COORD();
	entityPos.y = READ_COORD();
	entityPos.z = READ_COORD();

	entityAngle.x = READ_COORD();
	entityAngle.y = READ_COORD();
	entityAngle.z = READ_COORD();

	entityVelocity.x = READ_COORD();
	entityVelocity.y = READ_COORD();
	entityVelocity.z = READ_COORD();

	entityHealth    = READ_FLOAT();
	entityHealthMax = READ_FLOAT();
	entityFlags     = READ_BYTE();
	entityDeadFlags = READ_BYTE();

	return 1;
}

int AimEntity_OnAimClear( const char *pszName, int iSize, void *pbuf ) {
	aimingAtEntity = false;
	return 1;
}
