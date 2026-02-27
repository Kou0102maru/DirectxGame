/*==============================================================================

   ドラゴンモンスター [monster_dragon.h]
                                                         Author : 
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef MONSTER_DRAGON_H
#define MONSTER_DRAGON_H

#include "monster.h"
#include <DirectXMath.h>

class MonsterDragon : public Monster
{
public:
    MonsterDragon(const DirectX::XMFLOAT3& position, int level = 1);
    virtual ~MonsterDragon() = default;

private:
    // 旋回状態（空中を円を描いて飛ぶ）
    class StateCircle : public State
    {
    private:
        MonsterDragon* m_pOwner;
        DirectX::XMFLOAT3 m_center_position;
        double m_accumulated_time{ 0.0 };
        float m_radius{ 8.0f };

    public:
        StateCircle(MonsterDragon* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    // 突進状態（プレイヤーに向かって急降下）
    class StateDive : public State
    {
    private:
        MonsterDragon* m_pOwner;
        double m_give_up_time{ 0.0 };

    public:
        StateDive(MonsterDragon* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };
};

#endif // MONSTER_DRAGON_H
