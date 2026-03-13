/*==============================================================================

   テクスチャ管理 [texture.h]
														 Author : Youhei Sato
														 Date   : 2025/06/13
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef TEXTURE_H
#define TEXTURE_H


#include <d3d11.h>

void Texture_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Texture_Finalize(void);

// テクスチャ画像の読み込み
//
// 戻り値 : 管理番号。読み込めなかった場合-1。
//
int Texture_Load(const wchar_t* pFilename);

void Texture_AllRelease();

// ピクセルデータからテクスチャを生成して登録
// pPixels : RGBA 32bit ピクセルデータ
// 戻り値 : 管理番号。失敗時 -1。
int Texture_CreateFromMemory(const void* pPixels, unsigned int width, unsigned int height, const wchar_t* tag);

// 管理番号を指定してテクスチャを個別解放
void Texture_Release(int texid);

void Texture_SetTexture(int texid, int slot = 0);
unsigned int Texture_Width(int texid);
unsigned int Texture_Height(int texid);


#endif // TEXTURE_H
