/***
 *
 *      Kerotan — hidden Metal Gear frog figurine Easter egg.
 *
 *      One is hidden somewhere on every Half-Payne map (c1a0 onward).
 *      Shoot it with a bullet, blast, crowbar, or blade and it spins,
 *      wobbles, and ribbits in a degrading pitch until it finally breaks
 *      after 5 hits.  Ported from the GoldSRC Half-Payne source.
 *
 ***/

#ifndef KEROTAN_H
#define KEROTAN_H

class CKerotan : public CBaseToggle
{
public:
        void    Spawn( void );
        void    Precache( void );

        virtual int Save( CSave &save );
        virtual int Restore( CRestore &restore );
        virtual int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker,
                                float flDamage, int bitsDamageType );

        BOOL    hasBeenFound;
        int     hitsReceived;
        int     soundsLeft;
        float   nextSound;
        float   rotationLeft;
        int     rotationDirection;
        float   rollAmplitude;
        int     rollDirection;

        void EXPORT OnThink( void );

        static TYPEDESCRIPTION m_SaveData[];
};

#endif // KEROTAN_H
