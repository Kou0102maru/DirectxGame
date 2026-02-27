/*==============================================================================

   ３Ｄキューブの表示 [cube.cpp]
														 Author : Youhei Sato
														 Date   : 2025/09/09
--------------------------------------------------------------------------------

==============================================================================*/
#include "cube.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader3d.h"
#include"shader_depth.h"
#include "texture.h"


static constexpr int NUM_VERTEX = 4 * 6; // 頂点数
static constexpr int NUM_INDEX = 3 * 2 * 6; // インデックス数

static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
static ID3D11Buffer* g_pIndexBuffer = nullptr; // インデックスバッファ


// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


// 3D頂点構造体
struct Vertex3d
{
	XMFLOAT3 position; // 座標
	XMFLOAT3 normal;   // 法線
	XMFLOAT4 color;    // 色
	XMFLOAT2 texcoord; // UV
};


static Vertex3d g_CubeVertex[]{
	// 前
	{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f,  0.0f }}, // 0
	{{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.25f}}, // 1
	{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f,  0.25f}}, // 2
// 	{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f,  0.0f }},
	{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.0f }}, // 3
//	{{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.25f}},
	// 右
	{{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.0f }}, // 4
	{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.50f, 0.25f}}, // 5
	{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.25f}}, // 6
//	{{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.0f }},
	{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.50f, 0.0f }}, // 7
//	{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.50f, 0.25f}},
	// 後ろ
	{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f, 1.0f}, {0.50f, 0.0f }},
	{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f, 1.0f}, {0.75f, 0.25f}},
	{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f, 1.0f}, {0.50f, 0.25f}},
//	{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f, 1.0f}, {0.50f, 0.0f }},
	{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f, 1.0f}, {0.75f, 0.0f }},
//	{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f},{1.0f, 1.0f, 1.0f, 1.0f}, {0.75f, 0.25f}},
	// 左
	{{ -0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.75f, 0.0f }},
	{{ -0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f,  0.25f}},
	{{ -0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.75f, 0.25f}},
//	{{ -0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.75f, 0.0f }},
	{{ -0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f,  0.0f }},
//	{{ -0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f,  0.25f}},
	// 上
	{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f,  0.25f}},
	{{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.50f}},
	{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f,  0.50f}},
//	{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f,  0.25f}},
	{{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.25f}},
//	{{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.50f}},
	// 下
	{{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.25f}},
	{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.50f, 0.50f}},
	{{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.50f}},
//	{{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.25f, 0.25f}},
	{{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.50f, 0.25f}},
//	{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.50f, 0.50f}},
};

static unsigned short g_CubeIndex[]{
	0, 1, 2, 0, 3, 1,
	4, 5, 6, 4, 7, 5,
	8, 9, 10, 8, 11, 9,
	12, 13, 14, 12, 15, 13,
	16, 17, 18, 16, 19, 17,
	20, 21, 22, 20, 23, 21
};


void Cube_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX; // sizeof(g_CubeVertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = g_CubeVertex;

	g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	// インデックスバッファ作成
	bd.ByteWidth = sizeof(unsigned short) * NUM_INDEX; // sizeof(g_CubeIndex);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER; 

	sd.pSysMem = g_CubeIndex;

	g_pDevice->CreateBuffer(&bd, &sd, &g_pIndexBuffer);
}

void Cube_Finalize(void)
{
	SAFE_RELEASE(g_pIndexBuffer);
	SAFE_RELEASE(g_pVertexBuffer);
}

void Cube_Draw(int texId, const DirectX::XMMATRIX& mtxWorld)
{
	// シェーダーを描画パイプラインに設定
	Shader3d_Begin();

	// ピクセルシェーダーに色を設定
	Shader3d_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	// テクスチャの設定
	Texture_SetTexture(texId);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// インデックスバッファを描画パイプラインに設定
	g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 頂点シェーダーにワールド座標変換行列を設定
	Shader3d_SetWorldMatrix(mtxWorld);

	// ポリゴン描画命令発行
	g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
}

void Cube_DepthDraw(const DirectX::XMMATRIX& mtxWorld)
{
	// シェーダーを描画パイプラインに設定
	ShaderDepth_Begin();

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// インデックスバッファを描画パイプラインに設定
	g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 頂点シェーダーにワールド座標変換行列を設定
	ShaderDepth_SetWorldMatrix(mtxWorld);

	// ポリゴン描画命令発行
	g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
}

AABB Cube_GetAABB(const DirectX::XMFLOAT3& position)
{
	return { 
		{ position.x - 0.5f, position.y - 0.5f, position.z - 0.5f }, 
		{ position.x + 0.5f, position.y + 0.5f, position.z + 0.5f } 
	};
}
