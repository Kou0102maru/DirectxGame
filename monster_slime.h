/*==============================================================================

   スライムモンスター [monster_slime.h]
                                                         Author : 
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef MONSTER_SLIME_H
#define MONSTER_SLIME_H

#include "monster.h"
#include <DirectXMath.h>

class MonsterSlime : public Monster
{
public:
    MonsterSlime(const DirectX::XMFLOAT3& position, int level = 1);
    virtual ~MonsterSlime() = default;

private:
    // パトロール状態
    class StatePatrol : public State
    {
    private:
        MonsterSlime* m_pOwner;
        float m_start_x;
        double m_accumulated_time{ 0.0 };

    public:
        StatePatrol(MonsterSlime* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    // 追跡状態
    class StateChase : public State
    {
    private:
        MonsterSlime* m_pOwner;
        double m_give_up_time{ 0.0 };  // 追跡をあきらめるまでの時間

    public:
        StateChase(MonsterSlime* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };
};

#endif // MONSTER_SLIME_H
