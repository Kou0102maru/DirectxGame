/*==============================================================================

   プレイヤー制御 [player.cpp]
														 Author : Youhei Sato
														 Date   : 2025/10/31
--------------------------------------------------------------------------------

==============================================================================*/
#include "player.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "model.h"
#include "key_logger.h"
#include "pad_logger.h"
#include "light.h"
#include "player_camera.h"
#include "cube.h"
#include "map.h"
#include "bullet.h"
#include"cirel_shadow.h"


static XMFLOAT3 g_PlayerPosition{};
static XMFLOAT3 g_PlayerFront{ 0.0f, 0.0f, 1.0f };
static XMFLOAT3 g_PlayerVelocity{};
static MODEL* g_pPlayerModel{ nullptr };
static bool g_IsJump = false;
static constexpr double SHOT_INTERVAL = 0.25;
static double g_Rapid_Time = 0.0;


void Player_Initialize(const XMFLOAT3& position, const XMFLOAT3& front)
{
	g_PlayerPosition = position;
	g_PlayerVelocity = { 0.0f, 0.0f, 0.0f };
	XMStoreFloat3(&g_PlayerFront, XMVector3Normalize(XMLoadFloat3(&front)));
	g_IsJump = false;

	g_pPlayerModel = ModelLoad("resource/model/slime.fbx", 0.8f);

	
}

void Player_Finalize()
{
	ModelRelease(g_pPlayerModel);
}

void Player_Update(double elapsed_time)
{
	// 前のフレームで保存したプレイヤーの座標と移動エネルギーを演算できる形にする
	XMVECTOR position = XMLoadFloat3(&g_PlayerPosition);
	XMVECTOR velocity = XMLoadFloat3(&g_PlayerVelocity);

	// ジャンプ処理
	if ((KeyLogger_IsTrigger(KK_J) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_A)) && !g_IsJump) {
		velocity += { 0.0f, 30.0f, 0.0f };
		g_IsJump = true;
	}

	// 重力にひかれて落っこちる
	XMVECTOR gdir{ 0.0f, 1.0f, 0.0f };
	velocity += gdir * -9.8f * 10.0f * (float)elapsed_time;
	position += velocity * (float)elapsed_time;
	
	// 重力に引かれたあとのプレイヤーとマップオブジェクトとの当たり判定
	for (int i = 0; i < Map_GetObjectsCount(); i++) {

		AABB player = Player_ConvertPositionToAABB(position);

		// ここがオブジェクトによって変わるはず
		AABB object = Map_GetObject(i)->Aabb; 
		Hit hit = Collision_IsHitAABB(object, player);

		if (hit.isHit) {
			if (hit.noraml.y > 0.0f) { // 上にたぶんのっかった
				position = XMVectorSetY(position, object.max.y);
				velocity *= { 1.0f, 0.0f, 1.0f };
				g_IsJump = false;
			}
		}
	}

	XMVECTOR direction{};
	XMVECTOR front = XMLoadFloat3(&PlayerCamera_GetFront()) * XMVECTOR { 1.0f, 0.0f, 1.0f };

	if (KeyLogger_IsPressed(KK_W)) {
		direction += front;
	}

	if (KeyLogger_IsPressed(KK_S)) {
		direction += -front;
	}

	if (KeyLogger_IsPressed(KK_D)) {
		direction += XMVector3Cross({ 0.0f, 1.0f, 0.0f }, front);
	}

	if (KeyLogger_IsPressed(KK_A)) {
		direction -= XMVector3Cross({ 0.0f, 1.0f, 0.0f }, front);
	}

	XMFLOAT2 pad_left_Thumb = PadLogger_GetLeftThumbStick(0);

	if (XMVectorGetX(XMVector3LengthSq(XMVECTOR{ pad_left_Thumb.x, 0.0f, pad_left_Thumb.y })) > 0.0f) {
		direction += XMVECTOR{ pad_left_Thumb.x, 0.0f, pad_left_Thumb.y } + XMLoadFloat3(&g_PlayerFront);
	}

	if (XMVectorGetX(XMVector3LengthSq(direction)) > 0.0f) {
		direction = XMVector3Normalize(direction);
		
		// ２つのベクトルのなす角は
		float dot = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&g_PlayerFront), direction));
		float angle = acosf(dot);

		// 回転速度
		const float ROTATION_SPEED = XM_2PI * 2.0f * (float)elapsed_time;

		if (angle < ROTATION_SPEED) {
			front = direction;
		}
		else {
			// 向きたい方向が右回りか左回りか
			XMMATRIX r = XMMatrixIdentity();

			if (XMVectorGetY(XMVector3Cross(XMLoadFloat3(&g_PlayerFront), direction)) < 0.0f) {
				r = XMMatrixRotationY(-ROTATION_SPEED);
			}
			else {
				r = XMMatrixRotationY(ROTATION_SPEED);
			}

			front = XMVector3TransformNormal(XMLoadFloat3(&g_PlayerFront), r);
		}

		velocity += front * (float)(2000.0 / 50.0 * elapsed_time);
		XMStoreFloat3(&g_PlayerFront, front);
	}

	velocity += -velocity * (float)(10.0 * elapsed_time); // 抵抗
	position += velocity * (float)elapsed_time;

	// 移動したあとのプレイヤーとマップオブジェクトとの当たり判定
	for (int i = 0; i < Map_GetObjectsCount(); i++) {

		AABB player = Player_ConvertPositionToAABB(position);

		// ここがオブジェクトによって変わるはず
		AABB object = Map_GetObject(i)->Aabb;
		Hit hit = Collision_IsHitAABB(object, player);

		if (hit.isHit) {
			if (hit.noraml.x > 0.0f) {
				position = XMVectorSetX(position, object.max.x + g_pPlayerModel->local_aabb.GetHalf().x);
				velocity *= { 0.0f, 1.0f, 1.0f };
			}
			else if (hit.noraml.x < 0.0f) {
				position = XMVectorSetX(position, object.min.x - g_pPlayerModel->local_aabb.GetHalf().x);
				velocity *= { 0.0f, 1.0f, 1.0f };
			}
			else if (hit.noraml.y < 0.0f) {
				position = XMVectorSetY(position, object.min.y - g_pPlayerModel->local_aabb.GetHalf().y * 2.0f);
				velocity *= { 1.0f, 0.0f, 1.0f };
			}
			else if (hit.noraml.z > 0.0f) {
				position = XMVectorSetZ(position, object.max.z + g_pPlayerModel->local_aabb.GetHalf().z);
				velocity *= { 1.0f, 1.0f, 0.0f };
			}
			else if (hit.noraml.z < 0.0f) {
				position = XMVectorSetZ(position, object.min.z - g_pPlayerModel->local_aabb.GetHalf().z);
				velocity *= { 1.0f, 1.0f, 0.0f };
			}
		}
	}

	XMStoreFloat3(&g_PlayerPosition, position);
	XMStoreFloat3(&g_PlayerVelocity, velocity);

	if (KeyLogger_IsPressed(KK_SPACE) || PadLogger_GetRightTrigger(0) > 0.8f) {
		if (g_Rapid_Time <= 0.0) {
			XMFLOAT3 shot_position = g_PlayerPosition;
			shot_position.y += 1.0f; // 一応右手のあたりから
			shot_position.x += 1.0f;
			XMFLOAT3 shot_velocity;
			XMStoreFloat3(&shot_velocity, XMLoadFloat3(&g_PlayerFront) * 20.0f);
			Bullet_Create(shot_position, shot_velocity);
			g_Rapid_Time = SHOT_INTERVAL;
		}
		else {
			g_Rapid_Time -= elapsed_time;
		}
	}
	else {
		g_Rapid_Time = 0.0;
	}

}

