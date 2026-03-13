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

// パーティモンスターのレベルアップ処理
static void PartyMonster_LevelUp(PartyMonster& pm)
{
    pm.level++;
    const MonsterBaseParam* base = Monster_GetBaseParam(pm.kind);
    if (base) {
        // レベルごとにベースステータスの15%ずつ成長
        pm.hp_max += (int)(base->base_hp  * 0.15f);
        pm.atk    += (int)(base->base_atk * 0.15f);
        pm.def    += (int)(base->base_def * 0.15f);
        // 最低でも+1保証
        if (base->base_hp  * 0.15f < 1.0f) pm.hp_max += 1;
        if (base->base_atk * 0.15f < 1.0f) pm.atk    += 1;
        if (base->base_def * 0.15f < 1.0f) pm.def    += 1;
        // レベルアップ時はHP全回復
        pm.hp = pm.hp_max;
    }
    pm.exp_next = pm.level * pm.level * 10;
}

// パーティモンスターに経験値を付与（レベルアップ判定付き）
static void PartyMonster_GainExp(PartyMonster& pm, int exp)
{
    pm.exp += exp;
    while (pm.exp >= pm.exp_next) {
        pm.exp = 0;  // レベルアップ時に累積経験値を0にリセット
        PartyMonster_LevelUp(pm);
    }
}

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

    float growth = 1.0f + (level - 1) * 0.15f;

    PartyMonster pm{};
    pm.kind   = kind;
    pm.level  = level;
    pm.hp_max = (int)(base->base_hp  * growth);
    pm.hp     = pm.hp_max;  // 加入時はHP満タン
    pm.atk    = (int)(base->base_atk * growth);
    pm.def    = (int)(base->base_def * growth);
    pm.exp    = 0;
    pm.exp_next = level * level * 10;

    g_Party[g_PartyCount] = pm;
    g_PartyCount++;
    return true;
}

bool Party_Remove(int index)
{
    if (index < 0 || index >= g_PartyCount) return false;

    // アクティブ戦闘員が削除対象の場合はプレイヤーに戻す
    if (g_ActiveFighter == index) {
        g_ActiveFighter = -1;
    }
    else if (g_ActiveFighter > index) {
        // 削除位置より後ろのアクティブ戦闘員はインデックスがずれる
        g_ActiveFighter--;
    }

    // 配列を詰める
    for (int i = index; i < g_PartyCount - 1; i++) {
        g_Party[i] = g_Party[i + 1];
    }
    g_Party[g_PartyCount - 1] = {};
    g_PartyCount--;
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
    case MONSTER_KIND_SPIDER:  return "スパイダー";
    case MONSTER_KIND_WOLF:   return "オオカミ";
    case MONSTER_KIND_DRAGON: return "ドラゴン";
    case MONSTER_KIND_ROBOT:  return "ロボット";
    case MONSTER_KIND_EYEBALL: return "目玉";
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
    int totalSlots = g_PartyCount + 1;  // プレイヤー(-1) + パーティ(0..N-1)
    for (int attempts = 0; attempts < totalSlots; attempts++) {
        g_ActiveFighter++;
        if (g_ActiveFighter >= g_PartyCount) {
            g_ActiveFighter = -1;  // パーティ末尾を超えたらプレイヤーに戻る
        }
        // 生存チェック（HP0のメンバーはスキップ）
        if (g_ActiveFighter == -1) {
            if (Player_GetHp() > 0) return;  // プレイヤー生存
        } else {
            if (g_Party[g_ActiveFighter].hp > 0) return;  // モンスター生存
        }
    }
    // 全員倒れている場合はプレイヤーに戻す
    g_ActiveFighter = -1;
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
    // アクティブモンスターに経験値付与（レベルアップあり）
    PartyMonster_GainExp(g_Party[g_ActiveFighter], exp);
    // プレイヤーにも半分の経験値を付与
    Player_GainExp(exp / 2);
}

void Party_AllGainExp(int exp)
{
    // プレイヤーに経験値付与
    Player_GainExp(exp);
    // 全パーティモンスターに経験値付与（レベルアップあり）
    for (int i = 0; i < g_PartyCount; i++) {
        PartyMonster_GainExp(g_Party[i], exp);
    }
}

const char* Party_GetFighterName()
{
    if (g_ActiveFighter == -1) return "プレイヤー";
    return Party_GetKindName(g_Party[g_ActiveFighter].kind);
}

MonsterKind Party_GetFighterKind()
{
    if (g_ActiveFighter == -1) return MONSTER_KIND_MAX;
    return g_Party[g_ActiveFighter].kind;
}

//=============================================================================
// 生存チェック・交代
//=============================================================================
bool Party_HasAnyAliveFighter()
{
    if (Player_GetHp() > 0) return true;
    for (int i = 0; i < g_PartyCount; i++) {
        if (g_Party[i].hp > 0) return true;
    }
    return false;
}

bool Party_SwitchToNextAlive()
{
    if (!Party_HasAnyAliveFighter()) return false;
    Party_CycleActiveFighter();
    return true;
}
