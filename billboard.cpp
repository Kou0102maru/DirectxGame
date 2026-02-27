/*==============================================================================

   ビルボード描画 [billboard.cpp]
														 Author : Youhei Sato
														 Date   : 2025/11/14
--------------------------------------------------------------------------------

==============================================================================*/
#include "billboard.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader_billboard.h"
#include "texture.h"
#include "player_camera.h"


static constexpr int NUM_VERTEX = 4; // 頂点数

static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ

static XMFLOAT4X4 g_mtxView{}; // ビュー行列の平行移動成分をカットした行列


// 3D頂点構造体
struct Vertex3d
{
	XMFLOAT3 position; // 座標
	XMFLOAT4 color;    // 色
	XMFLOAT2 texcoord; // UV
};


void Billboard_Initialize()
{
	ShaderBillBoard_Initialize();

	Vertex3d vertex[]{
		{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = vertex;

	Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &g_pVertexBuffer);
}


void Billboard_Finalize()
{
	SAFE_RELEASE(g_pVertexBuffer);

	ShaderBillBoard_Finalize();
}

void Billboard_SetViewMatrix(const DirectX::XMFLOAT4X4& view)
{
	// カメラ行列の平行移動成分をカット
	g_mtxView = view;
	g_mtxView._41 = g_mtxView._42 = g_mtxView._43 = 0.0f;
}


// ピボットオフセット…スケールされたポリゴンの中心からのオフセットで指定する回転（ポリゴンのローカル中心）座標
void Billboard_Draw(int texId, const DirectX::XMFLOAT3& position, const XMFLOAT2& scale, const DirectX::XMFLOAT4& color, const XMFLOAT2& pivot)
{
	ShaderBillBoard_Begin();

	ShaderBillBoard_SetUVParameter({ {1.0f, 1.0f}, {0.0f, 0.0f} });

	// ピクセルシェーダーに色を設定
	ShaderBillBoard_SetColor(color);

	// テクスチャの設定
	Texture_SetTexture(texId);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	Direct3D_GetContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 頂点シェーダーにワールド座標変換行列を設定

	// カメラ行列の回転だけ逆行列を作る
	// XMMATRIX iv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&g_mtxView));
	// 直交行列の逆行列は転置行列に等しい
	XMMATRIX iv = XMMatrixTranspose(XMLoadFloat4x4(&g_mtxView));

	// 回転軸までのオフセット行列
	XMMATRIX pivot_offset = XMMatrixTranslation(-pivot.x, -pivot.y, 0.0f);

	XMMATRIX s = XMMatrixScaling(scale.x, scale.y, 1.0f);
	XMMATRIX t = XMMatrixTranslation(position.x, position.y, position.z);
	ShaderBillBoard_SetWorldMatrix(s * pivot_offset * iv * t);

	// ポリゴン描画命令発行
	Direct3D_GetContext()->Draw(NUM_VERTEX, 0);
}


void Billboard_Draw(int texId, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& scale, const DirectX::XMUINT4& tex_cut, const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT2& pivot)
{
	ShaderBillBoard_Begin();

	float uv_x = (float)tex_cut.x / Texture_Width(texId);
	float uv_y = (float)tex_cut.y / Texture_Height(texId);
	float uv_w = (float)tex_cut.z / Texture_Width(texId);
	float uv_h = (float)tex_cut.w / Texture_Height(texId);

	ShaderBillBoard_SetUVParameter({ {uv_w, uv_h}, {uv_x, uv_y} });

	// ピクセルシェーダーに色を設定
	ShaderBillBoard_SetColor(color);

	// テクスチャの設定
	Texture_SetTexture(texId);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	Direct3D_GetContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 頂点シェーダーにワールド座標変換行列を設定

	// カメラ行列の回転だけ逆行列を作る
	// XMMATRIX iv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&g_mtxView));
	// 直交行列の逆行列は転置行列に等しい
	XMMATRIX iv = XMMatrixTranspose(XMLoadFloat4x4(&g_mtxView));

	// 回転軸までのオフセット行列
	XMMATRIX pivot_offset = XMMatrixTranslation(-pivot.x, -pivot.y, 0.0f);

	XMMATRIX s = XMMatrixScaling(scale.x, scale.y, 1.0f);
	XMMATRIX t = XMMatrixTranslation(position.x, position.y, position.z);
	ShaderBillBoard_SetWorldMatrix(s * pivot_offset * iv * t);

	// ポリゴン描画命令発行
	Direct3D_GetContext()->Draw(NUM_VERTEX, 0);
}
