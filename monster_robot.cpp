/*==============================================================================

   ロボットモンスター [monster_robot.cpp]
                                                        Author :
                                                        Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "monster_robot.h"
#include "player.h"
#include "collision.h"
#include "cube.h"
#include "light.h"
#include "player_camera.h"
#include "texture.h"
#include <DirectXMath.h>
using namespace DirectX;

// 検知範囲
static constexpr float DETECTION_RADIUS = 10.0f;
// 行進範囲
static constexpr float MARCH_RANGE = 8.0f;
// 行進速度
static constexpr float MARCH_SPEED = 1.5f;
// 追跡速度
static constexpr float CHASE_SPEED = 3.0f;
// 追跡を諦める時間
static constexpr double GIVE_UP_TIME = 3.0;

// フィールド用モデルスケール
static constexpr float ROBOT_FIELD_SCALE = 0.3f;

// 共有モデル
MODEL* MonsterRobot::s_pModel = nullptr;
int    MonsterRobot::s_RefCount = 0;

//=============================================================================
// コンストラクタ
//=============================================================================
MonsterRobot::MonsterRobot(const XMFLOAT3& position, int level)
    : Monster(MONSTER_KIND_ROBOT, position, level)
{
    if (s_RefCount == 0) {
        s_pModel = ModelLoad("resource/model/robot.fbx", ROBOT_FIELD_SCALE, true);
    }
    s_RefCount++;

    ChangeState(new StateMarch(this));
}

//=============================================================================
// 行進状態
//=============================================================================
MonsterRobot::StateMarch::StateMarch(MonsterRobot* pOwner)
    : m_pOwner(pOwner)
    , m_start_z(pOwner->m_position.z)
{
}

void MonsterRobot::StateMarch::Update(double elapsed_time)
{
    // Z方向に直進
    m_pOwner->m_position.z += m_direction * MARCH_SPEED * (float)elapsed_time;

    // 範囲を超えたら折り返し
    if (m_pOwner->m_position.z > m_start_z + MARCH_RANGE) {
        m_direction = -1.0f;
    }
    else if (m_pOwner->m_position.z < m_start_z - MARCH_RANGE) {
        m_direction = 1.0f;
    }

    // 進行方向を更新
    m_pOwner->m_front = { 0.0f, 0.0f, m_direction };

    // プレイヤー検知
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
    float distance = XMVectorGetX(XMVector3Length(to_player));

    if (distance <= DETECTION_RADIUS) {
        m_pOwner->ChangeState(new StateChase(m_pOwner));
    }
}

void MonsterRobot::StateMarch::Draw() const
{
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.3f, 0.3f, 0.4f, 1.0f });

    if (s_pModel) {
        float s = m_pOwner->GetFieldScale();
        XMMATRIX scale = XMMatrixScaling(s, s, s);
        float yAngle = atan2f(-m_pOwner->m_front.z, m_pOwner->m_front.x);
        XMMATRIX rot = XMMatrixRotationY(yAngle);
        XMMATRIX trans = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y,
            m_pOwner->m_position.z);
        XMMATRIX world = scale * rot * trans;
        ModelDraw(s_pModel, world, { 0.4f, 0.4f, 0.5f, 1.0f });
    } else {
        extern int g_MonsterTexRobot;
        XMMATRIX world = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y,
            m_pOwner->m_position.z);
        Cube_Draw(g_MonsterTexRobot, world);
    }
}

//=============================================================================
// 追跡状態
//=============================================================================
MonsterRobot::StateChase::StateChase(MonsterRobot* pOwner)
    : m_pOwner(pOwner)
{
}

void MonsterRobot::StateChase::Update(double elapsed_time)
{
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
    float distance = XMVectorGetX(XMVector3Length(to_player));

    if (distance <= DETECTION_RADIUS) {
        XMVECTOR direction = XMVector3Normalize(to_player);
        XMVECTOR position = XMLoadFloat3(&m_pOwner->m_position);
        position += direction * CHASE_SPEED * (float)elapsed_time;
        XMStoreFloat3(&m_pOwner->m_position, position);

        m_pOwner->m_position.y = 1.5f;

        XMStoreFloat3(&m_pOwner->m_front, direction);

        m_give_up_time = 0.0;
    }
    else {
        m_give_up_time += elapsed_time;
        if (m_give_up_time >= GIVE_UP_TIME) {
            m_pOwner->ChangeState(new StateMarch(m_pOwner));
        }
    }
}

void MonsterRobot::StateChase::Draw() const
{
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.3f, 0.3f, 0.4f, 1.0f });

    if (s_pModel) {
        float s = m_pOwner->GetFieldScale();
        XMMATRIX scale = XMMatrixScaling(s, s, s);
        float yAngle = atan2f(-m_pOwner->m_front.z, m_pOwner->m_front.x);
        XMMATRIX rot = XMMatrixRotationY(yAngle);
        XMMATRIX trans = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y,
            m_pOwner->m_position.z);
        XMMATRIX world = scale * rot * trans;
        ModelDraw(s_pModel, world, { 0.4f, 0.4f, 0.5f, 1.0f });
    } else {
        extern int g_MonsterTexRobot;
        XMMATRIX world = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y,
            m_pOwner->m_position.z);
        Cube_Draw(g_MonsterTexRobot, world);
    }
}
