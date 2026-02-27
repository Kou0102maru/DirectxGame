#include "mapcamera.h"
using namespace DirectX;

static XMFLOAT3 g_Position{};
static XMFLOAT3 g_Front{ 0.0f,1.0f,0.0f };

void MapCamera_Initialize()
{
}

void MapCamera_Finalize()
{
}

void MapCamera_SetPosition(const XMFLOAT3& position)
{
	g_Position = position;
}

void MapCamera_SetFront(const XMFLOAT3& front)
{
	g_Front = front;
}

const DirectX::XMFLOAT4X4& MapCamera_GetViewMatrix()
{
	XMFLOAT4X4 mtxView;

	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&g_Position), XMVECTOR{ 0.0f,-1.0f,0.0f }, XMLoadFloat3(&g_Front));
	
	XMStoreFloat4x4(&mtxView, view);

	return mtxView;
}

const DirectX::XMFLOAT4X4& MapCamera_GetProjectionMatrix()
{
	XMFLOAT4X4 mtxProj;

	XMMATRIX proj = XMMatrixOrthographicOffCenterLH(-25.0f, 25.0f, -25.0f, 25.0f, 0.1f, 1000.0f);

	XMStoreFloat4x4(&mtxProj, proj);

	return mtxProj;
}

