/*==============================================================================

   É^ÉCÉgÉãêßå‰ [title.cpp]
														 Author : Youhei Sato
														 Date   : 2025/07/11
--------------------------------------------------------------------------------

==============================================================================*/
#include "title.h"
#include "texture.h"
#include "sprite.h"
#include "key_logger.h"
#include "fade.h"
#include "scene.h"
#include "direct3d.h"
#include <algorithm>


static int g_TitleBg00TexId = -1;
static int g_TitleBg01TexId = -1;
static int g_TitleBg02TexId = -1;

static constexpr double FIGHTER_SHOW_TIME{ 3.0 };
static constexpr double LOGO_SHOW_TIME{ 3.0 };
static double g_AccumulatedTime{ 0.0 };
static double g_ShowStartTime{ 0.0 };
static float g_FighterAlpha = 0.0f;
static float g_LogoAlpha = 0.0f;


enum TitleState
{
	TITLE_STATE_NONE,
	TITLE_STATE_FADE_IN,
	TITLE_STATE_FIGHTER_SHOW,
	TITLE_STATE_LOGO_SHOW,
	TITLE_STATE_KEY_WAIT,
	TITLE_STATE_FADE_OUT,
	TITLE_STATE_MAX
};

static TitleState g_State = TITLE_STATE_NONE;


void Title_Initialize()
{
	g_TitleBg00TexId = Texture_Load(L"resource/texture/title000.bmp");
	g_TitleBg01TexId = Texture_Load(L"resource/texture/title001.png");
	g_TitleBg02TexId = Texture_Load(L"resource/texture/title002.png");
	g_ShowStartTime = 0.0;
	g_AccumulatedTime = 0.0;
	Fade_Start(3.0, false);
	g_State = TITLE_STATE_FADE_IN;
}

void Title_Finalize()
{
	// Texture_AllRelease();
}

void Title_Update(double elapsed_time)
{
	g_AccumulatedTime += elapsed_time;

	switch(g_State)
	{ 
	case TITLE_STATE_FADE_IN:
		if (Fade_GetState() == FADE_STATE_FINISHED_IN) {
			g_ShowStartTime = g_AccumulatedTime;
			g_State = TITLE_STATE_FIGHTER_SHOW;
		}
		break;
	case TITLE_STATE_FIGHTER_SHOW:
		g_FighterAlpha = std::min((float)((g_AccumulatedTime - g_ShowStartTime) / FIGHTER_SHOW_TIME), 1.0f);
		if (g_FighterAlpha >= 1.0f) {
			g_State = TITLE_STATE_LOGO_SHOW;
		}
		break;
	case TITLE_STATE_LOGO_SHOW:
		break;
	case TITLE_STATE_KEY_WAIT:
		break;
	case TITLE_STATE_FADE_OUT:
		break;
	}

	if (KeyLogger_IsTrigger(KK_ENTER)) {
		Fade_Start(1.0, true);
	}

	if (Fade_GetState() == FADE_STATE_FINISHED_OUT) {
		Scene_Change(SCENE_GAME);
	}
}

void Title_Draw()
{
	switch (g_State)
	{
	case TITLE_STATE_FADE_IN:
		break;
	case TITLE_STATE_FIGHTER_SHOW:
		break;
	case TITLE_STATE_LOGO_SHOW:
		break;
	case TITLE_STATE_KEY_WAIT:
		break;
	case TITLE_STATE_FADE_OUT:
		break;
	}

	Sprite_Draw(g_TitleBg00TexId, 0.0f, 0.0f,
		(float)Direct3D_GetBackBufferWidth(), (float)Direct3D_GetBackBufferHeight());

	Sprite_Draw(g_TitleBg01TexId, 0.0f, 0.0f, { 1.0f, 1.0f, 1.0f, g_FighterAlpha });
}
