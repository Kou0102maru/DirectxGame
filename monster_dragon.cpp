/*==============================================================================

   ドラゴンモンスター [monster_dragon.cpp]
                                                         Author : 
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "monster_dragon.h"
#include "player.h"
#include "collision.h"
#include "cube.h"
#include "light.h"
#include "player_camera.h"
#include "texture.h"
#include <DirectXMath.h>
using namespace DirectX;

// 検知範囲
static constexpr float DETECTION_RADIUS = 20.0f;
// 旋回速度
static constexpr float CIRCLE_SPEED = 1.0f;
// 突進速度
static constexpr float DIVE_SPEED = 8.0f;
// 追跡をあきらめる時間
static constexpr double GIVE_UP_TIME = 4.0;
// 飛行高度
static constexpr float FLY_HEIGHT = 5.0f;

extern int g_MonsterTexDragon;

//=============================================================================
// コンストラクタ
//=============================================================================
MonsterDragon::MonsterDragon(const XMFLOAT3& position, int level)
    : Monster(MONSTER_KIND_DRAGON, position, level)
{
    m_position.y = FLY_HEIGHT;  // 空中に配置
    ChangeState(new StateCircle(this));
}

//=============================================================================
// 旋回状態（円を描いて飛ぶ）
//=============================================================================
MonsterDragon::StateCircle::StateCircle(MonsterDragon* pOwner)
    : m_pOwner(pOwner)
    , m_center_position(pOwner->m_position)
{
}

void MonsterDragon::StateCircle::Update(double elapsed_time)
{
    m_accumulated_time += elapsed_time;

    // 円を描いて移動
    float angle = (float)m_accumulated_time * CIRCLE_SPEED;
    m_pOwner->m_position.x = m_center_position.x + cosf(angle) * m_radius;
    m_pOwner->m_position.z = m_center_position.z + sinf(angle) * m_radius;
    m_pOwner->m_position.y = FLY_HEIGHT;

    // 進行方向を計算
    XMFLOAT3 prev_pos = { 
        m_center_position.x + cosf(angle - 0.1f) * m_radius,
        FLY_HEIGHT,
        m_center_position.z + sinf(angle - 0.1f) * m_radius
    };
    XMVECTOR direction = XMLoadFloat3(&m_pOwner->m_position) - XMLoadFloat3(&prev_pos);
    XMStoreFloat3(&m_pOwner->m_front, XMVector3Normalize(direction));

    // プレイヤーが近づいたら突進
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
    float distance = XMVectorGetX(XMVector3Length(to_player));

    if (distance <= DETECTION_RADIUS) {
        m_pOwner->ChangeState(new StateDive(m_pOwner));
    }
}

void MonsterDragon::StateCircle::Draw() const
{
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 4.0f, { 0.8f, 0.2f, 0.2f, 1.0f });

    XMMATRIX world = XMMatrixTranslation(
        m_pOwner->m_position.x, 
        m_pOwner->m_position.y, 
        m_pOwner->m_position.z);
    Cube_Draw(g_MonsterTexDragon, world);
}

//=============================================================================
// 突進状態
//=============================================================================
MonsterDragon::StateDive::StateDive(MonsterDragon* pOwner)
    : m_pOwner(pOwner)
{
}

void MonsterDragon::StateDive::Update(double elapsed_time)
{
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
    float distance = XMVectorGetX(XMVector3Length(to_player));

    if (distance <= DETECTION_RADIUS) {
        // 急降下攻撃
        XMVECTOR direction = XMVector3Normalize(to_player);
        XMVECTOR position = XMLoadFloat3(&m_pOwner->m_position);
        position += direction * DIVE_SPEED * (float)elapsed_time;
        XMStoreFloat3(&m_pOwner->m_position, position);
        XMStoreFloat3(&m_pOwner->m_front, direction);

        m_give_up_time = 0.0;
    }
    else {
        m_give_up_time += elapsed_time;
        if (m_give_up_time >= GIVE_UP_TIME) {
            // 空中に戻る
            m_pOwner->m_position.y = FLY_HEIGHT;
            m_pOwner->ChangeState(new StateCircle(m_pOwner));
        }
    }
}

void MonsterDragon::StateDive::Draw() const
{
    // 突進中は明るく
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 5.0f, { 1.0f, 0.3f, 0.3f, 1.0f });

    XMMATRIX world = XMMatrixTranslation(
        m_pOwner->m_position.x, 
        m_pOwner->m_position.y, 
        m_pOwner->m_position.z);
    Cube_Draw(g_MonsterTexDragon, world);
}
