/*==============================================================================

   丸影の表示 [circle_shadow.h]
														 Author : Youhei Sato
														 Date   : 2025/05/12
--------------------------------------------------------------------------------

==============================================================================*/
#include "cirel_shadow.h"
#include "direct3d.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "shader3d_unlit.h"
#include "texture.h"
#include"collision.h"
#include"map.h"

static constexpr int NUM_VERTEX = 4; // 頂点数

static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
static int g_TexId{ -1 };

// ミニマップ描画中かどうかのフラグ
static bool g_IsRenderingMinimap = false;

// 3D頂点構造体
struct Vertex3d
{
	XMFLOAT3 position; // 座標
	XMFLOAT3 normal; // 法線
	XMFLOAT4 color;    // 色
	XMFLOAT2 texcoord; // UV
};

void CircleShadow_Initialize()
{ 
	Vertex3d vertex[]{
		{{-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f },  {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{ 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f },  {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f },  {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{ 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f },  {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
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

	g_TexId = Texture_Load(L"resource/texture/marukage.png");
}

void CircleShadow_Finalize()
{
	SAFE_RELEASE(g_pVertexBuffer);
}

void CircleShadow_SetMinimapMode(bool is_minimap)
{
	g_IsRenderingMinimap = is_minimap;
}


void CircleShadow_Draw(const DirectX::XMFLOAT3& position)
{
	// ミニマップ描画中なら影を描画しない
	if (g_IsRenderingMinimap) {
		return;
	}
	float y = -1000.0f;
	//地面との当たり判定(position.yより下)
	AABB shadow_aabb{ { position.x - 0.5f, position.y - 10.0f , position.z - 0.5f},
		{position.x + 0.5f,position.y,position.z + 0.5f} };
	
	// 重力に引かれたあとのプレイヤーとマップオブジェクトとの当たり判定
	for (int i = 0; i < Map_GetObjectsCount(); i++) {

		// ここがオブジェクトによって変わるはず
		AABB object = Map_GetObject(i)->Aabb;

		bool isHit = Collision_IsOverlapAABB(object, shadow_aabb);

		if (isHit) {
			if (y < object.max.y) {
				y = object.max.y;
			}
		}
	}
	Shader3dUnlit_Begin();

	// ピクセルシェーダーに色を設定
	Shader3dUnlit_SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	// テクスチャの設定_
	Texture_SetTexture(g_TexId);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	Direct3D_GetContext()->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Shader3dUnlit_SetWorldMatrix(
		XMMatrixScaling(4.0f, 1.0f, 4.0f) * 
		XMMatrixTranslation(position.x, y + 0.01f, position.z));

	// ポリゴン描画命令発行
	Direct3D_GetContext()->Draw(NUM_VERTEX, 0);
}

