/*==============================================================================

   戦闘シーン [battle.cpp]
														 Author :
														 Date   : 2025/xx/xx
--------------------------------------------------------------------------------

==============================================================================*/
#include "battle.h"
#include"sprite.h"
#include"monster.h"
#include "scene.h"
#include "key_logger.h"
#include "direct3d.h"
#include "bullet.h"
#include "collision.h"
#include "player.h"
#include "cube.h"
#include "camera.h"
#include "light.h"
#include "sampler.h"
#include "player_camera.h"
#include"cirel_shadow.h"
#include <DirectXMath.h>
#include "particle_test.h"
#include "billboard.h"
using namespace DirectX;

// 戦闘相手のモンスター
static Monster* g_pBattleEnemy = nullptr;
static Monster* g_pOriginalEnemy = nullptr;

// ★モンスター情報のコピー★
static int g_EnemyHp = 0;
static int g_EnemyHpMax = 0;
static MonsterKind g_EnemyKind = MONSTER_KIND_SLIME;

// モンスターの戦闘用位置（固定）
static constexpr float ENEMY_BATTLE_X = 5.0f;
static constexpr float ENEMY_BATTLE_Y = 0.5f;
static constexpr float ENEMY_BATTLE_Z = 0.0f;

// プレイヤーの戦闘用位置（固定）
static constexpr float PLAYER_BATTLE_X = -5.0f;
static constexpr float PLAYER_BATTLE_Y = 0.5f;
static constexpr float PLAYER_BATTLE_Z = 0.0f;

struct BattleBullet {
	float x;
	bool active;
	double lifetime;
};
static BattleBullet g_BattleBullets[10]{};

static double g_ShootCooldown = 0.0;
static constexpr double SHOOT_INTERVAL = 0.5;

static NormalEmitter* g_HitEffect = nullptr;

static int g_WhiteTexture = -1;

XMFLOAT3 g_PlayerSavePosition;

void Battle_SetEnemy(Monster* enemy)
{
	g_pBattleEnemy = enemy;
	g_pOriginalEnemy = enemy;

	g_EnemyHp = enemy->GetHp();
	g_EnemyHpMax = enemy->GetHpMax();
	g_EnemyKind = enemy->GetKind();
}

void Battle_Initialize()
{
	// ★モンスター情報がコピーされてなかったらゲームシーンに戻る★
	if (g_EnemyHp <= 0 && g_EnemyHpMax <= 0) {
		Scene_Change(SCENE_GAME);
		return;
	}

	g_PlayerSavePosition = Player_GetPosition();

	// 戦闘用カメラを初期化（真横から見る）
	Camera_Initialize(
		{ 0.0f, 2.0f, -10.0f },  // カメラ位置（中央、少し上、手前）
		{ 0.0f, 0.0f, 1.0f },    // 前向き（奥を向く）
		{ 1.0f, 0.0f, 0.0f }     // 右向き
	);

	// 戦闘用にプレイヤーと敵の位置を固定
	Player_SetPosition({ PLAYER_BATTLE_X, PLAYER_BATTLE_Y, PLAYER_BATTLE_Z });
	//g_pBattleEnemy->SetPosition({ ENEMY_BATTLE_X, ENEMY_BATTLE_Y, ENEMY_BATTLE_Z });

	// ★弾を全削除（Finalize/Initializeは使わない）★
	for (int i = Bullet_GetBulletsCount() - 1; i >= 0; i--) {
		Bullet_Destroy(i);
	}

	g_WhiteTexture = Texture_Load(L"resource/texture/white.png");

	Billboard_Initialize();

	g_HitEffect = new NormalEmitter(500, { 2.0f, 0.5f, 5.0f }, 50.0, false);
}

void Battle_Finalize()
{
	Camera_Finalize();

	delete g_HitEffect;
	g_HitEffect = nullptr;

	Billboard_Finalize();

	Player_SetPosition(g_PlayerSavePosition);

	// ★戦闘したモンスターを削除★
	if (g_pOriginalEnemy) {
		Monster_Remove(g_pOriginalEnemy);  // ←この関数が必要
		g_pOriginalEnemy = nullptr;
	}

	g_pBattleEnemy = nullptr;
}

