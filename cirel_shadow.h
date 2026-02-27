/*==============================================================================

   ŠÛ‰e‚Ì•\Ž¦ [circle_shadow.h]
														 Author : Youhei Sato
														 Date   : 2025/05/12
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef CIRCLE_SHADOW_H
#define CIRCLE_SHADOW_H

#include <d3d11.h>
#include<DirectXMath.h>

void CircleShadow_Initialize();
void CircleShadow_Finalize();
void CircleShadow_Draw(const DirectX::XMFLOAT3& position);

void CircleShadow_SetMinimapMode(bool is_minimap);
#endif // CIRCLE_SHADOW_H
