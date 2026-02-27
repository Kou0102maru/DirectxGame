/*==============================================================================
    ÉâÉCÉgï˚å¸Ç©ÇÁÇÃÉJÉÅÉâêßå‰ [light_camera.cpp]
                                                           Author : Youhei Sato
                                                               Date : 2025/6/06
--------------------------------------------------------------------------------
==============================================================================*/
#include "light_camera.h"
using namespace DirectX;

static XMFLOAT3 g_Position{};
static XMFLOAT3 g_Front{ 0.0f,1.0f,0.0f };


void LightCamera_Initialize(const XMFLOAT3& world_directional, const XMFLOAT3& position)
{
    g_Front = world_directional;
    g_Position = position;
}

void LightCamera_Finalize()
{
}

void LightCamera_SetPosition(const DirectX::XMFLOAT3& position)
{
    g_Position = position;
}

void LightCamera_SetFront(const DirectX::XMFLOAT3& front)
{
    g_Front = front;
}

const DirectX::XMFLOAT4X4& LightCamera_GetViewMatrix()
{
    XMFLOAT4X4 mtxView;

    XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&g_Position), XMVECTOR{ 0.0f,-1.0f,0.0f }, XMLoadFloat3(&g_Front));

    XMStoreFloat4x4(&mtxView, view);

    return mtxView;
}

const DirectX::XMFLOAT4X4& LightCamera_GetProjectionMatrix()
{
    XMFLOAT4X4 mtxProj;

    float value = 40;

    XMMATRIX proj = XMMatrixOrthographicOffCenterLH(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 1000.0f);

    XMStoreFloat4x4(&mtxProj, proj);

    return mtxProj;
}
