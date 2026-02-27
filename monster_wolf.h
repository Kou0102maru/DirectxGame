/*==============================================================================

   オオカミモンスター [monster_wolf.h]
                                                         Author : 
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef MONSTER_WOLF_H
#define MONSTER_WOLF_H

#include "monster.h"
#include <DirectXMath.h>

class MonsterWolf : public Monster
{
public:
    MonsterWolf(const DirectX::XMFLOAT3& position, int level = 1);
    virtual ~MonsterWolf() = default;

private:
    // 徘徊状態（速く走り回る）
    class StateRoam : public State
    {
    private:
        MonsterWolf* m_pOwner;
        DirectX::XMFLOAT3 m_target_position;
        double m_change_target_time{ 0.0 };

    public:
        StateRoam(MonsterWolf* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
        void SetNewTarget();
    };

    // 追跡状態（プレイヤーに向かって全力疾走）
    class StateChase : public State
    {
    private:
        MonsterWolf* m_pOwner;
        double m_give_up_time{ 0.0 };

    public:
        StateChase(MonsterWolf* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };
};

#endif // MONSTER_WOLF_H
