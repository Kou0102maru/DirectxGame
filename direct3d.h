/*==============================================================================

   Direct3Dの初期化関連 [direct3d.h]
														 Author : Youhei Sato
														 Date   : 2025/05/12
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef DIRECT3D_H
#define DIRECT3D_H


#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>


// セーフリリースマクロ
#define SAFE_RELEASE(o) if (o) { (o)->Release(); o = NULL; }


bool Direct3D_Initialize(HWND hWnd); // Direct3Dの初期化
void Direct3D_Finalize(); // Direct3Dの終了処理
void Direct3D_Present(); // バックバッファの表示

// バックバッファの大きさの取得
unsigned int Direct3D_GetBackBufferWidth(); // 幅
unsigned int Direct3D_GetBackBufferHeight(); // 高さ

// Direct3Dデバイスの取得
ID3D11Device* Direct3D_GetDevice();

// Direct3Dデバイスコンテキストの取得
ID3D11DeviceContext* Direct3D_GetContext();

// αブレンド設定関数
void Direct3D_SetAlphaBlendTransparent(); // 透過処理
void Direct3D_SetAlphaBlendAdd();         // 加算合成

// 深度バッファの設定
void Direct3D_SetDepthEnable(bool enable);
void Direct3D_SetDepthWriteDisable();

// ビューポート行列の作成
DirectX::XMMATRIX Direct3D_MatrixViewport();

// スクリーン座標 ⇒ 3D座標変換
DirectX::XMFLOAT3 Direct3D_ScreenToWorld(int x, int y, float depth, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

// 3D座標変換 ⇒ スクリーン座標
DirectX::XMFLOAT2 Direct3D_WorldToScreen(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4X4 & view, const DirectX::XMFLOAT4X4 & projection);

// バックバッファのクリア
void Direct3D_ClearBackBuffer(); 

//バッファのレンダリングに切り替える
void Direct3D_SetBackBuffer();

//オフスクリーンバッファのクリア
void Direct3D_ClearOffscreen();

//オフスクリーンテクスチャへのレンダリングに切り替える
void Direct3D_SetOffscreen();

//オフスクリーンレンダリングテクスチャの設定
void Direct3D_SetOffscreenTexture(int slot);

//深度情報バッファのクリア
void Direct3D_ClearDepth();

//深度情報バッファへのレンダリングに切り替える
void Direct3D_SetDepth();

//深度情報バッファレンダリングテクスチャの設定
void Direct3D_SetDepthTexture(int slot);

#endif // DIRECT3D_H
