/*==============================================================================

   ?v???C???[???? [player.cpp]
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
#include "party.h"
#include "monster.h"


static XMFLOAT3 g_PlayerPosition{};
static XMFLOAT3 g_PlayerFront{ 0.0f, 0.0f, 1.0f };
static XMFLOAT3 g_PlayerVelocity{};
static MODEL* g_pPlayerModel{ nullptr };

// ?t?B?[???h?p?p?[?e?B?????X?^?[???f??
static MODEL* g_pFieldSpiderModel  = nullptr;
static MODEL* g_pFieldWolfModel    = nullptr;
static MODEL* g_pFieldDragonModel  = nullptr;
static MODEL* g_pFieldRobotModel   = nullptr;
static MODEL* g_pFieldEyeballModel = nullptr;
static bool g_IsJump = false;
static constexpr double SHOT_INTERVAL = 0.25;
static double g_Rapid_Time = 0.0;

static int g_PlayerHp = 100;
static int g_PlayerHpMax = 100;
static int g_PlayerLevel = 1;
static int g_PlayerAtk = 10;
static int g_PlayerDef = 5;
static int g_PlayerExp = 0;
static int g_PlayerExpNext = 10;

static void Player_LevelUp()
{
	g_PlayerLevel++;
	g_PlayerHpMax += 10;
	g_PlayerAtk += 2;
	g_PlayerDef += 1;
	g_PlayerExpNext = g_PlayerLevel * g_PlayerLevel * 10;
	g_PlayerHp = g_PlayerHpMax;
}


void Player_Initialize(const XMFLOAT3& position, const XMFLOAT3& front)
{
	g_PlayerPosition = position;
	g_PlayerVelocity = { 0.0f, 0.0f, 0.0f };
	XMStoreFloat3(&g_PlayerFront, XMVector3Normalize(XMLoadFloat3(&front)));
	g_IsJump = false;
	g_PlayerHp = g_PlayerHpMax;

	g_pPlayerModel = ModelLoad("resource/model/slime.fbx", 0.8f);

	// ?p?[?e?B?????X?^?[?p?t?B?[???h???f??????[?h
	if (g_pFieldSpiderModel)  { ModelRelease(g_pFieldSpiderModel);  g_pFieldSpiderModel  = nullptr; }
	if (g_pFieldWolfModel)    { ModelRelease(g_pFieldWolfModel);    g_pFieldWolfModel    = nullptr; }
	if (g_pFieldDragonModel)  { ModelRelease(g_pFieldDragonModel);  g_pFieldDragonModel  = nullptr; }
	if (g_pFieldRobotModel)   { ModelRelease(g_pFieldRobotModel);   g_pFieldRobotModel   = nullptr; }
	if (g_pFieldEyeballModel) { ModelRelease(g_pFieldEyeballModel); g_pFieldEyeballModel = nullptr; }
	g_pFieldSpiderModel  = ModelLoad("resource/model/sp.fbx", 0.15f, false);
	g_pFieldWolfModel    = ModelLoad("resource/model/Wolf.fbx", 3.0f, true);
	g_pFieldDragonModel  = ModelLoad("resource/model/Dragon.fbx", 0.2f, true);
	g_pFieldRobotModel   = ModelLoad("resource/model/robot.fbx", 0.3f, true);
	g_pFieldEyeballModel = ModelLoad("resource/model/eyeball.fbx", 0.3f, true);
}

void Player_Finalize()
{
	ModelRelease(g_pPlayerModel);
	if (g_pFieldSpiderModel)  { ModelRelease(g_pFieldSpiderModel);  g_pFieldSpiderModel  = nullptr; }
	if (g_pFieldWolfModel)    { ModelRelease(g_pFieldWolfModel);    g_pFieldWolfModel    = nullptr; }
	if (g_pFieldDragonModel)  { ModelRelease(g_pFieldDragonModel);  g_pFieldDragonModel  = nullptr; }
	if (g_pFieldRobotModel)   { ModelRelease(g_pFieldRobotModel);   g_pFieldRobotModel   = nullptr; }
	if (g_pFieldEyeballModel) { ModelRelease(g_pFieldEyeballModel); g_pFieldEyeballModel = nullptr; }
}

void Player_Update(double elapsed_time)
{
	// ?O?t???[??????x????v???C???[???W??G?l???M?[??v?Z
	XMVECTOR position = XMLoadFloat3(&g_PlayerPosition);
	XMVECTOR velocity = XMLoadFloat3(&g_PlayerVelocity);

	// ?W?????v????
	if ((KeyLogger_IsTrigger(KK_J) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_A)) && !g_IsJump) {
		velocity += { 0.0f, 30.0f, 0.0f };
		g_IsJump = true;
	}

	// ?d??K?p
	XMVECTOR gdir{ 0.0f, 1.0f, 0.0f };
	velocity += gdir * -9.8f * 10.0f * (float)elapsed_time;
	position += velocity * (float)elapsed_time;

	// ?d??K?p??v???C???[??}?b?v?I?u?W?F?N?g???????
	for (int i = 0; i < Map_GetObjectsCount(); i++) {

		AABB player = Player_ConvertPositionToAABB(position);

		// ?e?I?u?W?F?N?g???????
		AABB object = Map_GetObject(i)->Aabb;
		Hit hit = Collision_IsHitAABB(object, player);

		if (hit.isHit) {
			if (hit.noraml.y > 0.0f) { // ??????n
				position = XMVectorSetY(position, object.max.y);
				velocity *= { 1.0f, 0.0f, 1.0f };
				g_IsJump = false;
			}
		}
	}

	// ???b?V???t?B?[???h?n?????n????iY=0???n??j
	if (XMVectorGetY(position) < 0.0f) {
		position = XMVectorSetY(position, 0.0f);
		velocity *= { 1.0f, 0.0f, 1.0f };
		g_IsJump = false;
	}

	XMVECTOR direction{};
	// ?J?????O?????iXZ???????e?{???K???j
	XMVECTOR camFront = XMLoadFloat3(&PlayerCamera_GetFront()) * XMVECTOR{ 1.0f, 0.0f, 1.0f };
	camFront = XMVector3Normalize(camFront);
	XMVECTOR camRight = XMVector3Cross({ 0.0f, 1.0f, 0.0f }, camFront);

	// WASD?i?J?????õT?j
	if (KeyLogger_IsPressed(KK_W)) direction += camFront;
	if (KeyLogger_IsPressed(KK_S)) direction -= camFront;
	if (KeyLogger_IsPressed(KK_D)) direction += camRight;
	if (KeyLogger_IsPressed(KK_A)) direction -= camRight;

	// ???X?e?B?b?N?i?J?????õT?j
	XMFLOAT2 pad_left_Thumb = PadLogger_GetLeftThumbStick(0);
	if (pad_left_Thumb.x != 0.0f || pad_left_Thumb.y != 0.0f) {
		direction += camFront * pad_left_Thumb.y + camRight * pad_left_Thumb.x;
	}

	if (XMVectorGetX(XMVector3LengthSq(direction)) > 0.0f) {
		direction = XMVector3Normalize(direction);

		// 2?x?N?g?????p?x
		float dot = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&g_PlayerFront), direction));
		float angle = acosf(dot);

		// ??]???x
		const float ROTATION_SPEED = XM_2PI * 2.0f * (float)elapsed_time;

		XMVECTOR newFront;
		if (angle < ROTATION_SPEED) {
			newFront = direction;
		}
		else {
			// ???E???????]????????
			XMMATRIX r = XMMatrixIdentity();

			if (XMVectorGetY(XMVector3Cross(XMLoadFloat3(&g_PlayerFront), direction)) < 0.0f) {
				r = XMMatrixRotationY(-ROTATION_SPEED);
			}
			else {
				r = XMMatrixRotationY(ROTATION_SPEED);
			}

			newFront = XMVector3TransformNormal(XMLoadFloat3(&g_PlayerFront), r);
		}

		velocity += newFront * (float)(2000.0 / 50.0 * elapsed_time);
		XMStoreFloat3(&g_PlayerFront, newFront);
	}

	velocity += -velocity * (float)(10.0 * elapsed_time); // ???C
	position += velocity * (float)elapsed_time;

	// ?????v???C???[??}?b?v?I?u?W?F?N?g???????
	for (int i = 0; i < Map_GetObjectsCount(); i++) {

		AABB player = Player_ConvertPositionToAABB(position);

		// ?e?I?u?W?F?N?g???????
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

	/* ?t?B?[???h?e????????
	if (KeyLogger_IsPressed(KK_SPACE) || PadLogger_GetRightTrigger(0) > 0.8f) {
		if (g_Rapid_Time <= 0.0) {
			XMFLOAT3 shot_position = g_PlayerPosition;
			shot_position.y += 1.0f;
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
	*/

}

