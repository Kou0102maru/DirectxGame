/*==============================================================================

   クモモンスター [monster_spider.cpp]
                                                         Author :
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "monster_spider.h"
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
// 追跡速度
static constexpr float CHASE_SPEED = 2.5f;
// 追跡をあきらめる時間
static constexpr double GIVE_UP_TIME = 3.0;

// フィールド用モデルスケール
static constexpr float SPIDER_FIELD_SCALE = 0.15f;

// 共有モデル（全クモインスタンスで共有）
MODEL* MonsterSpider::s_pModel = nullptr;
int    MonsterSpider::s_RefCount = 0;

//=============================================================================
// コンストラクタ
//=============================================================================
MonsterSpider::MonsterSpider(const XMFLOAT3& position, int level)
    : Monster(MONSTER_KIND_SPIDER, position, level)
{
    // モデルの共有ロード
    if (s_RefCount == 0) {
        s_pModel = ModelLoad("resource/model/sp.fbx", SPIDER_FIELD_SCALE, false);
    }
    s_RefCount++;

    // 初期状態はパトロール
    ChangeState(new StatePatrol(this));
}

//=============================================================================
// パトロール状態
//=============================================================================
MonsterSpider::StatePatrol::StatePatrol(MonsterSpider* pOwner)
    : m_pOwner(pOwner)
    , m_start_x(pOwner->m_position.x)
{
}

void MonsterSpider::StatePatrol::Update(double elapsed_time)
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

void MonsterSpider::StatePatrol::Draw() const
{
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.2f, 0.2f, 0.2f, 1.0f });

    if (s_pModel) {
        float s = m_pOwner->GetFieldScale();
        XMMATRIX scale = XMMatrixScaling(s, s, s);
        XMMATRIX rot = XMMatrixRotationX(XM_PIDIV2) * XMMatrixRotationY(-XM_PIDIV2);
        XMMATRIX trans = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y - 0.5f,
            m_pOwner->m_position.z);
        XMMATRIX world = scale * rot * trans;
        ModelDraw(s_pModel, world, { 0.1f, 0.1f, 0.1f, 1.0f });
    } else {
        extern int g_MonsterTexSpider;
        XMMATRIX world = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y,
            m_pOwner->m_position.z);
        Cube_Draw(g_MonsterTexSpider, world);
    }
}

//=============================================================================
// 追跡状態
//=============================================================================
MonsterSpider::StateChase::StateChase(MonsterSpider* pOwner)
    : m_pOwner(pOwner)
{
}

void MonsterSpider::StateChase::Update(double elapsed_time)
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

        // y座標を固定
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

void MonsterSpider::StateChase::Draw() const
{
    // 追跡中は少し明るく
    Light_SetSpecularWorld(PlayerCamera_GetPosition(), 2.0f, { 0.2f, 0.2f, 0.2f, 1.0f });

    if (s_pModel) {
        // プレイヤー方向を向く（m_frontからY回転角を計算）
        float s = m_pOwner->GetFieldScale();
        XMMATRIX scale = XMMatrixScaling(s, s, s);
        float yAngle = atan2f(m_pOwner->m_front.x, m_pOwner->m_front.z);
        XMMATRIX rot = XMMatrixRotationX(XM_PIDIV2) * XMMatrixRotationY(yAngle);
        XMMATRIX trans = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y - 0.5f,
            m_pOwner->m_position.z);
        XMMATRIX world = scale * rot * trans;
        ModelDraw(s_pModel, world, { 0.1f, 0.1f, 0.1f, 1.0f });
    } else {
        extern int g_MonsterTexSpider;
        XMMATRIX world = XMMatrixTranslation(
            m_pOwner->m_position.x,
            m_pOwner->m_position.y,
            m_pOwner->m_position.z);
        Cube_Draw(g_MonsterTexSpider, world);
    }
}
