/*==============================================================================

   メッシュフィールドの表示 [meshfield.cpp]
														 Author : Youhei Sato
														 Date   : 2025/09/19
--------------------------------------------------------------------------------

==============================================================================*/
#include "meshfield.h"
#include "direct3d.h"
using namespace DirectX;
#include "shader_field.h"
#include "texture.h"
#include "camera.h"


// メッシュフィールドデータ
static constexpr float FIELD_MESH_SIZE = 1.0f; // メッシュ１枚分のサイズ（正方形)
static constexpr int FIELD_MESH_H_COUNT = 50; // 横のメッシュ数
static constexpr int FIELD_MESH_V_COUNT = 50; // 縦のメッシュ数
static constexpr int FIELD_MESH_H_VERTEX_COUNT = FIELD_MESH_H_COUNT + 1; // 横の頂点数
static constexpr int FIELD_MESH_V_VERTEX_COUNT = FIELD_MESH_V_COUNT + 1; // 縦の頂点数
static constexpr int NUM_VERTEX = FIELD_MESH_H_VERTEX_COUNT * FIELD_MESH_V_VERTEX_COUNT; // 頂点数
static constexpr int NUM_INDEX = 3 * 2 * FIELD_MESH_H_COUNT * FIELD_MESH_V_COUNT; // インデックス数

static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
static ID3D11Buffer* g_pIndexBuffer = nullptr; // インデックスバッファ

static int g_Tex0Id = -1;
static int g_Tex1Id = -1;

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

static Vertex3d g_MeshFieldVertex[NUM_VERTEX];
static unsigned short g_MeshFieldIndex[NUM_INDEX];


void MeshField_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex3d) * NUM_VERTEX; // sizeof(g_MeshFieldVertex);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// 頂点情報を配列に作る
	for (int z = 0; z < FIELD_MESH_V_VERTEX_COUNT; z++)	{
		for (int x = 0; x < FIELD_MESH_H_VERTEX_COUNT; x++) {
			// 横 + 横の最大数 * 縦;
			int index = x + FIELD_MESH_H_VERTEX_COUNT * z;
			g_MeshFieldVertex[index].position = { x * FIELD_MESH_SIZE, 0.0f, z * FIELD_MESH_SIZE };
			g_MeshFieldVertex[index].normal = { 0.0f, 1.0f, 0.0f };
			g_MeshFieldVertex[index].color = { 0.0f, 1.0f, 0.0f, 1.0f };
			g_MeshFieldVertex[index].texcoord = { x * 1.0f, z * 1.0f };
		}
	}

	for (int z = 0; z < FIELD_MESH_V_VERTEX_COUNT; z++) {
		int index = 26 + FIELD_MESH_H_VERTEX_COUNT * z;
		g_MeshFieldVertex[index].color = { 1.0f, 0.0f, 0.0f, 1.0f };
		index = 25 + FIELD_MESH_H_VERTEX_COUNT * z;
		g_MeshFieldVertex[index].color = { 1.0f, 0.0f, 0.0f, 1.0f };
		index = 27 + FIELD_MESH_H_VERTEX_COUNT * z;
		g_MeshFieldVertex[index].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	}


	D3D11_SUBRESOURCE_DATA sd{};
	sd.pSysMem = g_MeshFieldVertex;

	g_pDevice->CreateBuffer(&bd, &sd, &g_pVertexBuffer);

	// インデックスバッファ作成
	int index = 0;
	for (int v = 0; v < FIELD_MESH_V_COUNT; v++) {
		for (int h = 0; h < FIELD_MESH_H_COUNT; h++) {
			g_MeshFieldIndex[index + 0] = (unsigned short)(h + (v + 0) * FIELD_MESH_H_VERTEX_COUNT);     // 0 1   5
			g_MeshFieldIndex[index + 1] = (unsigned short)(h + (v + 1) * FIELD_MESH_H_VERTEX_COUNT + 1); // 5 6   10
			g_MeshFieldIndex[index + 2] = g_MeshFieldIndex[index + 0] + 1;             // 1 2   6
			g_MeshFieldIndex[index + 3] = g_MeshFieldIndex[index + 0];                 // 0 1   5
			g_MeshFieldIndex[index + 4] = g_MeshFieldIndex[index + 1] - 1;             // 4 5   9
			g_MeshFieldIndex[index + 5] = g_MeshFieldIndex[index + 1];                 // 5 6   10
			index += 6;
		}
	}

	bd.ByteWidth = sizeof(unsigned short) * NUM_INDEX; // sizeof(g_MeshFieldIndex);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	// インデックス情報を配列に作る
	sd.pSysMem = g_MeshFieldIndex;

	g_pDevice->CreateBuffer(&bd, &sd, &g_pIndexBuffer);

	g_Tex0Id = Texture_Load(L"resource/Texture/Bark_Soil_Mix_sesjcefb_1K_BaseColor.jpg");
	g_Tex1Id = Texture_Load(L"resource/Texture/Ground_Gravel_ukxmacclw_1K_BaseColor.jpg");
	// g_Tex1Id = Texture_Load(L"resource/Texture/knight.png");

	ShaderField_Initialize(pDevice, pContext);
}

void MeshField_Finalize()
{
	ShaderField_Finalize();

	SAFE_RELEASE(g_pIndexBuffer);
	SAFE_RELEASE(g_pVertexBuffer);
}

void MeshField_Draw()
{
	// シェーダーを描画パイプラインに設定
	ShaderField_Begin();

	// テクスチャの設定
	Texture_SetTexture(g_Tex0Id, 0);
	Texture_SetTexture(g_Tex1Id, 1);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3d);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// インデックスバッファを描画パイプラインに設定
	g_pContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// プリミティブトポロジ設定
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 頂点シェーダーにワールド座標変換行列を設定
	float offset_x = FIELD_MESH_H_COUNT * FIELD_MESH_SIZE * 0.5f;
	float offset_z = FIELD_MESH_V_COUNT * FIELD_MESH_SIZE * 0.5f;
	ShaderField_SetWorldMatrix(XMMatrixTranslation(-offset_x, 0.0f, -offset_z));

	ShaderField_SetColor({ 2.0f, 2.0f, 2.0f, 1.0f });

	// ポリゴン描画命令発行
	g_pContext->DrawIndexed(NUM_INDEX, 0, 0);
}
