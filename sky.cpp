/*==============================================================================

   ‹ó‚Ì•\Ž¦ [sky.cpp]
														 Author : Youhei Sato
														 Date   : 2025/11/21
--------------------------------------------------------------------------------

==============================================================================*/
#include "sky.h"
using namespace DirectX;
#include "model.h"
#include "shader3d_unlit.h"


static MODEL* g_pModelSky{ nullptr };
static XMFLOAT3 g_Position{};


void Sky_Initialize()
{
	g_pModelSky = ModelLoad("resource/model/sky.fbx", 100.0f, true);
}

void Sky_Finalize()
{
	ModelRelease(g_pModelSky);
}

void Sky_SetPosition(const DirectX::XMFLOAT3& position)
{
	g_Position = position;
}

void Sky_Draw()
{
	Shader3dUnlit_Begin();

	g_Position.y = 0;

	ModelUnlitDraw(g_pModelSky, XMMatrixTranslationFromVector(XMLoadFloat3(&g_Position)));
}
