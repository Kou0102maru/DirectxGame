/*==============================================================================

   オオカミモンスター [monster_wolf.cpp]
                                                         Author : 
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "monster_wolf.h"
#include "player.h"
#include "collision.h"
#include "cube.h"
#include "light.h"
#include "player_camera.h"
#include "texture.h"
#include <DirectXMath.h>
using namespace DirectX;
#include <cstdlib>

// 検知範囲
static constexpr float DETECTION_RADIUS = 10.0f;
// 徘徊速度
static constexpr float ROAM_SPEED = 4.0f;
// 追跡速度
static constexpr float CHASE_SPEED = 3.0f;
// 追跡をあきらめる時間
static constexpr double GIVE_UP_TIME = 5.0;
// ターゲット変更間隔
static constexpr double CHANGE_TARGET_INTERVAL = 3.0;

extern int g_MonsterTexWolf;

//=============================================================================
// コンストラクタ
//=============================================================================
MonsterWolf::MonsterWolf(const XMFLOAT3& position, int level)
    : Monster(MONSTER_KIND_WOLF, position, level)
{
    ChangeState(new StateRoam(this));
}

//=============================================================================
// 徘徊状態（速く走り回る）
//=============================================================================
MonsterWolf::StateRoam::StateRoam(MonsterWolf* pOwner)
    : m_pOwner(pOwner)
{
    SetNewTarget();
}

void MonsterWolf::StateRoam::SetNewTarget()
{
    // ランダムな位置を設定
    float random_x = ((float)rand() / RAND_MAX) * 30.0f - 15.0f;
    float random_z = ((float)rand() / RAND_MAX) * 30.0f - 15.0f;
    m_target_position = { random_x, 0.5f, random_z };
    m_change_target_time = 0.0;
}

void MonsterWolf::StateRoam::Update(double elapsed_time)
{
    m_change_target_time += elapsed_time;

    // ターゲット位置に向かって移動
    XMVECTOR target = XMLoadFloat3(&m_target_position);
    XMVECTOR current = XMLoadFloat3(&m_pOwner->m_position);
    XMVECTOR to_target = target - current;
    float distance = XMVectorGetX(XMVector3Length(to_target));

    // ターゲットに到着したか、一定時間経過したら新しいターゲット
    if (distance < 1.0f || m_change_target_time >= CHANGE_TARGET_INTERVAL) {
        SetNewTarget();
    }

    // 移動
    XMVECTOR direction = XMVector3Normalize(to_target);
    current += direction * ROAM_SPEED * (float)elapsed_time;
    XMStoreFloat3(&m_pOwner->m_position, current);
    XMStoreFloat3(&m_pOwner->m_front, direction);

    // プレイヤーが近づいたら追跡開始
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - current;
    float player_distance = XMVectorGetX(XMVector3Length(to_player));

    if (player_distance <= DETECTION_RADIUS) {
        m_pOwner->ChangeState(new StateChase(m_pOwner));
    }
}

void MonsterWolf::StateRoam::Draw() const
{
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.6f, 0.5f, 0.3f, 1.0f });

    XMMATRIX world = XMMatrixTranslation(
        m_pOwner->m_position.x, 
        m_pOwner->m_position.y, 
        m_pOwner->m_position.z);
    Cube_Draw(g_MonsterTexWolf, world);
}

//=============================================================================
// 追跡状態
//=============================================================================
MonsterWolf::StateChase::StateChase(MonsterWolf* pOwner)
    : m_pOwner(pOwner)
{
}

void MonsterWolf::StateChase::Update(double elapsed_time)
{
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
    float distance = XMVectorGetX(XMVector3Length(to_player));

    if (distance <= DETECTION_RADIUS) {
        // 全力疾走でプレイヤーを追跡
        XMVECTOR direction = XMVector3Normalize(to_player);
        XMVECTOR position = XMLoadFloat3(&m_pOwner->m_position);
        position += direction * CHASE_SPEED * (float)elapsed_time;
        XMStoreFloat3(&m_pOwner->m_position, position);
        XMStoreFloat3(&m_pOwner->m_front, direction);

        // y座標を固定
        m_pOwner->m_position.y = 0.5f;
        XMStoreFloat3(&m_pOwner->m_front, direction);

        m_give_up_time = 0.0;
    }
    else {
        m_give_up_time += elapsed_time;
        if (m_give_up_time >= GIVE_UP_TIME) {
            m_pOwner->ChangeState(new StateRoam(m_pOwner));
        }
    }
}

void MonsterWolf::StateChase::Draw() const
{
    // 追跡中は赤っぽく
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 3.0f, { 0.8f, 0.4f, 0.2f, 1.0f });

    XMMATRIX world = XMMatrixTranslation(
        m_pOwner->m_position.x, 
        m_pOwner->m_position.y, 
        m_pOwner->m_position.z);
    Cube_Draw(g_MonsterTexWolf, world);
}