void Player_Draw()
{
	Light_SetSpecularWorld(PlayerCamera_GetPosition(), 4.0f, { 0.3f, 0.25f, 0.2f, 1.0f });

	float angle = -atan2f(g_PlayerFront.z, g_PlayerFront.x) + XMConvertToRadians(270);

	XMMATRIX r = XMMatrixRotationY(angle);
	XMMATRIX t = XMMatrixTranslation(g_PlayerPosition.x, g_PlayerPosition.y + 0.65f, g_PlayerPosition.z);
	XMMATRIX world = r * t;
	ModelDraw(g_pPlayerModel, world);

	CircleShadow_Draw(g_PlayerPosition);
}

void Player_DepthDraw()
{
	Light_SetSpecularWorld(PlayerCamera_GetPosition(), 4.0f, { 0.3f, 0.25f, 0.2f, 1.0f });

	float angle = -atan2f(g_PlayerFront.z, g_PlayerFront.x) + XMConvertToRadians(270);

	XMMATRIX r = XMMatrixRotationY(angle);
	XMMATRIX t = XMMatrixTranslation(g_PlayerPosition.x, g_PlayerPosition.y + 1.0f, g_PlayerPosition.z);
	XMMATRIX world = r * t;
	ModelDepthDraw(g_pPlayerModel, world);
}

const DirectX::XMFLOAT3& Player_GetPosition()
{
	return g_PlayerPosition;
}

const DirectX::XMFLOAT3& Player_GetFront()
{
	return g_PlayerFront;
}

AABB Player_GetAABB()
{
	XMFLOAT3 half = g_pPlayerModel->local_aabb.GetHalf();

	return {
		{ g_PlayerPosition.x - half.x, g_PlayerPosition.y,                 g_PlayerPosition.z - half.z },
		{ g_PlayerPosition.x + half.x, g_PlayerPosition.y + half.y * 2.0f, g_PlayerPosition.z + half.z }
	};
}

AABB Player_ConvertPositionToAABB(const DirectX::XMVECTOR& position)
{
	AABB aabb;
	XMFLOAT3 half = g_pPlayerModel->local_aabb.GetHalf();
	half.y = 0.0f;
	XMStoreFloat3(&aabb.min, position - XMLoadFloat3(&half));
	half.y = g_pPlayerModel->local_aabb.GetHalf().y * 2.0f;
	XMStoreFloat3(&aabb.max, position + XMLoadFloat3(&half));
	return aabb;
}

void Player_SetPosition(const DirectX::XMFLOAT3& position)
{
	g_PlayerPosition = position;
}