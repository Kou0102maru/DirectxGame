/*==============================================================================

   ライトの設定 [light.h]
														 Author : Youhei Sato
														 Date   : 2025/09/30
--------------------------------------------------------------------------------

==============================================================================*/
#include "light.h"
using namespace DirectX;
#include "direct3d.h"


static ID3D11Buffer* g_pPSConstantBuffer1 = nullptr; // 定数バッファb1
static ID3D11Buffer* g_pPSConstantBuffer2 = nullptr; // 定数バッファb2
static ID3D11Buffer* g_pPSConstantBuffer3 = nullptr; // 定数バッファb3
static ID3D11Buffer* g_pPSConstantBuffer4 = nullptr; // 定数バッファb4
static ID3D11Buffer* g_pPSConstantBuffer5 = nullptr; // 定数バッファb5


// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// 並行光源（拡散反射光）
struct DirectionalLight
{
	XMFLOAT4 Directional;
	XMFLOAT4 Color;
};

// 鏡面反射光
struct SpecularLight
{
	XMFLOAT3 CameraPosition;
	float Power;
	XMFLOAT4 Color;
};

// 点光源（ポイントライト）
struct PointLight
{
	XMFLOAT3 LightPosition;
	float Range;
	XMFLOAT4 Color;
	// float SpecularPower;
	// XMFLOAT3 SpecularColor;
};

struct PointLightList
{
	PointLight light[4];
	int count;
	XMFLOAT3 dummy;
};

static PointLightList g_PointLights{};


void Light_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ

	buffer_desc.ByteWidth = sizeof(XMFLOAT4); // バッファのサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer1); // ambient

	buffer_desc.ByteWidth = sizeof(DirectionalLight); // バッファのサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer2); // directional

	buffer_desc.ByteWidth = sizeof(SpecularLight); // バッファのサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer3); // specular

	buffer_desc.ByteWidth = sizeof(PointLightList); // バッファのサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer4); // specular

	buffer_desc.ByteWidth = sizeof(XMFLOAT4); // バッファのサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSConstantBuffer5); // specular
}

void Light_Finalize()
{
	SAFE_RELEASE(g_pPSConstantBuffer5);
	SAFE_RELEASE(g_pPSConstantBuffer4);
	SAFE_RELEASE(g_pPSConstantBuffer3);
	SAFE_RELEASE(g_pPSConstantBuffer2);
	SAFE_RELEASE(g_pPSConstantBuffer1);
}

void Light_SetAmbient(const XMFLOAT3& color)
{
	// 定数バッファアンビエントをセット
	XMFLOAT4 ambient(color.x, color.y, color.z, 1.0f);
	g_pContext->UpdateSubresource(g_pPSConstantBuffer1, 0, nullptr, &ambient, 0, 0);
	g_pContext->PSSetConstantBuffers(1, 1, &g_pPSConstantBuffer1);
}


void Light_SetDirectionalWorld(const DirectX::XMFLOAT4& world_directional, const DirectX::XMFLOAT4& color)
{
	DirectionalLight light{ world_directional,color };

	g_pContext->UpdateSubresource(g_pPSConstantBuffer2, 0, nullptr, &light, 0, 0);
	g_pContext->PSSetConstantBuffers(2, 1, &g_pPSConstantBuffer2);
}

void Light_SetSpecularWorld(const DirectX::XMFLOAT3& camera_position, float power, const DirectX::XMFLOAT4& color)
{
	SpecularLight light{ camera_position,power,color };

	g_pContext->UpdateSubresource(g_pPSConstantBuffer3, 0, nullptr, &light, 0, 0);
	g_pContext->PSSetConstantBuffers(3, 1, &g_pPSConstantBuffer3);
}

void Light_SetLimLight(const DirectX::XMFLOAT3& color, float power)
{
	XMFLOAT4 lim{ color.x, color.y, color.z, power };
	g_pContext->UpdateSubresource(g_pPSConstantBuffer5, 0, nullptr, &lim, 0, 0);
	g_pContext->PSSetConstantBuffers(5, 1, &g_pPSConstantBuffer5);
}

void Light_SetPointLightCount(int count)
{
	g_PointLights.count = count;

	g_pContext->UpdateSubresource(g_pPSConstantBuffer4, 0, nullptr, &g_PointLights, 0, 0);
	g_pContext->PSSetConstantBuffers(4, 1, &g_pPSConstantBuffer4);
}

void Light_SetPointLight(int n, const DirectX::XMFLOAT3& position, float range, const DirectX::XMFLOAT3& color)
{
	g_PointLights.light[n].LightPosition = position;
	g_PointLights.light[n].Range = range;
	g_PointLights.light[n].Color = { color.x, color.y, color.z, 1.0f };

	g_pContext->UpdateSubresource(g_pPSConstantBuffer4, 0, nullptr, &g_PointLights, 0, 0);
	g_pContext->PSSetConstantBuffers(4, 1, &g_pPSConstantBuffer4);
}
