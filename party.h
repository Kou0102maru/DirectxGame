/*==============================================================================

   Party Management [party.h]
                                                         Author :
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef PARTY_H
#define PARTY_H

#include "monster.h"

#define PARTY_MAX 8

struct PartyMonster
{
    MonsterKind kind;
    int level;
    int hp_max;
    int atk;
    int def;
};

void Party_Initialize();
void Party_Finalize();
int  Party_GetCount();
int  Party_GetMax();
bool Party_Add(MonsterKind kind, int level);
const PartyMonster* Party_Get(int index);

const char* Party_GetKindName(MonsterKind kind);

#endif // PARTY_H
