/*==============================================================================

   画面遷移制御 [scene.cpp]
														 Author : Youhei Sato
														 Date   : 2025/07/11
--------------------------------------------------------------------------------

==============================================================================*/
#include "scene.h"
#include "title.h"
#include "game.h"
#include "battle.h"  // ★追加★


// static Scene g_Scene = SCENE_TITLE;
static Scene g_Scene = SCENE_GAME;
static Scene g_SceneNext = g_Scene;

bool g_ReturnFromBattle = false;

void Scene_Initialize()
{
	switch (g_Scene)
	{
	case SCENE_TITLE:
		Title_Initialize();
		break;
	case SCENE_GAME:
		Game_Initialize();
		break;
	case SCENE_BATTLE:  // ★追加★
		Battle_Initialize();
		break;
	case SCENE_RESULT:
		// リザルトシーンは未実装
		break;
	default:
		break;
	}
}

void Scene_Finalize()
{
	switch (g_Scene)
	{
	case SCENE_TITLE:
		Title_Finalize();
		break;
	case SCENE_GAME:
		Game_Finalize();
		break;
	case SCENE_BATTLE:  // ★追加★
		Battle_Finalize();
		break;
	case SCENE_RESULT:
		// リザルトシーンは未実装
		break;
	default:
		break;
	}
}

void Scene_Update(double elapsed_time)
{
	switch (g_Scene)
	{
	case SCENE_TITLE:
		Title_Update(elapsed_time);
		break;
	case SCENE_GAME:
		Game_Update(elapsed_time);
		break;
	case SCENE_BATTLE:  // ★追加★
		Battle_Update(elapsed_time);
		break;
	case SCENE_RESULT:
		// リザルトシーンは未実装
		break;
	default:
		break;
	}
}

void Scene_Draw()
{
	switch (g_Scene)
	{
	case SCENE_TITLE:
		Title_Draw();
		break;
	case SCENE_GAME:
		Game_Draw();
		break;
	case SCENE_BATTLE:  // ★追加★
		Battle_Draw();
		break;
	case SCENE_RESULT:
		// リザルトシーンは未実装
		break;
	default:
		break;
	}
}

void Scene_Refresh()
{
	if (g_Scene != g_SceneNext) {
		Scene_Finalize();
		g_Scene = g_SceneNext;
		Scene_Initialize();
	}
}

void Scene_Change(Scene scene)
{
	g_SceneNext = scene;
}
