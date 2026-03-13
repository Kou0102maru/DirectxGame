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
#include "pad_logger.h"
#include "fade.h"
#include "scene.h"
#include "direct3d.h"
#include <algorithm>


static int g_TitleBgTexId = -1;

// UIメニュー用
static int g_TitleSelect = 0;  // 0=Start, 1=Controls
static int g_WhiteTex = -1;
static int g_BtnStartTex = -1;
static int g_BtnControlsTex = -1;
static int g_ControlsInfoTex = -1;

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
	g_BtnStartTex = Texture_Load(L"resource/texture/btn_start.png");
	g_BtnControlsTex = Texture_Load(L"resource/texture/btn_controls.png");
	g_ControlsInfoTex = Texture_Load(L"resource/texture/controls_info.png");
	g_TitleSelect = 0;
	g_State = TITLE_STATE_KEY_WAIT;
}

void Title_Finalize()
{
}

void Title_Update(double elapsed_time)
{
	switch(g_State)
	{
	case TITLE_STATE_KEY_WAIT:
		// 上下キー / D-pad でメニュー選択
		if (KeyLogger_IsTrigger(KK_UP) || KeyLogger_IsTrigger(KK_W)
			|| PadLogger_IsTrigger(0, XINPUT_GAMEPAD_DPAD_UP)) {
			g_TitleSelect--;
			if (g_TitleSelect < 0) g_TitleSelect = 1;
		}
		if (KeyLogger_IsTrigger(KK_DOWN) || KeyLogger_IsTrigger(KK_S)
			|| PadLogger_IsTrigger(0, XINPUT_GAMEPAD_DPAD_DOWN)) {
			g_TitleSelect++;
			if (g_TitleSelect > 1) g_TitleSelect = 0;
		}
		// Enter / Aボタン で決定
		if (KeyLogger_IsTrigger(KK_ENTER)
			|| PadLogger_IsTrigger(0, XINPUT_GAMEPAD_A)) {
			if (g_TitleSelect == 0) {
				// Start → ゲームシーン
				Scene_Change(SCENE_GAME);
			} else {
				// Controls
				g_State = TITLE_STATE_CONTROLS;
			}
		}
		break;
	case TITLE_STATE_CONTROLS:
		// Enter / Escape / A / Bボタン で戻る
		if (KeyLogger_IsTrigger(KK_ENTER) || KeyLogger_IsTrigger(KK_ESCAPE)
			|| PadLogger_IsTrigger(0, XINPUT_GAMEPAD_A)
			|| PadLogger_IsTrigger(0, XINPUT_GAMEPAD_B)) {
			g_State = TITLE_STATE_KEY_WAIT;
		}
		break;
	}
}

void Title_Draw()
{
	Direct3D_SetBackBuffer();
	Direct3D_ClearBackBuffer();

	Direct3D_SetDepthEnable(false);
	Sprite_Begin();

	float sw = (float)Direct3D_GetBackBufferWidth();
	float sh = (float)Direct3D_GetBackBufferHeight();

	// タイトル背景画像を全画面描画
	Sprite_Draw(g_TitleBgTexId, 0.0f, 0.0f, sw, sh);

	// メニュー表示
	if (g_State == TITLE_STATE_KEY_WAIT) {
		float btnW = 256.0f;
		float btnH = 64.0f;
		float lineH = 70.0f;
		float menuX = sw * 0.5f - btnW * 0.5f;
		float menuY = sh * 0.62f;

		// メニュー全体の暗い背景パネル
		Sprite_Draw(g_WhiteTex,
			menuX - 20.0f, menuY - 15.0f,
			btnW + 40.0f, lineH * 2.0f + 10.0f,
			{ 0.0f, 0.0f, 0.0f, 0.6f });

		// 選択中のボタン背景ハイライト
		Sprite_Draw(g_WhiteTex,
			menuX - 10.0f, menuY + g_TitleSelect * lineH - 5.0f,
			btnW + 20.0f, btnH + 10.0f,
			{ 0.3f, 0.5f, 1.0f, 0.4f });

		// スタートボタン（選択中=白、非選択=やや暗め）
		if (g_TitleSelect == 0)
			Sprite_Draw(g_BtnStartTex, menuX, menuY, btnW, btnH, { 1.0f, 1.0f, 1.0f, 1.0f });
		else
			Sprite_Draw(g_BtnStartTex, menuX, menuY, btnW, btnH, { 0.7f, 0.7f, 0.7f, 0.9f });

		// 操作説明ボタン
		if (g_TitleSelect == 1)
			Sprite_Draw(g_BtnControlsTex, menuX, menuY + lineH, btnW, btnH, { 1.0f, 1.0f, 1.0f, 1.0f });
		else
			Sprite_Draw(g_BtnControlsTex, menuX, menuY + lineH, btnW, btnH, { 0.7f, 0.7f, 0.7f, 0.9f });
	}

	// 操作説明オーバーレイ
	if (g_State == TITLE_STATE_CONTROLS) {
		// 半透明黒オーバーレイ
		Sprite_Draw(g_WhiteTex, 0.0f, 0.0f, sw, sh, { 0.0f, 0.0f, 0.0f, 0.8f });

		// 操作説明テクスチャ（中央表示）
		float infoW = 550.0f;
		float infoH = 480.0f;
		Sprite_Draw(g_ControlsInfoTex,
			sw * 0.5f - infoW * 0.5f, sh * 0.5f - infoH * 0.5f,
			infoW, infoH, { 1.0f, 1.0f, 1.0f, 1.0f });
	}

	Direct3D_SetDepthEnable(true);
}
