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
    int hp;       // 現在HP
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

// アクティブ戦闘員管理（-1 = プレイヤー本人、0?7 = パーティモンスター）
void Party_SetActiveFighter(int index);
int  Party_GetActiveFighter();
void Party_CycleActiveFighter();   // Tab用: -1 → 0 → 1 → ... → -1
bool Party_IsPlayerActive();

// 戦闘用（アクティブ戦闘員のステータスを透過的に返す）
int  Party_GetFighterHp();
int  Party_GetFighterHpMax();
int  Party_GetFighterAtk();
int  Party_GetFighterDef();
int  Party_GetFighterLevel();
void Party_FighterTakeDamage(int dmg);
void Party_FighterGainExp(int exp);
const char* Party_GetFighterName();
MonsterKind Party_GetFighterKind();  // プレイヤー時は MONSTER_KIND_MAX

#endif // PARTY_H