void Player_Draw()
{
	Light_SetSpecularWorld(PlayerCamera_GetPosition(), 4.0f, { 0.3f, 0.25f, 0.2f, 1.0f });

	float angle = -atan2f(g_PlayerFront.z, g_PlayerFront.x) + XMConvertToRadians(270);

	MonsterKind fighterKind = Party_GetFighterKind();

	if (fighterKind == MONSTER_KIND_MAX) {
		// ?v???C???[?{?l?F?X???C?????f??
		XMMATRIX r = XMMatrixRotationY(angle);
		XMMATRIX t = XMMatrixTranslation(g_PlayerPosition.x, g_PlayerPosition.y + 0.65f, g_PlayerPosition.z);
		ModelDraw(g_pPlayerModel, r * t);
	} else {
		// ?p?[?e?B?????X?^?[????f????\??
		switch (fighterKind) {
		case MONSTER_KIND_SPIDER:
		{
			XMMATRIX baseRot = XMMatrixRotationX(XM_PIDIV2);
			XMMATRIX faceRot = XMMatrixRotationY(angle + XM_PI);
			XMMATRIX t = XMMatrixTranslation(g_PlayerPosition.x, g_PlayerPosition.y + 0.5f, g_PlayerPosition.z);
			if (g_pFieldSpiderModel) ModelDraw(g_pFieldSpiderModel, baseRot * faceRot * t, { 0.1f, 0.1f, 0.1f, 1.0f });
			break;
		}
		case MONSTER_KIND_WOLF:
		{
			XMMATRIX r = XMMatrixRotationY(angle);
			XMMATRIX t = XMMatrixTranslation(g_PlayerPosition.x, g_PlayerPosition.y + 0.0f, g_PlayerPosition.z);
			if (g_pFieldWolfModel) ModelDraw(g_pFieldWolfModel, r * t);
			break;
		}
		case MONSTER_KIND_DRAGON:
		{
			XMMATRIX r = XMMatrixRotationY(angle + XM_PIDIV2);
			XMMATRIX t = XMMatrixTranslation(g_PlayerPosition.x, g_PlayerPosition.y - 1.0f, g_PlayerPosition.z);
			if (g_pFieldDragonModel) ModelDraw(g_pFieldDragonModel, r * t, { 0.0f, 0.0f, 1.0f, 1.0f });
			break;
		}
		case MONSTER_KIND_ROBOT:
		{
			XMMATRIX r = XMMatrixRotationY(angle + XM_PIDIV2);
			XMMATRIX t = XMMatrixTranslation(g_PlayerPosition.x, g_PlayerPosition.y + 1.5f, g_PlayerPosition.z);
			if (g_pFieldRobotModel) ModelDraw(g_pFieldRobotModel, r * t, { 0.4f, 0.4f, 0.5f, 1.0f });
			break;
		}
		case MONSTER_KIND_EYEBALL:
		{
			XMMATRIX baseRot = XMMatrixRotationZ(-XM_PIDIV2);
			XMMATRIX faceRot = XMMatrixRotationY(angle + XM_PIDIV2);
			XMMATRIX t = XMMatrixTranslation(g_PlayerPosition.x, g_PlayerPosition.y + 1.0f, g_PlayerPosition.z);
			if (g_pFieldEyeballModel) ModelDraw(g_pFieldEyeballModel, baseRot * faceRot * t, { 0.8f, 0.1f, 0.1f, 1.0f });
			break;
		}
		default:
		{
			// ?t?H?[???o?b?N?F?X???C?????f??
			XMMATRIX r = XMMatrixRotationY(angle);
			XMMATRIX t = XMMatrixTranslation(g_PlayerPosition.x, g_PlayerPosition.y + 0.65f, g_PlayerPosition.z);
			ModelDraw(g_pPlayerModel, r * t);
			break;
		}
		}
	}

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

int Player_GetHp()
{
	return g_PlayerHp;
}

int Player_GetHpMax()
{
	return g_PlayerHpMax;
}

void Player_TakeDamage(int damage)
{
	g_PlayerHp -= damage;
	if (g_PlayerHp < 0) g_PlayerHp = 0;
}

int Player_GetLevel()
{
	return g_PlayerLevel;
}

int Player_GetAtk()
{
	return g_PlayerAtk;
}

int Player_GetDef()
{
	return g_PlayerDef;
}

int Player_GetExp()
{
	return g_PlayerExp;
}

int Player_GetExpNext()
{
	return g_PlayerExpNext;
}

void Player_GainExp(int exp)
{
	g_PlayerExp += exp;
	while (g_PlayerExp >= g_PlayerExpNext) {
		g_PlayerExp = 0;  // ???x???A?b?v??????o???l??0????Z?b?g
		Player_LevelUp();
	}
}
