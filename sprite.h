/*==============================================================================

   スプライト描画 [sprite.h]
														 Author : Youhei Sato
														 Date   : 2025/06/12
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SPRITE_H
#define SPRITE_H

#include <d3d11.h>
#include <DirectXMath.h>

void Sprite_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Sprite_Finalize(void);

void Sprite_Begin();

// テクスチャ全表示
void Sprite_Draw(int texid, float dx, float dy, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

// テクスチャ全表示（表示サイズ変更）
void Sprite_Draw(int texid, float dx, float dy, float dw, float dh, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

// UVカット
void Sprite_Draw(int texid, float dx, float dy, int px, int py, int pw, int ph, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

// UVカット（表示サイズ変更）
void Sprite_Draw(int texid, float dx, float dy, float dw, float dh, int px, int py, int pw, int ph, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

// UVカット（表示サイズ変更）回転
void Sprite_Draw(int texid, float dx, float dy, float dw, float dh, int px, int py, int pw, int ph, float angle, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

// テクスチャ全表示（テクスチャ指定がない、表示サイズ変更）
void Sprite_Draw(float dx, float dy, float dw, float dh, const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });


#endif // SPRITE_H
