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
#include "model.h"
#include <DirectXMath.h>
using namespace DirectX;
#include <cstdlib>

// 索敵範囲
static constexpr float DETECTION_RADIUS = 10.0f;
// 徘徊速度
static constexpr float ROAM_SPEED = 4.0f;
// 追跡速度
static constexpr float CHASE_SPEED = 3.0f;
// 追跡を諦めるまでの時間
static constexpr double GIVE_UP_TIME = 5.0;
// ターゲット変更間隔
static constexpr double CHANGE_TARGET_INTERVAL = 3.0;

// フィールド用オオカミモデル（一度ロードしたら保持）
static MODEL* g_pWolfFieldModel = nullptr;
static constexpr float WOLF_FIELD_SCALE = 3.0f;

extern int g_MonsterTexWolf;

//=============================================================================
// コンストラクタ
//=============================================================================
MonsterWolf::MonsterWolf(const XMFLOAT3& position, int level)
    : Monster(MONSTER_KIND_WOLF, position, level)
{
    // モデルを一度だけロード（スケール変更時は再起動で反映）
    if (!g_pWolfFieldModel) {
        g_pWolfFieldModel = ModelLoad("resource/model/Wolf.fbx", WOLF_FIELD_SCALE, true);
    }
    ChangeState(new StateRoam(this));
}

//=============================================================================
// 徘徊状態（ランダム移動）
//=============================================================================
MonsterWolf::StateRoam::StateRoam(MonsterWolf* pOwner)
    : m_pOwner(pOwner)
{
    SetNewTarget();
}

void MonsterWolf::StateRoam::SetNewTarget()
{
    // ランダムな位置を設定
    float random_x = ((float)rand() / RAND_MAX) * 80.0f - 40.0f;
    float random_z = ((float)rand() / RAND_MAX) * 80.0f - 40.0f;
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

    // 進行方向を向くよう Y 軸回転を計算（bBlenderで座標系変換済み）
    float angle = -atan2f(m_pOwner->m_front.z, m_pOwner->m_front.x) + XMConvertToRadians(270);
    XMMATRIX rotY   = XMMatrixRotationY(angle);
    XMMATRIX trans = XMMatrixTranslation(
        m_pOwner->m_position.x,
        m_pOwner->m_position.y,
        m_pOwner->m_position.z);
    XMMATRIX world = rotY * trans;

    if (g_pWolfFieldModel) {
        ModelDraw(g_pWolfFieldModel, world);
    } else {
        Cube_Draw(g_MonsterTexWolf, trans);
    }
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
        // 全力でプレイヤーを追跡
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
    // 追跡中は赤みがかった光
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.6f, 0.5f, 0.3f, 1.0f });

    float angle = -atan2f(m_pOwner->m_front.z, m_pOwner->m_front.x) + XMConvertToRadians(270);
    XMMATRIX rotY   = XMMatrixRotationY(angle);
    XMMATRIX trans = XMMatrixTranslation(
        m_pOwner->m_position.x,
        m_pOwner->m_position.y,
        m_pOwner->m_position.z);
    XMMATRIX world = rotY * trans;

    if (g_pWolfFieldModel) {
        ModelDraw(g_pWolfFieldModel, world);
    } else {
        Cube_Draw(g_MonsterTexWolf, trans);
    }
}
