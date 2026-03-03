/*==============================================================================

   目玉モンスター [monster_eyeball.cpp]
                                                        Author :
                                                        Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "monster_eyeball.h"
#include "player.h"
#include "collision.h"
#include "cube.h"
#include "light.h"
#include "player_camera.h"
#include "texture.h"
#include <DirectXMath.h>
using namespace DirectX;

// 検知範囲
static constexpr float DETECTION_RADIUS = 12.0f;
// 浮遊振幅
static constexpr float FLOAT_AMPLITUDE = 1.0f;
// 浮遊速度
static constexpr float FLOAT_SPEED = 2.0f;
// 追跡速度
static constexpr float CHASE_SPEED = 2.0f;
// 追跡を諦める時間
static constexpr double GIVE_UP_TIME = 4.0;

// フィールド用モデルスケール
static constexpr float EYEBALL_FIELD_SCALE = 0.3f;

// 共有モデル
MODEL* MonsterEyeball::s_pModel = nullptr;
int    MonsterEyeball::s_RefCount = 0;

//=============================================================================
// コンストラクタ
//=============================================================================
MonsterEyeball::MonsterEyeball(const XMFLOAT3& position, int level)
    : Monster(MONSTER_KIND_EYEBALL, position, level)
{
    if (s_RefCount == 0) {
        s_pModel = ModelLoad("resource/model/eyeball.fbx", EYEBALL_FIELD_SCALE, true);
    }
    s_RefCount++;

    ChangeState(new StateFloat(this));
}

//=============================================================================
// 浮遊状態
//=============================================================================
MonsterEyeball::StateFloat::StateFloat(MonsterEyeball* pOwner)
    : m_pOwner(pOwner)
    , m_base_y(pOwner->m_position.y)
{
}

void MonsterEyeball::StateFloat::Update(double elapsed_time)
{
    m_accumulated_time += elapsed_time;

    // 上下にふわふわ浮遊
    m_pOwner->m_position.y = m_base_y + sinf((float)m_accumulated_time * FLOAT_SPEED) * FLOAT_AMPLITUDE;

    // プレイヤー検知
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
    float distance = XMVectorGetX(XMVector3Length(to_player));

    if (distance <= DETECTION_RADIUS) {
        m_pOwner->ChangeState(new StateChase(m_pOwner));
    }
}

void MonsterEyeball::StateFloat::Draw() const
{
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.5f, 0.1f, 0.1f, 1.0f });

    if (s_pModel) {
        // プレイヤー方向を常に見る（目玉らしい挙動）
        float s = m_pOwner->GetFieldScale();
        XMMATRIX scale = XMMatrixScaling(s, s, s);
        XMFLOAT3 player_pos = Player_GetPosition();
        XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
        XMFLOAT3 dir;
        XMStoreFloat3(&dir, XMVector3Normalize(to_player));
        float yAngle = atan2f(dir.z, -dir.x);
        XMMATRIX rot = XMMatrixRotationZ(XM_PIDIV2) * XMMatrixRotationY(yAngle);
        XMMATRIX trans = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y,
            m_pOwner->m_position.z);
        XMMATRIX world = scale * rot * trans;
        ModelDraw(s_pModel, world, { 0.8f, 0.1f, 0.1f, 1.0f });
    } else {
        extern int g_MonsterTexEyeball;
        XMMATRIX world = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y,
            m_pOwner->m_position.z);
        Cube_Draw(g_MonsterTexEyeball, world);
    }
}

//=============================================================================
// 追跡状態
//=============================================================================
MonsterEyeball::StateChase::StateChase(MonsterEyeball* pOwner)
    : m_pOwner(pOwner)
{
}

void MonsterEyeball::StateChase::Update(double elapsed_time)
{
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
    float distance = XMVectorGetX(XMVector3Length(to_player));

    if (distance <= DETECTION_RADIUS) {
        XMVECTOR direction = XMVector3Normalize(to_player);
        XMVECTOR position = XMLoadFloat3(&m_pOwner->m_position);
        position += direction * CHASE_SPEED * (float)elapsed_time;
        XMStoreFloat3(&m_pOwner->m_position, position);

        // 浮遊高度を維持（ボスでも当たれる高さ）
        m_pOwner->m_position.y = 1.0f;

        XMStoreFloat3(&m_pOwner->m_front, direction);

        m_give_up_time = 0.0;
    }
    else {
        m_give_up_time += elapsed_time;
        if (m_give_up_time >= GIVE_UP_TIME) {
            m_pOwner->ChangeState(new StateFloat(m_pOwner));
        }
    }
}

void MonsterEyeball::StateChase::Draw() const
{
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.5f, 0.1f, 0.1f, 1.0f });

    if (s_pModel) {
        float s = m_pOwner->GetFieldScale();
        XMMATRIX scale = XMMatrixScaling(s, s, s);
        float yAngle = atan2f(m_pOwner->m_front.z, -m_pOwner->m_front.x);
        XMMATRIX rot = XMMatrixRotationZ(XM_PIDIV2) * XMMatrixRotationY(yAngle);
        XMMATRIX trans = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y,
            m_pOwner->m_position.z);
        XMMATRIX world = scale * rot * trans;
        ModelDraw(s_pModel, world, { 0.8f, 0.1f, 0.1f, 1.0f });
    } else {
        extern int g_MonsterTexEyeball;
        XMMATRIX world = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y,
            m_pOwner->m_position.z);
        Cube_Draw(g_MonsterTexEyeball, world);
    }
}
