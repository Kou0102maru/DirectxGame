/*==============================================================================
    ÉâÉCÉgï˚å¸Ç©ÇÁÇÃÉJÉÅÉâêßå‰ [light_camera.h]
                                                           Author : Youhei Sato
                                                               Date : 2025/6/06
--------------------------------------------------------------------------------
==============================================================================*/
#ifndef LIGHT_CAMERA_H
#define LIGHT_CAMERA_H

#include <DirectXMath.h>

void LightCamera_Initialize(const DirectX::XMFLOAT3& world_directional, const DirectX::XMFLOAT3& position);
void LightCamera_Finalize();


void LightCamera_SetPosition(const DirectX::XMFLOAT3& position);
void LightCamera_SetFront(const DirectX::XMFLOAT3& front);

const DirectX::XMFLOAT4X4& LightCamera_GetViewMatrix();
const DirectX::XMFLOAT4X4& LightCamera_GetProjectionMatrix();


#endif // LIGHT_CAMERA_H
