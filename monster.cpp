/*==============================================================================

   モンスター管理 [monster.cpp]
                                                         Author : 
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "monster.h"
#include "monster_slime.h"
#include "monster_wolf.h"
#include "monster_dragon.h"
#include "model.h"
#include "texture.h"
#include "cube.h"
#include "light.h"
#include "player_camera.h"
#include <DirectXMath.h>
using namespace DirectX;
#include <algorithm>
#include <cassert>

// モンスター描画用テクスチャ（外部から参照可能に）
int g_MonsterTexSlime = -1;
int g_MonsterTexWolf = -1;
int g_MonsterTexDragon = -1;

//=============================================================================
// 種族別パラメータテーブル
//=============================================================================
static const MonsterBaseParam g_MonsterBaseParams[MONSTER_KIND_MAX] =
{
    // kind               name        hp   mp  atk  def  spd  exp   skills
    { MONSTER_KIND_SLIME,  "スライム",  20,  10,   5,   3,   4,  10, { SKILL_ATTACK, SKILL_NONE,   SKILL_NONE, SKILL_NONE } },
    { MONSTER_KIND_WOLF,   "オオカミ",  50,  10,  15,   8,  12,  35, { SKILL_ATTACK, SKILL_NONE,   SKILL_NONE, SKILL_NONE } },
    { MONSTER_KIND_DRAGON, "ドラゴン", 120,  50,  30,  20,  10, 100, { SKILL_ATTACK, SKILL_FIRE,   SKILL_NONE, SKILL_NONE } },
};

//=============================================================================
// スキルデータテーブル
//=============================================================================
static const Skill g_SkillData[SKILL_MAX] =
{
    { SKILL_ATTACK, "こうげき",   10,  0 },
    { SKILL_FIRE,   "ファイア",   25, 10 },
    { SKILL_HEAL,   "ヒール",      0,  8 },
};

const MonsterBaseParam* Monster_GetBaseParam(MonsterKind kind)
{
    assert(kind >= 0 && kind < MONSTER_KIND_MAX);
    return &g_MonsterBaseParams[kind];
}

const Skill* Monster_GetSkillData(SkillId skill_id)
{
    assert(skill_id >= 0 && skill_id < SKILL_MAX);
    return &g_SkillData[skill_id];
}

//=============================================================================
// パラメータ初期化（レベルに応じてステータスを計算）
//=============================================================================
void Monster::InitParams(MonsterKind kind, int level)
{
    const MonsterBaseParam* base = Monster_GetBaseParam(kind);

    m_kind  = kind;
    m_name  = base->name;
    m_level = level;

    // レベルに応じてパラメータを成長させる（シンプルな線形成長）
    float growth = 1.0f + (level - 1) * 0.1f;
    m_hp_max = (int)(base->base_hp  * growth);
    m_mp_max = (int)(base->base_mp  * growth);
    m_atk    = (int)(base->base_atk * growth);
    m_def    = (int)(base->base_def * growth);
    m_spd    = (int)(base->base_spd * growth);
    m_exp    = 0;

    // 次レベルまでの必要経験値（レベルが上がるほど多く必要）
    m_exp_next = level * level * 10;

    // HP・MPは最大値で開始
    m_hp = m_hp_max;
    m_mp = m_mp_max;

    // スキルをコピー
    for (int i = 0; i < 4; i++) {
        m_skills[i] = base->skills[i];
    }

    // 倒した時の経験値
    m_exp = 0;
}

//=============================================================================
// コンストラクタ
//=============================================================================
Monster::Monster(MonsterKind kind, const XMFLOAT3& position, int level)
    : m_position(position)
{
    InitParams(kind, level);
}

//=============================================================================
// ステートマシン
//=============================================================================
void Monster::ChangeState(State* pNext)
{
    m_pNextState = pNext;
}

void Monster::UpdateState()
{
    if (m_pNextState != m_pState) {
        delete m_pState;
        m_pState = m_pNextState;
    }
}

//=============================================================================
// 更新・描画
//=============================================================================
void Monster::Update(double elapsed_time)
{
    if (m_pState) {
        m_pState->Update(elapsed_time);
    }
}

void Monster::Draw() const
{
    if (m_pState) {
        m_pState->Draw();
    }
    else {
        // デフォルト描画（ステートがない場合）
        int texture_id = g_MonsterTexSlime;
        
        switch (m_kind) {
        case MONSTER_KIND_SLIME:
            texture_id = g_MonsterTexSlime;
            Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.2f, 0.8f, 0.2f, 1.0f });
            break;
        case MONSTER_KIND_WOLF:
            texture_id = g_MonsterTexWolf;
            Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.6f, 0.5f, 0.3f, 1.0f });
            break;
        case MONSTER_KIND_DRAGON:
            texture_id = g_MonsterTexDragon;
            Light_SetSpecularWorld(PlayerCamera_GetPosition(), 4.0f, { 0.8f, 0.2f, 0.2f, 1.0f });
            break;
        default:
            texture_id = g_MonsterTexSlime;
            break;
        }
        
        // キューブで描画
        XMMATRIX world = XMMatrixTranslation(m_position.x, m_position.y + 0.5f, m_position.z);
        Cube_Draw(texture_id, world);
    }
}

//=============================================================================
// 戦闘処理
//=============================================================================
void Monster::TakeDamage(int damage)
{
    // 防御力でダメージ軽減（最低1ダメージ）
    int actual_damage = std::max(1, damage - m_def / 2);
    m_hp -= actual_damage;

    if (m_hp <= 0) {
        m_hp = 0;
        m_is_dead = true;
    }
}

void Monster::Heal(int amount)
{
    m_hp = std::min(m_hp + amount, m_hp_max);
}

bool Monster::UseSkill(SkillId skill_id, Monster* target)
{
    if (skill_id == SKILL_NONE) return false;
    if (skill_id < 0 || skill_id >= SKILL_MAX) return false;

    const Skill* skill = Monster_GetSkillData(skill_id);

    // MPが足りなければ失敗
    if (m_mp < skill->mp_cost) return false;

    m_mp -= skill->mp_cost;

    switch (skill_id)
    {
    case SKILL_ATTACK:
        if (target) target->TakeDamage(m_atk + skill->power);
        break;

    case SKILL_FIRE:
        if (target) target->TakeDamage(m_atk + skill->power);
        break;

    case SKILL_HEAL:
        Heal(skill->power + m_atk / 2);
        break;

    default:
        break;
    }

    return true;
}

//=============================================================================
// 経験値・レベルアップ
//=============================================================================
void Monster::GainExp(int exp)
{
    m_exp += exp;

    while (m_exp >= m_exp_next) {
        m_exp -= m_exp_next;
        LevelUp();
    }
}

void Monster::LevelUp()
{
    m_level++;

    // ステータスを再計算
    const MonsterBaseParam* base = Monster_GetBaseParam(m_kind);
    float growth = 1.0f + (m_level - 1) * 0.1f;

    int new_hp_max = (int)(base->base_hp  * growth);
    int new_mp_max = (int)(base->base_mp  * growth);

    // HP・MPの最大値が増えた分だけ回復
    m_hp += new_hp_max - m_hp_max;
    m_mp += new_mp_max - m_mp_max;

    m_hp_max = new_hp_max;
    m_mp_max = new_mp_max;
    m_atk    = (int)(base->base_atk * growth);
    m_def    = (int)(base->base_def * growth);
    m_spd    = (int)(base->base_spd * growth);

    // 次レベルまでの必要経験値を更新
    m_exp_next = m_level * m_level * 10;

    // 上限チェック
    m_hp = std::min(m_hp, m_hp_max);
    m_mp = std::min(m_mp, m_mp_max);
}

//=============================================================================
// 衝突判定
//=============================================================================
Sphere Monster::GetCollision() const
{
    return { m_position, 1.0f };
}

//=============================================================================
// フィールド上のモンスター群管理
//=============================================================================
static constexpr int MAX_FIELD_MONSTERS = 32;
static Monster* g_pMonsters[MAX_FIELD_MONSTERS]{};
static int g_MonsterCount{ 0 };

void Monster_Initialize()
{
    g_MonsterCount = 0;
    
    // テクスチャロード（既存のテクスチャを流用）
    g_MonsterTexSlime = Texture_Load(L"resource/texture/blue.png");  // 青系
    g_MonsterTexWolf = Texture_Load(L"resource/texture/green.png");    // 緑系
    g_MonsterTexDragon = Texture_Load(L"resource/texture/red.png");   // 赤系
}

void Monster_Finalize()
{
    for (int i = 0; i < g_MonsterCount; i++) {
        delete g_pMonsters[i];
        g_pMonsters[i] = nullptr;
    }
    g_MonsterCount = 0;
}

void Monster_Update(double elapsed_time)
{
    // ステートを先に更新
    for (int i = 0; i < g_MonsterCount; i++) {
        g_pMonsters[i]->UpdateState();
    }

    // 行動更新
    for (int i = 0; i < g_MonsterCount; i++) {
        g_pMonsters[i]->Update(elapsed_time);
    }

    // 死亡したモンスターを削除
    for (int i = g_MonsterCount - 1; i >= 0; i--) {
        if (g_pMonsters[i]->IsDead()) {
            delete g_pMonsters[i];
            g_pMonsters[i] = g_pMonsters[--g_MonsterCount];
        }
    }
}

void Monster_Draw()
{
    for (int i = 0; i < g_MonsterCount; i++) {
        g_pMonsters[i]->Draw();
    }
}

void Monster_Create(MonsterKind kind, const XMFLOAT3& position, int level)
{
    if (g_MonsterCount >= MAX_FIELD_MONSTERS) return;

    g_pMonsters[g_MonsterCount++] = new Monster(kind, position, level);
}

void Monster_CreateSlime(const XMFLOAT3& position, int level)
{
    if (g_MonsterCount >= MAX_FIELD_MONSTERS) return;

    g_pMonsters[g_MonsterCount++] = new MonsterSlime(position, level);
}

void Monster_CreateWolf(const XMFLOAT3& position, int level)
{
    if (g_MonsterCount >= MAX_FIELD_MONSTERS) return;

    g_pMonsters[g_MonsterCount++] = new MonsterWolf(position, level);
}

void Monster_CreateDragon(const XMFLOAT3& position, int level)
{
    if (g_MonsterCount >= MAX_FIELD_MONSTERS) return;

    g_pMonsters[g_MonsterCount++] = new MonsterDragon(position, level);
}

int Monster_GetCount()
{
    return g_MonsterCount;
}

Monster* Monster_Get(int index)
{
    assert(index >= 0 && index < g_MonsterCount);
    return g_pMonsters[index];
}

void Monster_Remove(Monster* monster)
{
    for (int i = 0; i < g_MonsterCount; i++) {
        if (g_pMonsters[i] == monster) {
            delete g_pMonsters[i];
            g_pMonsters[i] = g_pMonsters[--g_MonsterCount];
            return;
        }
    }
}
