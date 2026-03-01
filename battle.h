/*==============================================================================

   戦闘シーン [battle.h]
                                                         Author : 
                                                         Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef BATTLE_H
#define BATTLE_H

#include "monster.h"

// 戦闘開始（エンカウントしたモンスターを設定）
void Battle_SetEnemy(Monster* enemy);
void Battle_SetBossFlag(bool is_boss);

void Battle_Initialize();
void Battle_Finalize();
void Battle_Update(double elapsed_time);
void Battle_Draw();

extern DirectX::XMFLOAT3 g_PlayerSavePosition;
extern bool g_BossDefeated;

#endif // BATTLE_H
