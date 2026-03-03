/*==============================================================================

   タイトル制御 [title.cpp]
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
#include "debug_text.h"
#include <algorithm>
#include <cstdio>


static int g_TitleBgTexId = -1;  // タイトル背景画像

static constexpr double FIGHTER_SHOW_TIME{ 3.0 };
static constexpr double LOGO_SHOW_TIME{ 3.0 };
static double g_AccumulatedTime{ 0.0 };
static double g_ShowStartTime{ 0.0 };
static float g_FighterAlpha = 0.0f;
static float g_LogoAlpha = 0.0f;

// UIメニュー用
static int g_TitleSelect = 0;  // 0=Start, 1=Controls
static int g_WhiteTex = -1;
static hal::DebugText* g_pMenuText = nullptr;
static hal::DebugText* g_pCtrlText = nullptr;

enum TitleState
{
	TITLE_STATE_NONE,
	TITLE_STATE_FADE_IN,
	TITLE_STATE_FIGHTER_SHOW,
	TITLE_STATE_LOGO_SHOW,
	TITLE_STATE_KEY_WAIT,
	TITLE_STATE_FADE_OUT,
	TITLE_STATE_CONTROLS,
	TITLE_STATE_MAX
};

static TitleState g_State = TITLE_STATE_NONE;


void Title_Initialize()
{
	g_TitleBgTexId = Texture_Load(L"resource/texture/gametitle.png");
	g_WhiteTex = Texture_Load(L"resource/texture/white.png");
	g_ShowStartTime = 0.0;
	g_AccumulatedTime = 0.0;
	g_TitleSelect = 0;
	g_State = TITLE_STATE_KEY_WAIT;

	float sw = (float)Direct3D_GetBackBufferWidth();
	float sh = (float)Direct3D_GetBackBufferHeight();

	// メニューテキスト（画面中央下部）
	g_pMenuText = new hal::DebugText(
		Direct3D_GetDevice(), Direct3D_GetContext(),
		L"resource/texture/consolab_ascii_512.png",
		(UINT)sw, (UINT)sh,
		sw * 0.5f - 100.0f, sh * 0.65f,
		4, 0, 45.0f, 20.0f
	);

	// 操作説明テキスト（画面中央）
	g_pCtrlText = new hal::DebugText(
		Direct3D_GetDevice(), Direct3D_GetContext(),
		L"resource/texture/consolab_ascii_512.png",
		(UINT)sw, (UINT)sh,
		sw * 0.5f - 180.0f, sh * 0.25f,
		12, 0, 30.0f, 13.0f
	);
}

void Title_Finalize()
{
	if (g_pMenuText) { delete g_pMenuText; g_pMenuText = nullptr; }
	if (g_pCtrlText) { delete g_pCtrlText; g_pCtrlText = nullptr; }
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
		// Enterで演出スキップ
		if (KeyLogger_IsTrigger(KK_ENTER)) {
			g_State = TITLE_STATE_KEY_WAIT;
		}
		break;
	case TITLE_STATE_FIGHTER_SHOW:
		g_FighterAlpha = std::min((float)((g_AccumulatedTime - g_ShowStartTime) / FIGHTER_SHOW_TIME), 1.0f);
		if (g_FighterAlpha >= 1.0f) {
			g_State = TITLE_STATE_KEY_WAIT;
		}
		// Enterで演出スキップ
		if (KeyLogger_IsTrigger(KK_ENTER)) {
			g_State = TITLE_STATE_KEY_WAIT;
		}
		break;
	case TITLE_STATE_LOGO_SHOW:
		g_State = TITLE_STATE_KEY_WAIT;
		break;
	case TITLE_STATE_KEY_WAIT:
		// 上下キーでメニュー選択
		if (KeyLogger_IsTrigger(KK_UP)) {
			g_TitleSelect--;
			if (g_TitleSelect < 0) g_TitleSelect = 1;
		}
		if (KeyLogger_IsTrigger(KK_DOWN)) {
			g_TitleSelect++;
			if (g_TitleSelect > 1) g_TitleSelect = 0;
		}
		// Enter で決定
		if (KeyLogger_IsTrigger(KK_ENTER)) {
			if (g_TitleSelect == 0) {
				// Start → ゲームシーンへ直接遷移
				Scene_Change(SCENE_GAME);
			} else {
				// Controls
				g_State = TITLE_STATE_CONTROLS;
			}
		}
		break;
	case TITLE_STATE_CONTROLS:
		// Enter or Escape で戻る
		if (KeyLogger_IsTrigger(KK_ENTER) || KeyLogger_IsTrigger(KK_ESCAPE)) {
			g_State = TITLE_STATE_KEY_WAIT;
		}
		break;
	case TITLE_STATE_FADE_OUT:
		if (Fade_GetState() == FADE_STATE_FINISHED_OUT) {
			Scene_Change(SCENE_GAME);
		}
		break;
	}
}

void Title_Draw()
{
	float sw = (float)Direct3D_GetBackBufferWidth();
	float sh = (float)Direct3D_GetBackBufferHeight();

	// タイトル背景画像を全画面描画
	Sprite_Draw(g_TitleBgTexId, 0.0f, 0.0f, sw, sh);

	// メニュー表示（KEY_WAIT / FADE_OUT）
	if (g_State == TITLE_STATE_KEY_WAIT || g_State == TITLE_STATE_FADE_OUT) {
		float menuX = sw * 0.5f - 100.0f;
		float menuY = sh * 0.65f;
		float btnW = 260.0f;
		float btnH = 38.0f;
		float lineH = 45.0f;

		// 選択中のボタン背景ハイライト
		Sprite_Draw(g_WhiteTex,
			menuX - 15.0f, menuY + g_TitleSelect * lineH - 3.0f,
			btnW, btnH,
			{ 1.0f, 1.0f, 0.0f, 0.3f });

		// メニューテキスト
		if (g_pMenuText) {
			g_pMenuText->Clear();
			char buf[64];
			snprintf(buf, sizeof(buf), "%c Start\n%c Controls",
				g_TitleSelect == 0 ? '>' : ' ',
				g_TitleSelect == 1 ? '>' : ' ');
			g_pMenuText->SetText(buf, { 1.0f, 1.0f, 1.0f, 1.0f });
			g_pMenuText->Draw();
		}
	}

	// 操作説明オーバーレイ
	if (g_State == TITLE_STATE_CONTROLS) {
		// 半透明黒オーバーレイ
		Sprite_Draw(g_WhiteTex, 0.0f, 0.0f, sw, sh, { 0.0f, 0.0f, 0.0f, 0.8f });

		if (g_pCtrlText) {
			g_pCtrlText->Clear();
			g_pCtrlText->SetText(
				"[Controls]\n"
				"\n"
				"Arrow Keys : Move\n"
				"Tab        : Switch Fighter\n"
				"Space      : Shoot (Battle)\n"
				"Enter      : Select\n"
				"M          : Status Menu\n"
				"\n"
				"[Enter / Esc] Back",
				{ 1.0f, 1.0f, 1.0f, 1.0f });
			g_pCtrlText->Draw();
		}
	}
}
