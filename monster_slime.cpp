/*==============================================================================

   スライムモンスター [monster_slime.cpp]
                                                         Author : 
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "monster_slime.h"
#include "player.h"
#include "collision.h"
#include "cube.h"
#include "light.h"
#include "player_camera.h"
#include "texture.h"
#include <DirectXMath.h>
using namespace DirectX;

// 検知範囲
static constexpr float DETECTION_RADIUS = 8.0f;
// パトロール移動範囲
static constexpr float PATROL_RANGE = 5.0f;
// 追跡速度（遅くした）
static constexpr float CHASE_SPEED = 2.5f;
// 追跡をあきらめる時間
static constexpr double GIVE_UP_TIME = 3.0;

// テクスチャID（monster.cppで定義されているものを参照）
extern int g_MonsterTexSlime;

//=============================================================================
// コンストラクタ
//=============================================================================
MonsterSlime::MonsterSlime(const XMFLOAT3& position, int level)
    : Monster(MONSTER_KIND_SLIME, position, level)
{
    // 初期状態はパトロール
    ChangeState(new StatePatrol(this));
}

//=============================================================================
// パトロール状態
//=============================================================================
MonsterSlime::StatePatrol::StatePatrol(MonsterSlime* pOwner)
    : m_pOwner(pOwner)
    , m_start_x(pOwner->m_position.x)
{
}

void MonsterSlime::StatePatrol::Update(double elapsed_time)
{
    m_accumulated_time += elapsed_time;

    // 左右にゆっくり移動
    m_pOwner->m_position.x = m_start_x + sinf((float)m_accumulated_time) * PATROL_RANGE;

    // プレイヤーが近づいたら追跡開始
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
    float distance = XMVectorGetX(XMVector3Length(to_player));

    if (distance <= DETECTION_RADIUS) {
        m_pOwner->ChangeState(new StateChase(m_pOwner));
    }
}

void MonsterSlime::StatePatrol::Draw() const
{
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.2f, 0.8f, 0.2f, 1.0f });

    XMMATRIX world = XMMatrixTranslation(
        m_pOwner->m_position.x, 
        m_pOwner->m_position.y, 
        m_pOwner->m_position.z);
    Cube_Draw(g_MonsterTexSlime, world);
}

//=============================================================================
// 追跡状態
//=============================================================================
MonsterSlime::StateChase::StateChase(MonsterSlime* pOwner)
    : m_pOwner(pOwner)
{
}

void MonsterSlime::StateChase::Update(double elapsed_time)
{
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
    float distance = XMVectorGetX(XMVector3Length(to_player));

    // プレイヤーが範囲内なら追跡
    if (distance <= DETECTION_RADIUS) {
        // プレイヤーに向かって移動
        XMVECTOR direction = XMVector3Normalize(to_player);
        XMVECTOR position = XMLoadFloat3(&m_pOwner->m_position);
        position += direction * CHASE_SPEED * (float)elapsed_time;
        XMStoreFloat3(&m_pOwner->m_position, position);

        // y座標を固定（地面にめり込まないように）
        m_pOwner->m_position.y = 0.5f;

        // 向きを更新
        XMStoreFloat3(&m_pOwner->m_front, direction);

        // あきらめタイマーをリセット
        m_give_up_time = 0.0;
    }
    else {
        // プレイヤーが離れたらあきらめタイマー開始
        m_give_up_time += elapsed_time;

        if (m_give_up_time >= GIVE_UP_TIME) {
            // パトロールに戻る
            m_pOwner->ChangeState(new StatePatrol(m_pOwner));
        }
    }
}

void MonsterSlime::StateChase::Draw() const
{
    // 追跡中は少し明るく
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 3.0f, { 0.3f, 1.0f, 0.3f, 1.0f });

    XMMATRIX world = XMMatrixTranslation(
        m_pOwner->m_position.x, 
        m_pOwner->m_position.y, 
        m_pOwner->m_position.z);
    Cube_Draw(g_MonsterTexSlime, world);
}
