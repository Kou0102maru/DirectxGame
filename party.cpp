/*==============================================================================

   Party Management [party.cpp]
                                                         Author :
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "party.h"

static PartyMonster g_Party[PARTY_MAX]{};
static int g_PartyCount = 0;

void Party_Initialize()
{
    g_PartyCount = 0;
    for (int i = 0; i < PARTY_MAX; i++) {
        g_Party[i] = {};
    }
}

void Party_Finalize()
{
    g_PartyCount = 0;
}

int Party_GetCount()
{
    return g_PartyCount;
}

int Party_GetMax()
{
    return PARTY_MAX;
}

bool Party_Add(MonsterKind kind, int level)
{
    if (g_PartyCount >= PARTY_MAX) return false;

    const MonsterBaseParam* base = Monster_GetBaseParam(kind);
    if (!base) return false;

    float growth = 1.0f + (level - 1) * 0.1f;

    PartyMonster pm{};
    pm.kind   = kind;
    pm.level  = level;
    pm.hp_max = (int)(base->base_hp  * growth);
    pm.atk    = (int)(base->base_atk * growth);
    pm.def    = (int)(base->base_def * growth);

    g_Party[g_PartyCount] = pm;
    g_PartyCount++;
    return true;
}

const PartyMonster* Party_Get(int index)
{
    if (index < 0 || index >= g_PartyCount) return nullptr;
    return &g_Party[index];
}

const char* Party_GetKindName(MonsterKind kind)
{
    switch (kind) {
    case MONSTER_KIND_SPIDER:  return "Spider";
    case MONSTER_KIND_WOLF:   return "Wolf";
    case MONSTER_KIND_DRAGON: return "Dragon";
    default:                  return "???";
    }
}