void Battle_Update(double elapsed_time)
{
	// プレイヤー更新（移動は制限）
	Player_Update(elapsed_time);

	// プレイヤー位置を固定（動けないように）
	Player_SetPosition({ PLAYER_BATTLE_X, PLAYER_BATTLE_Y, PLAYER_BATTLE_Z });

	g_ShootCooldown -= elapsed_time;
	if (g_ShootCooldown < 0.0) g_ShootCooldown = 0.0;

	if (KeyLogger_IsTrigger(KK_SPACE) && g_ShootCooldown <= 0.0) {
		// 空いてるスロットを探し
		int slot = -1;
		for (int i = 0; i < 10; i++) {
			if (!g_BattleBullets[i].active) {
				slot = i;
				break;
			}
		}

		// 弾を追加
		if(slot >= 0) {
			g_BattleBullets[slot].x = -2.0f;  // プレイヤー位置
			g_BattleBullets[slot].active = true;
			g_BattleBullets[slot].lifetime = 0.0;
		}
		g_ShootCooldown = SHOOT_INTERVAL;
	}

	// 弾を移動
	for (int i = 0; i < 10; i++) {
		if (g_BattleBullets[i].active) {
			g_BattleBullets[i].x += 10.0f * (float)elapsed_time;
			g_BattleBullets[i].lifetime += elapsed_time;

			// ★3秒経過したら自動削除★
			if (g_BattleBullets[i].lifetime > 3.0) {
				g_BattleBullets[i].active = false;
				continue;
			}

			// 当たり判定
			if (g_EnemyHp > 0 && g_BattleBullets[i].x >= 1.5f && g_BattleBullets[i].x <= 2.5f) {
				g_EnemyHp -= 10;
				if (g_EnemyHp < 0) g_EnemyHp = 0;
				g_BattleBullets[i].active = false;

				// ★ヒットパーティクル発生（位置を設定するだけ）★
				if (g_HitEffect) {// ★3秒経過したら自動削除★
			if (g_BattleBullets[i].lifetime > 3.0) {
				g_BattleBullets[i].active = false;
				continue;
			}
					g_HitEffect->SetPosition({ 2.0f, 0.5f, 5.0f });  // モンスターの位置
				}
			}

			// 画面外
			if (g_BattleBullets[i].x > 5.0f) {
				g_BattleBullets[i].active = false;
			}
		}
	}

	// ★パーティクル更新★
	if (g_HitEffect) {
		g_HitEffect->Update(elapsed_time);
	}

	// モンスターを倒したらゲームシーンに戻る
	if (g_EnemyHp <= 0) {
		Scene_Change(SCENE_GAME);
	}

	// デバッグ：Mキーで強制終了
	if (KeyLogger_IsTrigger(KK_M)) {
		Scene_Change(SCENE_GAME);
	}
}

