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
#include "debug_text.h"
#include "key_logger.h"

static int g_GameClearBgTexId = -1;  // ゲームクリア背景画像
static int g_WhiteTex = -1;
static hal::DebugText* g_pBtnText = nullptr;

void GameClear_Initialize()
{
    g_GameClearBgTexId = Texture_Load(L"resource/texture/gameclear.png");
    g_WhiteTex = Texture_Load(L"resource/texture/white.png");

    float sw = (float)Direct3D_GetBackBufferWidth();
    float sh = (float)Direct3D_GetBackBufferHeight();

    g_pBtnText = new hal::DebugText(
        Direct3D_GetDevice(), Direct3D_GetContext(),
        L"resource/texture/consolab_ascii_512.png",
        (UINT)sw, (UINT)sh,
        sw * 0.5f - 65.0f, sh * 0.70f,
        2, 0, 45.0f, 20.0f
    );
}

void GameClear_Finalize()
{
    if (g_pBtnText) { delete g_pBtnText; g_pBtnText = nullptr; }
}

void GameClear_Update(double elapsed_time)
{
    // Enter でタイトルへ
    if (KeyLogger_IsTrigger(KK_ENTER)) {
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

    // Titleボタン
    float btnX = sw * 0.5f - 65.0f;
    float btnY = sh * 0.70f;

    // ボタン背景ハイライト
    Sprite_Draw(g_WhiteTex,
        btnX - 15.0f, btnY - 3.0f,
        200.0f, 38.0f,
        { 1.0f, 1.0f, 0.0f, 0.3f });

    // ボタンテキスト
    if (g_pBtnText) {
        g_pBtnText->Clear();
        g_pBtnText->SetText("> Title", { 1.0f, 1.0f, 1.0f, 1.0f });
        g_pBtnText->Draw();
    }

    Direct3D_SetDepthEnable(true);
}
