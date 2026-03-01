/*==============================================================================

   目玉モンスター [monster_eyeball.h]
                                                        Author :
                                                        Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef MONSTER_EYEBALL_H
#define MONSTER_EYEBALL_H

#include "monster.h"
#include "model.h"

class MonsterEyeball : public Monster
{
public:
    MonsterEyeball(const DirectX::XMFLOAT3& position, int level = 1);
    virtual ~MonsterEyeball() = default;

private:
    static MODEL* s_pModel;
    static int    s_RefCount;

    // 浮遊状態（上下にふわふわ）
    class StateFloat : public State
    {
    private:
        MonsterEyeball* m_pOwner;
        float m_base_y;
        double m_accumulated_time{ 0.0 };
    public:
        StateFloat(MonsterEyeball* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    // 追跡状態
    class StateChase : public State
    {
    private:
        MonsterEyeball* m_pOwner;
        double m_give_up_time{ 0.0 };
    public:
        StateChase(MonsterEyeball* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };
};

#endif // MONSTER_EYEBALL_H
