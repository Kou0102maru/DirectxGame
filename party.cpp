/*==============================================================================

   Party Management [party.cpp]
                                                         Author :
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "party.h"
#include "player.h"

static PartyMonster g_Party[PARTY_MAX]{};
static int g_PartyCount = 0;
static int g_ActiveFighter = -1;  // -1 = プレイヤー本人

void Party_Initialize()
{
    g_PartyCount = 0;
    g_ActiveFighter = -1;
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
    pm.hp     = pm.hp_max;  // 加入時はHP満タン
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
    case MONSTER_KIND_ROBOT:  return "Robot";
    case MONSTER_KIND_EYEBALL: return "Eyeball";
    default:                  return "???";
    }
}

//=============================================================================
// アクティブ戦闘員管理
//=============================================================================
void Party_SetActiveFighter(int index)
{
    if (index < -1 || index >= g_PartyCount) {
        g_ActiveFighter = -1;
    } else {
        g_ActiveFighter = index;
    }
}

int Party_GetActiveFighter()
{
    return g_ActiveFighter;
}

void Party_CycleActiveFighter()
{
    if (g_PartyCount == 0) {
        g_ActiveFighter = -1;
        return;
    }
    g_ActiveFighter++;
    if (g_ActiveFighter >= g_PartyCount) {
        g_ActiveFighter = -1;  // パーティ末尾を超えたらプレイヤーに戻る
    }
    // HP0のモンスターはスキップ
    while (g_ActiveFighter >= 0 && g_Party[g_ActiveFighter].hp <= 0) {
        g_ActiveFighter++;
        if (g_ActiveFighter >= g_PartyCount) {
            g_ActiveFighter = -1;
            break;
        }
    }
}

bool Party_IsPlayerActive()
{
    return g_ActiveFighter == -1;
}

//=============================================================================
// 戦闘用ステータスAPI（アクティブ戦闘員に透過的にアクセス）
//=============================================================================
int Party_GetFighterHp()
{
    if (g_ActiveFighter == -1) return Player_GetHp();
    return g_Party[g_ActiveFighter].hp;
}

int Party_GetFighterHpMax()
{
    if (g_ActiveFighter == -1) return Player_GetHpMax();
    return g_Party[g_ActiveFighter].hp_max;
}

int Party_GetFighterAtk()
{
    if (g_ActiveFighter == -1) return Player_GetAtk();
    return g_Party[g_ActiveFighter].atk;
}

int Party_GetFighterDef()
{
    if (g_ActiveFighter == -1) return Player_GetDef();
    return g_Party[g_ActiveFighter].def;
}

int Party_GetFighterLevel()
{
    if (g_ActiveFighter == -1) return Player_GetLevel();
    return g_Party[g_ActiveFighter].level;
}

void Party_FighterTakeDamage(int dmg)
{
    if (g_ActiveFighter == -1) {
        Player_TakeDamage(dmg);
        return;
    }
    g_Party[g_ActiveFighter].hp -= dmg;
    if (g_Party[g_ActiveFighter].hp < 0) g_Party[g_ActiveFighter].hp = 0;
}

void Party_FighterGainExp(int exp)
{
    if (g_ActiveFighter == -1) {
        Player_GainExp(exp);
        return;
    }
    // パーティモンスターはレベルアップなし（将来拡張用）
    // プレイヤーにも半分の経験値を付与
    Player_GainExp(exp / 2);
}

const char* Party_GetFighterName()
{
    if (g_ActiveFighter == -1) return "Player";
    return Party_GetKindName(g_Party[g_ActiveFighter].kind);
}

MonsterKind Party_GetFighterKind()
{
    if (g_ActiveFighter == -1) return MONSTER_KIND_MAX;
    return g_Party[g_ActiveFighter].kind;
}
