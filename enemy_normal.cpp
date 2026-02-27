#include "enemy_normal.h"
#include "collision.h"
#include "player.h"
using namespace DirectX;
#include "cube.h"
#include "shader3d.h"
#include"light.h"

void EnemyNormal::EnemyNormalStatePatrol::Update(double elapsed_time)
{
	g_AccumulatedTime += elapsed_time;
	m_pOwner->m_Position.x = m_PointX + sinf(g_AccumulatedTime) * 5.0f;

	if (Collision_IsOverlapSphere(
		{ m_pOwner->m_Position, m_pOwner->m_DetectionRadius },
		Player_GetPosition())) {

		m_pOwner->ChangeState(new EnemyNormalStateChase(m_pOwner));
	}
}

void EnemyNormal::EnemyNormalStatePatrol::Draw() const
{
	Light_SetSpecularWorld(Player_GetPosition(), 4.0f, { 0.3f,0.25f,0.2f,1.0f });

	Cube_Draw(m_pOwner->m_TexWhiteId,
		XMMatrixTranslation(m_pOwner->m_Position.x, m_pOwner->m_Position.y, m_pOwner->m_Position.z));
}

void EnemyNormal::EnemyNormalStatePatrol::DepthDraw() const
{
	Cube_DepthDraw(
		XMMatrixTranslation(m_pOwner->m_Position.x, m_pOwner->m_Position.y, m_pOwner->m_Position.z));
}

void EnemyNormal::EnemyNormalStateChase::Update(double elapsed_time)
{
	// プレイヤーはどっちにいますかー
	XMVECTOR toPlayer = XMLoadFloat3(&Player_GetPosition()) - XMLoadFloat3(&m_pOwner->m_Position);
	toPlayer = XMVector3Normalize(toPlayer);

	// 歩くぜー
	XMVECTOR position = XMLoadFloat3(&m_pOwner->m_Position) + toPlayer * 3.0f * (float)elapsed_time;
	XMStoreFloat3(&m_pOwner->m_Position, position);

	// 諦める
	if (!Collision_IsOverlapSphere(
		{ m_pOwner->m_Position, m_pOwner->m_DetectionRadius },
		Player_GetPosition())) {

		g_AccumulatedTime += elapsed_time;

		if (g_AccumulatedTime >= 3.0) {
			m_pOwner->ChangeState(new EnemyNormalStatePatrol(m_pOwner));
		}
	}
	else {
		g_AccumulatedTime = 0.0;
	}
}

void EnemyNormal::EnemyNormalStateChase::Draw() const
{
	Cube_Draw(m_pOwner->m_TexRedId,
		XMMatrixTranslation(m_pOwner->m_Position.x, m_pOwner->m_Position.y, m_pOwner->m_Position.z));
}

void EnemyNormal::EnemyNormalStateChase::DepthDraw() const
{
	Cube_DepthDraw(
		XMMatrixTranslation(m_pOwner->m_Position.x, m_pOwner->m_Position.y, m_pOwner->m_Position.z));
}
