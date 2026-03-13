/*==============================================================================

   テクスチャ管理 [texture.cpp]
														 Author : Youhei Sato
														 Date   : 2025/06/13
--------------------------------------------------------------------------------

==============================================================================*/
#include "texture.h"
#include "direct3d.h"
#include <string>
#include "WICTextureLoader11.h"
using namespace DirectX;


static constexpr int TEXTURE_MAX = 1024;


struct Texture
{
	std::wstring filename;
	unsigned int width;
	unsigned int height;
	ID3D11Resource* pTexture = nullptr;
	ID3D11ShaderResourceView* pTextureView = nullptr;
};


static Texture g_Textures[TEXTURE_MAX]{};
static int g_SetTextureIndex = -1;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;


void Texture_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	for (Texture& t : g_Textures) {
		t.pTexture = nullptr;
	}
	
	g_SetTextureIndex = -1;

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;
}

void Texture_Finalize(void)
{
	Texture_AllRelease();
}

int Texture_Load(const wchar_t* pFilename)
{
	// すでに読み込んだファイルは読み込まない
	for (int i = 0; i < TEXTURE_MAX; i++) {
		if (g_Textures[i].filename == pFilename) {
			return i;
		}
	}

	// 空いている管理領域を探す
	for (int i = 0; i < TEXTURE_MAX; i++) {

		if (g_Textures[i].pTexture) continue; // 使用中

		// テクスチャの読み込み
		HRESULT hr;

		hr = CreateWICTextureFromFile(g_pDevice, g_pContext, pFilename, &g_Textures[i].pTexture, &g_Textures[i].pTextureView);
		
		if (FAILED(hr)) {

			std::wstring errMsg = L"テクスチャの読み込みに失敗しました \n 指定されたパス：";
			errMsg += pFilename;
			MessageBoxW(nullptr,errMsg.c_str(), pFilename, MB_OK | MB_ICONERROR);
			return -1;
		}
		ID3D11Texture2D* pTexture = (ID3D11Texture2D*)g_Textures[i].pTexture;
		D3D11_TEXTURE2D_DESC t2desc;
 		pTexture->GetDesc(&t2desc);
		g_Textures[i].width = t2desc.Width;
		g_Textures[i].height = t2desc.Height;


		g_Textures[i].filename = pFilename;

		return i;
	}

	return -1;
}

int Texture_CreateFromMemory(const void* pPixels, unsigned int width, unsigned int height, const wchar_t* tag)
{
	// 同じタグがあれば既存を返す
	if (tag) {
		for (int i = 0; i < TEXTURE_MAX; i++) {
			if (g_Textures[i].pTexture && g_Textures[i].filename == tag) {
				return i;
			}
		}
	}

	// 空きスロットを探す
	for (int i = 0; i < TEXTURE_MAX; i++) {
		if (g_Textures[i].pTexture) continue;

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = pPixels;
		initData.SysMemPitch = width * 4;

		ID3D11Texture2D* pTex2D = nullptr;
		HRESULT hr = g_pDevice->CreateTexture2D(&desc, &initData, &pTex2D);
		if (FAILED(hr)) return -1;

		hr = g_pDevice->CreateShaderResourceView(pTex2D, nullptr, &g_Textures[i].pTextureView);
		if (FAILED(hr)) {
			pTex2D->Release();
			return -1;
		}

		g_Textures[i].pTexture = pTex2D;
		g_Textures[i].width = width;
		g_Textures[i].height = height;
		if (tag) g_Textures[i].filename = tag;

		return i;
	}

	return -1;
}

void Texture_Release(int texid)
{
	if (texid < 0 || texid >= TEXTURE_MAX) return;
	g_Textures[texid].filename.clear();
	SAFE_RELEASE(g_Textures[texid].pTexture);
	SAFE_RELEASE(g_Textures[texid].pTextureView);
}

void Texture_AllRelease()
{
	for (Texture& t : g_Textures) {
		t.filename.clear();
		SAFE_RELEASE(t.pTexture);
		SAFE_RELEASE(t.pTextureView);
	}
}

void Texture_SetTexture(int texid, int slot)
{
	if (texid < 0) return;

	g_SetTextureIndex = texid;

	// テクスチャ設定
	g_pContext->PSSetShaderResources(slot, 1, &g_Textures[texid].pTextureView);
}

unsigned int Texture_Width(int texid)
{
	if (texid < 0) return 0;

	return g_Textures[texid].width;
}

unsigned int Texture_Height(int texid)
{
	if (texid < 0) return 0;

	return g_Textures[texid].height;
}
