/*==============================================================================

   シーン管理 [scene.cpp]
														 Author : Youhei Sato
														 Date   : 2025/07/11
--------------------------------------------------------------------------------

==============================================================================*/
#include "scene.h"
#include "title.h"
#include "game.h"
#include "battle.h"
#include "gameover.h"
#include "gameclear.h"


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
	case SCENE_BATTLE:  // バトル追加
		Battle_Initialize();
		break;
	case SCENE_RESULT:
		break;
	case SCENE_GAMEOVER:
		GameOver_Initialize();
		break;
	case SCENE_GAMECLEAR:
		GameClear_Initialize();
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
	case SCENE_BATTLE:  // バトル追加
		Battle_Finalize();
		break;
	case SCENE_RESULT:
		break;
	case SCENE_GAMEOVER:
		GameOver_Finalize();
		break;
	case SCENE_GAMECLEAR:
		GameClear_Finalize();
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
	case SCENE_BATTLE:  // バトル追加
		Battle_Update(elapsed_time);
		break;
	case SCENE_RESULT:
		break;
	case SCENE_GAMEOVER:
		GameOver_Update(elapsed_time);
		break;
	case SCENE_GAMECLEAR:
		GameClear_Update(elapsed_time);
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
	case SCENE_BATTLE:  // バトル追加
		Battle_Draw();
		break;
	case SCENE_RESULT:
		break;
	case SCENE_GAMEOVER:
		GameOver_Draw();
		break;
	case SCENE_GAMECLEAR:
		GameClear_Draw();
		break;
	default:
		break;
	}
}

void Scene_Refresh()
{
	if (g_Scene != g_SceneNext) {
		if (g_Scene == SCENE_GAME && g_SceneNext == SCENE_BATTLE) {
			g_Scene = g_SceneNext;
			Battle_Initialize();
		}
		else if (g_Scene == SCENE_BATTLE && g_SceneNext == SCENE_GAME) {
			Battle_Finalize();
			g_Scene = g_SceneNext;
		}
		else {
			Scene_Finalize();
			g_Scene = g_SceneNext;
			Scene_Initialize();
		}
	}
}

void Scene_Change(Scene scene)
{
	g_SceneNext = scene;
}
