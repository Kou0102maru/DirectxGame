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

static double g_Timer = 0.0;
static constexpr double GAMECLEAR_DURATION = 5.0;

static hal::DebugText* g_pGameClearText = nullptr;
static int g_WhiteTexture = -1;

void GameClear_Initialize()
{
    g_Timer = GAMECLEAR_DURATION;

    float screenW = (float)Direct3D_GetBackBufferWidth();
    float screenH = (float)Direct3D_GetBackBufferHeight();

    g_WhiteTexture = Texture_Load(L"resource/texture/white.png");

    auto* dev = Direct3D_GetDevice();
    auto* ctx = Direct3D_GetContext();
    const wchar_t* fontTex = L"resource/texture/consolab_ascii_512.png";

    const float CHAR_W = 20.0f;
    const float LINE_H = 36.0f;
    float textX = screenW * 0.5f - CHAR_W * 6.0f;
    float textY = screenH * 0.5f - LINE_H * 0.5f;

    delete g_pGameClearText;
    g_pGameClearText = new hal::DebugText(
        dev, ctx, fontTex,
        (UINT)screenW, (UINT)screenH,
        textX, textY,
        1, 0, LINE_H, CHAR_W
    );
}

void GameClear_Finalize()
{
    delete g_pGameClearText;
    g_pGameClearText = nullptr;
}

void GameClear_Update(double elapsed_time)
{
    g_Timer -= elapsed_time;
    if (g_Timer <= 0.0) {
        Scene_Change(SCENE_TITLE);
    }
}

void GameClear_Draw()
{
    Direct3D_SetBackBuffer();
    Direct3D_ClearBackBuffer();

    Direct3D_SetDepthEnable(false);
    Sprite_Begin();

    float screenW = (float)Direct3D_GetBackBufferWidth();
    float screenH = (float)Direct3D_GetBackBufferHeight();

    // 黒背景
    Sprite_Draw(g_WhiteTexture, 0, 0, screenW, screenH, { 0.0f, 0.0f, 0.0f, 1.0f });

    // "GAME CLEAR!" テキスト（黄色）
    if (g_pGameClearText) {
        g_pGameClearText->Clear();
        g_pGameClearText->SetText("GAME CLEAR!", { 1.0f, 1.0f, 0.0f, 1.0f });
        g_pGameClearText->Draw();
    }

    Direct3D_SetDepthEnable(true);
}
