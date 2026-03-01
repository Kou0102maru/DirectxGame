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

static double g_Timer = 0.0;
static constexpr double GAMEOVER_DURATION = 3.0;

static hal::DebugText* g_pGameOverText = nullptr;
static int g_WhiteTexture = -1;

void GameOver_Initialize()
{
    g_Timer = GAMEOVER_DURATION;

    float screenW = (float)Direct3D_GetBackBufferWidth();
    float screenH = (float)Direct3D_GetBackBufferHeight();

    g_WhiteTexture = Texture_Load(L"resource/texture/white.png");

    auto* dev = Direct3D_GetDevice();
    auto* ctx = Direct3D_GetContext();
    const wchar_t* fontTex = L"resource/texture/consolab_ascii_512.png";

    const float CHAR_W = 20.0f;
    const float LINE_H = 36.0f;
    float textX = screenW * 0.5f - CHAR_W * 5.0f;
    float textY = screenH * 0.5f - LINE_H * 0.5f;

    delete g_pGameOverText;
    g_pGameOverText = new hal::DebugText(
        dev, ctx, fontTex,
        (UINT)screenW, (UINT)screenH,
        textX, textY,
        1, 0, LINE_H, CHAR_W
    );
}

void GameOver_Finalize()
{
    delete g_pGameOverText;
    g_pGameOverText = nullptr;
}

void GameOver_Update(double elapsed_time)
{
    g_Timer -= elapsed_time;
    if (g_Timer <= 0.0) {
        Scene_Change(SCENE_TITLE);
    }
}

void GameOver_Draw()
{
    Direct3D_SetBackBuffer();
    Direct3D_ClearBackBuffer();

    Direct3D_SetDepthEnable(false);
    Sprite_Begin();

    float screenW = (float)Direct3D_GetBackBufferWidth();
    float screenH = (float)Direct3D_GetBackBufferHeight();

    // 黒背景
    Sprite_Draw(g_WhiteTexture, 0, 0, screenW, screenH, { 0.0f, 0.0f, 0.0f, 1.0f });

    // "GAME OVER" テキスト（赤）
    if (g_pGameOverText) {
        g_pGameOverText->Clear();
        g_pGameOverText->SetText("GAME OVER", { 1.0f, 0.2f, 0.2f, 1.0f });
        g_pGameOverText->Draw();
    }

    Direct3D_SetDepthEnable(true);
}
