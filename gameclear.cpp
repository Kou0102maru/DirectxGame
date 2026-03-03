/*==============================================================================

   ゲームクリア画面 [gameclear.cpp]
                                                        Author :
                                                        Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "gameclear.h"
#include "scene.h"
#include "direct3d.h"
#include "sprite.h"
#include "texture.h"
#include "key_logger.h"
#include "pad_logger.h"

static int g_GameClearBgTexId = -1;
static int g_WhiteTex = -1;
static int g_BtnTitleTex = -1;

void GameClear_Initialize()
{
    g_GameClearBgTexId = Texture_Load(L"resource/texture/gameclear.png");
    g_WhiteTex = Texture_Load(L"resource/texture/white.png");
    g_BtnTitleTex = Texture_Load(L"resource/texture/btn_title.png");
}

void GameClear_Finalize()
{
}

void GameClear_Update(double elapsed_time)
{
    // Enter / Aボタン でタイトルへ
    if (KeyLogger_IsTrigger(KK_ENTER)
        || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_A)) {
        Scene_Change(SCENE_TITLE);
    }
}

void GameClear_Draw()
{
    Direct3D_SetBackBuffer();
    Direct3D_ClearBackBuffer();

    Direct3D_SetDepthEnable(false);
    Sprite_Begin();

    float sw = (float)Direct3D_GetBackBufferWidth();
    float sh = (float)Direct3D_GetBackBufferHeight();

    // ゲームクリア背景画像を全画面描画
    Sprite_Draw(g_GameClearBgTexId, 0.0f, 0.0f, sw, sh);

    // タイトルへボタン
    float btnW = 256.0f;
    float btnH = 64.0f;
    float btnX = sw * 0.5f - btnW * 0.5f;
    float btnY = sh * 0.68f;

    // ボタン背景ハイライト
    Sprite_Draw(g_WhiteTex,
        btnX - 10.0f, btnY - 5.0f,
        btnW + 20.0f, btnH + 10.0f,
        { 1.0f, 1.0f, 0.0f, 0.25f });

    // タイトルへボタン
    Sprite_Draw(g_BtnTitleTex, btnX, btnY,
        btnW, btnH, { 1.0f, 1.0f, 0.2f, 1.0f });

    Direct3D_SetDepthEnable(true);
}
