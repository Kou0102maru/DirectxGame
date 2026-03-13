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
#include "model.h"
#include <DirectXMath.h>
using namespace DirectX;

// 索敵範囲
static constexpr float DETECTION_RADIUS = 15.0f;
// 旋回速度
static constexpr float CIRCLE_SPEED = 1.0f;
// 急降下速度
static constexpr float DIVE_SPEED = 5.0f;
// 追跡を諦めるまでの時間
static constexpr double GIVE_UP_TIME = 3.0;
// 飛行高度
static constexpr float FLY_HEIGHT = 3.0f;

// フィールド用ドラゴンモデル（一度ロードしたら保持）
static MODEL* g_pDragonFieldModel = nullptr;
static constexpr float DRAGON_FIELD_SCALE = 0.1f;

extern int g_MonsterTexDragon;

//=============================================================================
// コンストラクタ
//=============================================================================
MonsterDragon::MonsterDragon(const XMFLOAT3& position, int level)
    : Monster(MONSTER_KIND_DRAGON, position, level)
{
    // モデルを一度だけロード
    if (!g_pDragonFieldModel) {
        g_pDragonFieldModel = ModelLoad("resource/model/Dragon.fbx", DRAGON_FIELD_SCALE, true);
    }
    // ボス部屋で待機（どっしり構える）
    ChangeState(new StateCircle(this));
}

//=============================================================================
// 旋回状態（円を描いて飛行）
//=============================================================================
MonsterDragon::StateCircle::StateCircle(MonsterDragon* pOwner)
    : m_pOwner(pOwner)
    , m_center_position(pOwner->m_position)
{
}

void MonsterDragon::StateCircle::Update(double elapsed_time)
{
    // どっしり構えて待機（移動しない）
    // プレイヤーの方向をゆっくり向く
    XMFLOAT3 player_pos = Player_GetPosition();
    XMVECTOR to_player = XMLoadFloat3(&player_pos) - XMLoadFloat3(&m_pOwner->m_position);
    float distance = XMVectorGetX(XMVector3Length(to_player));

    if (distance < 50.0f) {
        // プレイヤーが見える範囲ならそちらを向く
        XMVECTOR dir = XMVector3Normalize(to_player);
        XMFLOAT3 target_front;
        XMStoreFloat3(&target_front, dir);
        // ゆっくり向きを補間
        float t = 2.0f * (float)elapsed_time;
        if (t > 1.0f) t = 1.0f;
        m_pOwner->m_front.x += (target_front.x - m_pOwner->m_front.x) * t;
        m_pOwner->m_front.z += (target_front.z - m_pOwner->m_front.z) * t;
    }

    // 動かず構えるだけ（攻撃遷移なし）
}

void MonsterDragon::StateCircle::Draw() const
{
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 0.5f, { 0.1f, 0.1f, 0.2f, 1.0f });

    // 進行方向を向くよう Y 軸回転を計算（bBlenderで座標系変換済み）
    float s = m_pOwner->GetFieldScale();
    XMMATRIX scale = XMMatrixScaling(s, s, s);
    float angle = -atan2f(m_pOwner->m_front.z, m_pOwner->m_front.x) + XMConvertToRadians(270);
    XMMATRIX rotY   = XMMatrixRotationY(angle);
    XMMATRIX trans = XMMatrixTranslation(
        m_pOwner->m_position.x,
        m_pOwner->m_position.y,
        m_pOwner->m_position.z);
    XMMATRIX world = scale * rotY * trans;

    if (g_pDragonFieldModel) {
        ModelDraw(g_pDragonFieldModel, world, { 0.0f, 0.0f, 1.0f, 1.0f });
    } else {
        Cube_Draw(g_MonsterTexDragon, trans);
    }
}

//=============================================================================
// 急降下状態
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
        // まっすぐ急降下
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
            // 元の位置に戻って構える
            m_pOwner->ChangeState(new StateCircle(m_pOwner));
        }
    }
}

void MonsterDragon::StateDive::Draw() const
{
    // 急降下中は明るく
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 0.5f, { 0.1f, 0.1f, 0.2f, 1.0f });

    // 進行方向を向くよう Y 軸回転を計算（bBlenderで座標系変換済み）
    float s = m_pOwner->GetFieldScale();
    XMMATRIX scale = XMMatrixScaling(s, s, s);
    float angle = -atan2f(m_pOwner->m_front.z, m_pOwner->m_front.x) + XMConvertToRadians(270);
    XMMATRIX rotY   = XMMatrixRotationY(angle);
    XMMATRIX trans = XMMatrixTranslation(
        m_pOwner->m_position.x,
        m_pOwner->m_position.y,
        m_pOwner->m_position.z);
    XMMATRIX world = scale * rotY * trans;

    if (g_pDragonFieldModel) {
        ModelDraw(g_pDragonFieldModel, world, { 0.0f, 0.0f, 1.0f, 1.0f });
    } else {
        Cube_Draw(g_MonsterTexDragon, trans);
    }
}
