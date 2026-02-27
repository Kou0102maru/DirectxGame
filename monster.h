/*==============================================================================

   モンスター管理 [monster.h]
                                                         Author : 
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef MONSTER_H
#define MONSTER_H

#include <DirectXMath.h>
#include <string>
#include <vector>
#include "collision.h"

//=============================================================================
// モンスターの種類ID
//=============================================================================
enum MonsterKind
{
    MONSTER_KIND_SLIME = 0,   // スライム
    MONSTER_KIND_WOLF,        // オオカミ
    MONSTER_KIND_DRAGON,      // ドラゴン
    MONSTER_KIND_MAX
};

//=============================================================================
// スキルの定義
//=============================================================================
enum SkillId
{
    SKILL_NONE = -1,
    SKILL_ATTACK = 0,    // 通常攻撃
    SKILL_FIRE,          // ファイア
    SKILL_HEAL,          // ヒール
    SKILL_MAX
};

struct Skill
{
    SkillId     id;
    std::string name;     // スキル名
    int         power;    // 威力
    int         mp_cost;  // MP消費
};

//=============================================================================
// モンスターの基本パラメータ（種族ごとの固定値）
//=============================================================================
struct MonsterBaseParam
{
    MonsterKind kind;
    std::string name;        // モンスター名
    int         base_hp;     // 基本HP
    int         base_mp;     // 基本MP
    int         base_atk;    // 基本攻撃力
    int         base_def;    // 基本防御力
    int         base_spd;    // 基本素早さ
    int         base_exp;    // 倒したときの経験値
    SkillId     skills[4];   // 覚えているスキル（最大4つ）
};

//=============================================================================
// モンスタークラス（実際にフィールドやパーティに存在する個体）
//=============================================================================
class Monster
{
protected:
    // ステートマシン用内部クラス
    class State
    {
    public:
        virtual ~State() = default;
        virtual void Update(double elapsed_time) = 0;
        virtual void Draw() const = 0;
    };

private:
    State* m_pState{};
    State* m_pNextState{};

protected:
    // 種族・外見
    MonsterKind             m_kind{};
    std::string             m_name{};

    // 位置・方向
    DirectX::XMFLOAT3       m_position{};
    DirectX::XMFLOAT3       m_front{ 0.0f, 0.0f, 1.0f };
    DirectX::XMFLOAT3       m_velocity{};

    // 現在のパラメータ
    int                     m_level{ 1 };
    int                     m_hp{};
    int                     m_hp_max{};
    int                     m_mp{};
    int                     m_mp_max{};
    int                     m_atk{};
    int                     m_def{};
    int                     m_spd{};
    int                     m_exp{};         // 蓄積経験値
    int                     m_exp_next{};    // 次のレベルまでの経験値

    // スキル
    SkillId                 m_skills[4]{ SKILL_NONE, SKILL_NONE, SKILL_NONE, SKILL_NONE };

    // 仲間かどうか
    bool                    m_is_party_member{ false };

    // 生死
    bool                    m_is_dead{ false };

public:
    Monster() = default;
    Monster(MonsterKind kind, const DirectX::XMFLOAT3& position, int level = 1);
    virtual ~Monster() = default;

    // 更新・描画
    virtual void Update(double elapsed_time);
    virtual void Draw() const;
    void UpdateState();

    // 戦闘
    void TakeDamage(int damage);
    void Heal(int amount);
    bool UseSkill(SkillId skill_id, Monster* target);
    bool IsDead() const { return m_is_dead; }

    // 成長
    void GainExp(int exp);
    void LevelUp();

    // 仲間化
    void JoinParty() { m_is_party_member = true; }
    bool IsPartyMember() const { return m_is_party_member; }

    // 衝突判定
    virtual Sphere GetCollision() const;

    // ゲッター
    MonsterKind             GetKind()     const { return m_kind; }
    const std::string&      GetName()     const { return m_name; }
    int                     GetLevel()    const { return m_level; }
    int                     GetHp()       const { return m_hp; }
    int                     GetHpMax()    const { return m_hp_max; }
    int                     GetMp()       const { return m_mp; }
    int                     GetMpMax()    const { return m_mp_max; }
    int                     GetAtk()      const { return m_atk; }
    int                     GetDef()      const { return m_def; }
    int                     GetSpd()      const { return m_spd; }
    int                     GetExp()      const { return m_exp; }
    int                     GetExpNext()  const { return m_exp_next; }
    const DirectX::XMFLOAT3& GetPosition() const { return m_position; }
    const DirectX::XMFLOAT3& GetFront()    const { return m_front; }
    const SkillId*          GetSkills()   const { return m_skills; }

    // セッター
    void SetPosition(const DirectX::XMFLOAT3& pos) { m_position = pos; }

protected:
    void ChangeState(State* pNext);
    void InitParams(MonsterKind kind, int level);  // パラメータ初期化
};

//=============================================================================
// 種族別パラメータテーブル（外部から参照可能）
//=============================================================================
const MonsterBaseParam* Monster_GetBaseParam(MonsterKind kind);
const Skill*            Monster_GetSkillData(SkillId skill_id);

//=============================================================================
// フィールド上のモンスター群管理（敵として出現するモンスター）
//=============================================================================
void    Monster_Initialize();
void    Monster_Finalize();
void    Monster_Update(double elapsed_time);
void    Monster_Draw();
void    Monster_Create(MonsterKind kind, const DirectX::XMFLOAT3& position, int level = 1);
void    Monster_CreateSlime(const DirectX::XMFLOAT3& position, int level = 1);   // AI付きスライム生成
void    Monster_CreateWolf(const DirectX::XMFLOAT3& position, int level = 1);    // AI付きオオカミ生成
void    Monster_CreateDragon(const DirectX::XMFLOAT3& position, int level = 1);  // AI付きドラゴン生成
int     Monster_GetCount();
Monster* Monster_Get(int index);
void Monster_Remove(Monster* monster);

#endif // MONSTER_H