void Battle_Draw()
{
	Direct3D_SetBackBuffer();
	Direct3D_ClearBackBuffer();

	Direct3D_SetDepthEnable(true);

	// カメラ設定（PlayerCameraを使う）
	XMFLOAT4X4 mtxView = Camera_GetMatrix();
	Billboard_SetViewMatrix(mtxView);
	XMFLOAT4X4 mtxProj = PlayerCamera_GetPerspectiveMatrix();
	XMMATRIX view = XMLoadFloat4x4(&mtxView);
	XMMATRIX proj = XMLoadFloat4x4(&mtxProj);
	Camera_SetMatrix(view, proj);

	Sampler_SetFilterAnisotropic();

	// ライト設定
	Light_SetAmbient({ 1.0f, 1.0f, 1.0f });
	XMVECTOR v{ 0.0f, -1.0f, 0.0f };
	v = XMVector3Normalize(v);
	XMFLOAT4 dir;
	XMStoreFloat4(&dir, v);
	Light_SetDirectionalWorld(dir, { 1.0f, 1.0f, 1.0f, 1.0f });

	extern int g_MonsterTexSlime;
	extern int g_MonsterTexWolf;
	extern int g_MonsterTexDragon;

	//プレイヤー
	XMMATRIX worldPlayer = XMMatrixTranslation(-2.0f, 0.5f, 5.0f);  
	Cube_Draw(g_MonsterTexSlime, worldPlayer);

	//モンスター
	XMMATRIX worldEnemy = XMMatrixTranslation(2.0f, 0.5f, 5.0f);  
	Cube_Draw(g_MonsterTexWolf, worldEnemy);

	// ★ここから2D描画（HPバー）★
	Direct3D_SetDepthEnable(false);
	//Direct3D_SetAlphaBlendTransparent();
	Sprite_Begin();


	float bar_width = 200.0f;
	float bar_height = 15.0f;


	// ★プレイヤーのHPバー
	float player_hp_ratio = 1.0f;
	float player_bar_x = Direct3D_GetBackBufferWidth() * 0.25f - bar_width / 2;
	float player_bar_y = Direct3D_GetBackBufferHeight() * 0.75f;

	// HP枠
	Sprite_Draw(g_WhiteTexture, player_bar_x - 2, player_bar_y - 2, bar_width + 4, bar_height + 4, { 1.0f, 1.0f, 1.0f, 1.0f });
	// HP背景
	Sprite_Draw(g_WhiteTexture, player_bar_x, player_bar_y, bar_width, bar_height, { 0.0f, 0.0f, 0.0f, 1.0f });
	// HP
	Sprite_Draw(g_WhiteTexture, player_bar_x, player_bar_y, bar_width * player_hp_ratio, bar_height, { 0.0f, 1.0f, 0.0f, 1.0f });

	// モンスターのHPバー
	float enemy_hp_ratio = (float)g_EnemyHp / (float)g_EnemyHpMax;
	float enemy_bar_x = Direct3D_GetBackBufferWidth() * 0.75f - bar_width / 2;
	float enemy_bar_y = Direct3D_GetBackBufferHeight() * 0.75f;
	
	// HP枠
	Sprite_Draw(g_WhiteTexture, enemy_bar_x - 2, enemy_bar_y - 2, bar_width + 4, bar_height + 4, { 1.0f, 1.0f, 1.0f, 1.0f });
	// HP背景
	Sprite_Draw(g_WhiteTexture, enemy_bar_x, enemy_bar_y, bar_width, bar_height, { 0.1f, 0.1f, 0.1f, 1.0f });
	// HP
	XMFLOAT4 hp_color = { 0.0f, 1.0f, 0.0f, 1.0f };  // 緑
	if (enemy_hp_ratio <= 0.5f) hp_color = { 1.0f, 1.0f, 0.0f, 1.0f };  // 黄色
	if (enemy_hp_ratio <= 0.25f) hp_color = { 1.0f, 0.0f, 0.0f, 1.0f };  // 赤

	if (g_EnemyHp > 0) {
		Sprite_Draw(g_WhiteTexture, enemy_bar_x, enemy_bar_y, bar_width * enemy_hp_ratio, bar_height, hp_color);
	}

	Direct3D_SetDepthEnable(true);


	for (int i = 0; i < 10; i++) {
		if (g_BattleBullets[i].active) {
			XMMATRIX scale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
			XMMATRIX trans = XMMatrixTranslation(g_BattleBullets[i].x, 0.5f, 5.0f);
			Cube_Draw(g_MonsterTexDragon, scale * trans);
		}
	}

		// ★パーティクル描画を追加★
		if (g_HitEffect) {
			Direct3D_SetDepthWriteDisable();
			Direct3D_SetAlphaBlendAdd();
			g_HitEffect->Draw();
			Direct3D_SetDepthEnable(true);
			Direct3D_SetAlphaBlendTransparent();  // 元に戻す
		}
}