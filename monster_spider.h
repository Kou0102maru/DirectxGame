/*==============================================================================

   クモモンスター [monster_spider.h]
                                                         Author :
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef MONSTER_SPIDER_H
#define MONSTER_SPIDER_H

#include "monster.h"
#include "model.h"
#include <DirectXMath.h>

class MonsterSpider : public Monster
{
public:
    MonsterSpider(const DirectX::XMFLOAT3& position, int level = 1);
    virtual ~MonsterSpider() = default;

private:
    static MODEL* s_pModel;
    static int    s_RefCount;

    // パトロール状態
    class StatePatrol : public State
    {
    private:
        MonsterSpider* m_pOwner;
        float m_start_x;
        double m_accumulated_time{ 0.0 };

    public:
        StatePatrol(MonsterSpider* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };

    // 追跡状態
    class StateChase : public State
    {
    private:
        MonsterSpider* m_pOwner;
        double m_give_up_time{ 0.0 };  // 追跡をあきらめるまでの時間

    public:
        StateChase(MonsterSpider* pOwner);
        void Update(double elapsed_time) override;
        void Draw() const override;
    };
};

#endif // MONSTER_SPIDER_H
