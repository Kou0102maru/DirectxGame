/*==============================================================================

   ロボットモンスター [monster_robot.h]
                                                        Author :
                                                        Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef MONSTER_ROBOT_H
#define MONSTER_ROBOT_H

#include "monster.h"
#include "model.h"

class MonsterRobot : public Monster
{
public:
    MonsterRobot(const DirectX::XMFLOAT3& position, int level = 1);
    virtual ~MonsterRobot() = default;

private:
    static MODEL* s_pModel;
    static int    s_RefCount;

    // 行進状態（直進して折り返す）
    class StateMarch : public State
    {
    private:
        MonsterRobot* m_pOwner;
        float m_start_z;
        float m_direction{ 1.0f };  // +1 or -1
    public:
        StateMarch(MonsterRobot* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    // 追跡状態
    class StateChase : public State
    {
    private:
        MonsterRobot* m_pOwner;
        double m_give_up_time{ 0.0 };
    public:
        StateChase(MonsterRobot* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };
};

#endif // MONSTER_ROBOT_H
