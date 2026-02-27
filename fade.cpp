/*==============================================================================

   フェードイン・アウト制御 [fade.cpp]
														 Author : Youhei Sato
														 Date   : 2025/07/10
--------------------------------------------------------------------------------

==============================================================================*/
#include "fade.h"
using namespace DirectX;
#include <algorithm>
#include "texture.h"
#include "sprite.h"
#include "direct3d.h"


static double g_FadeTime{ 0.0 };
static double g_FadeStartTime{ 0.0 };
static double g_AccumulatedTime{ 0.0 };
static XMFLOAT3 g_Color{ 0.0f, 0.0f, 0.0f };
static float g_Alpha = 0.0f;
static FadeState g_State = FADE_STATE_NONE;
static int g_FadeTexId = -1;


void Fade_Initialize()
{
	g_FadeTime = 0.0;
	g_FadeStartTime = 0.0;
	g_AccumulatedTime = 0.0;
	g_Color = { 0.0f, 0.0f, 0.0f };
	g_Alpha = 0.0f;
	g_State = FADE_STATE_NONE;

	g_FadeTexId = Texture_Load(L"resource/texture/white.png");
}

void Fade_Finalize()
{
}

void Fade_Update(double elapsed_time)
{
	if (g_State <= FADE_STATE_FINISHED_OUT) return;

	g_AccumulatedTime += elapsed_time;

	double ratio = std::min((g_AccumulatedTime - g_FadeStartTime) / g_FadeTime, 1.0);

	if (ratio >= 1.0) {
		g_State = g_State == FADE_STATE_IN ? FADE_STATE_FINISHED_IN : FADE_STATE_FINISHED_OUT;
	}

	g_Alpha = (float)(g_State == FADE_STATE_IN ? 1.0 - ratio : ratio);
}

void Fade_Draw()
{
	if (g_State == FADE_STATE_NONE) return;
	if (g_State == FADE_STATE_FINISHED_IN) return;

	XMFLOAT4 color{ g_Color.x, g_Color.y, g_Color.z, g_Alpha };
	Sprite_Draw(g_FadeTexId, 0.0f, 0.0f, 
		(float)Direct3D_GetBackBufferWidth(), (float)Direct3D_GetBackBufferHeight(), color);
}

void Fade_Start(double time, bool isFadeOut, DirectX::XMFLOAT3 color)
{
	g_FadeStartTime = g_AccumulatedTime;
	g_FadeTime = time;
	g_State = isFadeOut ? FADE_STATE_OUT : FADE_STATE_IN;
	g_Color = color;
	g_Alpha = isFadeOut ? 0.0f : 1.0f;
}

FadeState Fade_GetState()
{
	return g_State;
}
