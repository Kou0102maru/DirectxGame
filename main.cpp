/*==============================================================================
    ウィンドウ表示 [main.cpp]
                                                           Author : Youhei Sato
                                                               Date : 2025/4/21
--------------------------------------------------------------------------------
==============================================================================*/
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "game_window.h"
#include "direct3d.h"
#include "shader.h"
#include "shader3d.h"
#include "shader3d_unlit.h"
#include"shader_depth.h"
#include "sampler.h"
#include "texture.h"
#include "sprite.h"
#include "sprite_anim.h"
#include "fade.h"
#include "collision.h"
#include "debug_text.h"
#include <sstream>
#include "system_timer.h"
#include "key_logger.h"
#include "pad_logger.h"
#include "mouse.h"
#include "Audio.h"
#include "light.h"

#include "scene.h"
#include "cube.h"
#include "grid.h"
#include "meshfield.h"


/*------------------------------------------------------------------------------
    メイン
------------------------------------------------------------------------------*/
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int nCmdShow)
{
    (void)CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    // DPIスケーリング
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    HWND hWnd = GameWindow_Create(hInstance);
   
    SystemTimer_Initialize();
    KeyLogger_Initialize();
    PadLogger_Initialize();
    Mouse_Initialize(hWnd);
    InitAudio();

    Direct3D_Initialize(hWnd);
    Shader_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
    Shader3d_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
    Shader3dUnlit_Initialize();
    ShaderDepth_Initialize();
    Sampler_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
    Texture_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
    Sprite_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
    SpriteAnim_Initialize();
    Fade_Initialize();
    Scene_Initialize();
    Grid_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
    Cube_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
    Light_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
    MeshField_Initialize(Direct3D_GetDevice(), Direct3D_GetContext());
    
   //  Mouse_SetVisible(false);

#if defined(DEBUG) || defined(_DEBUG)  
    hal::DebugText dt(Direct3D_GetDevice(), Direct3D_GetContext(),
        L"resource/texture/consolab_ascii_512.png",
        Direct3D_GetBackBufferWidth(), Direct3D_GetBackBufferHeight(),
        0.0f, 0.0f, 
        0, 0, 
        0.0f, 14.0f);

    Collision_DebugInitialize(Direct3D_GetDevice(), Direct3D_GetContext());
#endif 

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // fps・実行フレーム速度計測用
    double exec_last_time = SystemTimer_GetTime();
    double fps_last_time = exec_last_time;
    double current_time = 0.0;
    ULONG frame_count = 0;
    double fps = 0.0;

    /* メッセージ & ゲームループ */
    MSG msg;

    do {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { // ウィンドウメッセージが来ていたら
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else { // ゲームの処理
            // fps計測
            current_time = SystemTimer_GetTime(); // システム時刻を取得
            double elapsed_time = current_time - fps_last_time; // fps計測用の経過時間を計算

            if (elapsed_time >= 1.0) // 1秒ごとに計測
            {
                fps = frame_count / elapsed_time;
                fps_last_time = current_time; // FPSを測定した時刻を保存
                frame_count = 0; // カウントをクリア
            }

            // 1/60秒ごとに実行

           // elapsed_time += SystemTimer_GetElapsedTime();
           // if (elapsed_time >= 1.0 / 15.0) {
           //     elapsed_time = 1.0 / 15.0;
           // }

           elapsed_time = current_time - exec_last_time;
           if (elapsed_time >= (1.0 / 60.0)) {
               exec_last_time = current_time; // 処理した時刻を保存

               if (elapsed_time > 1.0 / 30.0) {
                   elapsed_time = 1.0 / 30.0;
               }
           }

            {
                // ゲームの更新
                KeyLogger_Update();
                PadLogger_Update();
                Scene_Update(elapsed_time);
                SpriteAnim_Update(elapsed_time);
                Fade_Update(elapsed_time);

                // ゲームの描画
                Direct3D_SetBackBuffer();
                Direct3D_ClearBackBuffer();
                Scene_Draw();
                Fade_Draw();

#if defined(DEBUG) || defined(_DEBUG)      
                std::stringstream ss;
                //ss << "fps:" << fps << std::endl;
                ss << "elapsed time:" << elapsed_time << std::endl;
                dt.SetText(ss.str().c_str(), { 1.0f, 1.0f, 0.0f, 1.0f });
                dt.Draw();
                dt.Clear();
#endif
                Direct3D_Present();

                Scene_Refresh();

                frame_count++;
            }
        }

    } while (msg.message != WM_QUIT);

#if defined(DEBUG) || defined(_DEBUG)
    Collision_DebugFinalize();
#endif

    MeshField_Finalize();
    Grid_Finalize();
    Light_Finalize();
    Cube_Finalize();
    Scene_Finalize();

    Fade_Finalize();
    SpriteAnim_Finalize();
    Sprite_Finalize();
    Texture_Finalize();
    Sampler_Finalize();
    ShaderDepth_Finalize();
    Shader3dUnlit_Finalize();
    Shader3d_Finalize();
    Shader_Finalize();
    Direct3D_Finalize();

    UninitAudio();
    Mouse_Finalize();

    return (int)msg.wParam;
}
