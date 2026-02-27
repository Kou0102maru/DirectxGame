/*==============================================================================

   âÊñ ëJà⁄êßå‰ [scene.h]
														 Author : 
														 Date   : 2025/07/10
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SCENE_H
#define SCENE_H


void Scene_Initialize();
void Scene_Finalize();
void Scene_Update(double elapsed_time);
void Scene_Draw();
void Scene_Refresh();

enum Scene
{
	SCENE_TITLE,
	SCENE_GAME,
	SCENE_BATTLE,  
	SCENE_RESULT,
	SCENE_MAX
};

extern bool g_ReturnFromBattle;

void Scene_Change(Scene scene);


#endif // SCENE_H
