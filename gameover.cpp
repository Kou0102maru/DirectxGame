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
#include "key_logger.h"
#include "pad_logger.h"

static int g_GameOverBgTexId = -1;
static int g_WhiteTex = -1;
static int g_GOSelect = 0;  // 0=Retry, 1=Title
static int g_BtnRetryTex = -1;
static int g_BtnTitleTex = -1;

void GameOver_Initialize()
{
    g_GameOverBgTexId = Texture_Load(L"resource/texture/gameover.png");
    g_WhiteTex = Texture_Load(L"resource/texture/white.png");
    g_BtnRetryTex = Texture_Load(L"resource/texture/btn_retry.png");
    g_BtnTitleTex = Texture_Load(L"resource/texture/btn_title.png");
    g_GOSelect = 0;
}

void GameOver_Finalize()
{
}

void GameOver_Update(double elapsed_time)
{
    // 上下キー / D-pad でメニュー選択
    if (KeyLogger_IsTrigger(KK_UP) || KeyLogger_IsTrigger(KK_W)
        || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_DPAD_UP)) {
        g_GOSelect--;
        if (g_GOSelect < 0) g_GOSelect = 1;
    }
    if (KeyLogger_IsTrigger(KK_DOWN) || KeyLogger_IsTrigger(KK_S)
        || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_DPAD_DOWN)) {
        g_GOSelect++;
        if (g_GOSelect > 1) g_GOSelect = 0;
    }
    // Enter / Aボタン で決定
    if (KeyLogger_IsTrigger(KK_ENTER)
        || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_A)) {
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
    float btnW = 256.0f;
    float btnH = 64.0f;
    float lineH = 70.0f;
    float menuX = sw * 0.5f - btnW * 0.5f;
    float menuY = sh * 0.62f;

    // 選択中のボタン背景ハイライト
    Sprite_Draw(g_WhiteTex,
        menuX - 10.0f, menuY + g_GOSelect * lineH - 5.0f,
        btnW + 20.0f, btnH + 10.0f,
        { 1.0f, 1.0f, 0.0f, 0.25f });

    // リトライボタン（選択中=黄色、非選択=薄白）
    if (g_GOSelect == 0)
        Sprite_Draw(g_BtnRetryTex, menuX, menuY, btnW, btnH, { 1.0f, 1.0f, 0.2f, 1.0f });
    else
        Sprite_Draw(g_BtnRetryTex, menuX, menuY, btnW, btnH, { 0.7f, 0.7f, 0.7f, 0.8f });

    // タイトルへボタン
    if (g_GOSelect == 1)
        Sprite_Draw(g_BtnTitleTex, menuX, menuY + lineH, btnW, btnH, { 1.0f, 1.0f, 0.2f, 1.0f });
    else
        Sprite_Draw(g_BtnTitleTex, menuX, menuY + lineH, btnW, btnH, { 0.7f, 0.7f, 0.7f, 0.8f });

    Direct3D_SetDepthEnable(true);
}
