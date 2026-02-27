/*==============================================================================

   ’eŠÛ‚ÌŠÇ— [bullet.cpp]
														 Author : Youhei Sato
														 Date   : 2025/11/12
--------------------------------------------------------------------------------

==============================================================================*/
#include "bullet.h"
using namespace DirectX;
#include "model.h"
#include "trajectory3d.h"


class Bullet
{
private:
	XMFLOAT3 m_position{};
	XMFLOAT3 m_velocity{};
	double m_accumulated_time{ 0.0 };
	static constexpr double TIME_LIMIT = 3.0;

public:
	Bullet(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& velocity)
		: m_position(position), m_velocity(velocity) 
	{
	}

	void Update(double elapsed_time) {
		m_accumulated_time += elapsed_time;
		XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) + XMLoadFloat3(&m_velocity) * (float)elapsed_time);
		Trajectory3d_Create(m_position, { 0.8f, 0.4f, 0.4f, 1.0f }, 1.0f, 1.0);
	}

	const XMFLOAT3& GetPosition() const {
		return m_position;
	}

	XMFLOAT3 GetFront() const {
		XMFLOAT3 front;
		XMStoreFloat3(&front, XMVector3Normalize(XMLoadFloat3(&m_velocity)));
		return front;
	}

	bool IsDestroy() const {
		return m_accumulated_time >= TIME_LIMIT;
	}
};

static constexpr int MAX_BULLET = 1024;
static Bullet* g_pBullets[MAX_BULLET]{};
static int g_BulletsCount{ 0 };
static MODEL* g_pBulletModel{ nullptr };


void Bullet_Initialize()
{
	g_pBulletModel = ModelLoad("resource/model/test.fbx", 0.05f);
	
	for (int i = 0; i < g_BulletsCount; i++) {
		delete g_pBullets[i];
	}

	g_BulletsCount = 0;
}

void Bullet_Finalize()
{
	ModelRelease(g_pBulletModel);

	for (int i = 0; i < g_BulletsCount; i++) {
		delete g_pBullets[i];
	}

	g_BulletsCount = 0;
}

void Bullet_Update(double elapsed_time)
{

	for (int i = 0; i < g_BulletsCount; i++) {
		g_pBullets[i]->Update(elapsed_time);
	}

	for (int i = 0; i < g_BulletsCount; i++) {

		if (g_pBullets[i]->IsDestroy()) {
			Bullet_Destroy(i);
		}
	}
}

void Bullet_Draw()
{
	// 

	XMMATRIX mtxWorld;

	for (int i = 0; i < g_BulletsCount; i++) {
		XMFLOAT3 front = g_pBullets[i]->GetFront();
		XMVECTOR axis = XMVector3Cross(XMLoadFloat3(&front), { 1.0f, 0.0f, 0.0f });
		float angle = 0.0f;
		XMMATRIX r = XMMatrixIdentity();
		if (XMVectorGetX(XMVector3LengthSq(axis)) > 0.0f) {
			angle = acosf(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&front), { 1.0f, 0.0f, 0.0f })));
			r = XMMatrixRotationAxis(axis, -angle + 270);
			// r = XMMatrixRotationY(angle);
		}
		XMVECTOR position = XMLoadFloat3(&g_pBullets[i]->GetPosition());
		mtxWorld = r * XMMatrixTranslationFromVector(position);
		ModelDraw(g_pBulletModel, mtxWorld);
	}
}

void Bullet_Create(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& velocity)
{
	if (g_BulletsCount >= MAX_BULLET) {
		return;
	}

	g_pBullets[g_BulletsCount++] = new Bullet(position, velocity);
}

void Bullet_Destroy(int index)
{
	delete g_pBullets[index];
	g_pBullets[index] = g_pBullets[g_BulletsCount - 1];
	g_BulletsCount--;
}

int Bullet_GetBulletsCount()
{
	return g_BulletsCount;
}

AABB Bullet_GetAABB(int index)
{
	return Model_GetAABB(g_pBulletModel, g_pBullets[index]->GetPosition());
}

Sphere Bullet_GetSphere(int index)
{
	return { g_pBullets[index]->GetPosition(), g_pBulletModel->local_aabb.GetHalf().x };
}

const DirectX::XMFLOAT3& Bullet_GetPosition(int index)
{
	return g_pBullets[index]->GetPosition();
}
