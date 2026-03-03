/*==============================================================================

   ゲームオーバー画面 [gameover.cpp]
                                                        Author :
                                                        Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "gameover.h"
#include "scene.h"
#include "direct3d.h"
#include "sprite.h"
#include "texture.h"
#include "debug_text.h"
#include "key_logger.h"
#include <cstdio>

static int g_GameOverBgTexId = -1;  // ゲームオーバー背景画像
static int g_WhiteTex = -1;
static int g_GOSelect = 0;  // 0=Retry, 1=Title
static hal::DebugText* g_pMenuText = nullptr;

void GameOver_Initialize()
{
    g_GameOverBgTexId = Texture_Load(L"resource/texture/gameover.png");
    g_WhiteTex = Texture_Load(L"resource/texture/white.png");
    g_GOSelect = 0;

    float sw = (float)Direct3D_GetBackBufferWidth();
    float sh = (float)Direct3D_GetBackBufferHeight();

    g_pMenuText = new hal::DebugText(
        Direct3D_GetDevice(), Direct3D_GetContext(),
        L"resource/texture/consolab_ascii_512.png",
        (UINT)sw, (UINT)sh,
        sw * 0.5f - 80.0f, sh * 0.65f,
        4, 0, 45.0f, 20.0f
    );
}

void GameOver_Finalize()
{
    if (g_pMenuText) { delete g_pMenuText; g_pMenuText = nullptr; }
}

void GameOver_Update(double elapsed_time)
{
    // 上下キーでメニュー選択
    if (KeyLogger_IsTrigger(KK_UP)) {
        g_GOSelect--;
        if (g_GOSelect < 0) g_GOSelect = 1;
    }
    if (KeyLogger_IsTrigger(KK_DOWN)) {
        g_GOSelect++;
        if (g_GOSelect > 1) g_GOSelect = 0;
    }
    // Enter で決定
    if (KeyLogger_IsTrigger(KK_ENTER)) {
        if (g_GOSelect == 0) {
            Scene_Change(SCENE_GAME);   // Retry
        } else {
            Scene_Change(SCENE_TITLE);  // Title
        }
    }
}

void GameOver_Draw()
{
    Direct3D_SetBackBuffer();
    Direct3D_ClearBackBuffer();

    Direct3D_SetDepthEnable(false);
    Sprite_Begin();

    float sw = (float)Direct3D_GetBackBufferWidth();
    float sh = (float)Direct3D_GetBackBufferHeight();

    // ゲームオーバー背景画像を全画面描画
    Sprite_Draw(g_GameOverBgTexId, 0.0f, 0.0f, sw, sh);

    // メニューボタン
    float menuX = sw * 0.5f - 80.0f;
    float menuY = sh * 0.65f;
    float btnW = 220.0f;
    float btnH = 38.0f;
    float lineH = 45.0f;

    // 選択中のボタン背景ハイライト
    Sprite_Draw(g_WhiteTex,
        menuX - 15.0f, menuY + g_GOSelect * lineH - 3.0f,
        btnW, btnH,
        { 1.0f, 1.0f, 0.0f, 0.3f });

    // メニューテキスト
    if (g_pMenuText) {
        g_pMenuText->Clear();
        char buf[64];
        snprintf(buf, sizeof(buf), "%c Retry\n%c Title",
            g_GOSelect == 0 ? '>' : ' ',
            g_GOSelect == 1 ? '>' : ' ');
        g_pMenuText->SetText(buf, { 1.0f, 1.0f, 1.0f, 1.0f });
        g_pMenuText->Draw();
    }

    Direct3D_SetDepthEnable(true);
}
