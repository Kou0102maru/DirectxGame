/*==============================================================================

   バトルシーン [battle.cpp]
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
#include "debug_text.h"
#include "model.h"
#include "party.h"
#include "pad_logger.h"
#include "text_texture.h"
#include <cstdio>
#include <cstdlib>
using namespace DirectX;

// バトル用モンスター
static Monster* g_pBattleEnemy = nullptr;
static Monster* g_pOriginalEnemy = nullptr;

// モンスターステータスのコピー
static int g_EnemyHp = 0;
static int g_EnemyHpMax = 0;
static int g_EnemyLevel = 1;
static MonsterKind g_EnemyKind = MONSTER_KIND_SPIDER;

// モンスター配置（右）
static constexpr float ENEMY_BATTLE_X = 5.0f;
static constexpr float ENEMY_BATTLE_Y = 0.5f;
static constexpr float ENEMY_BATTLE_Z = 0.0f;

// プレイヤー配置（左）
static constexpr float PLAYER_BATTLE_X = -5.0f;
static constexpr float PLAYER_BATTLE_Y = 0.5f;
static constexpr float PLAYER_BATTLE_Z = 0.0f;

// HPバーサイズ（Draw と Initialize で共有）
static constexpr float BATTLE_BAR_WIDTH  = 200.0f;
static constexpr float BATTLE_BAR_HEIGHT = 15.0f;

// HPバー位置（Initialize で計算・設定）
static float g_PlayerBarX = 0.0f;
static float g_PlayerBarY = 0.0f;
static float g_EnemyBarX  = 0.0f;
static float g_EnemyBarY  = 0.0f;

struct BattleBullet {
	float x;
	bool active;
	double lifetime;
};
static BattleBullet g_BattleBullets[10]{};

struct EnemyBullet {
	float x;
	bool active;
	double lifetime;
};
static EnemyBullet g_EnemyBullets[5]{};
static double g_EnemyAttackTimer = 0.0;
static int g_EnemyAtk = 0;
static int g_EnemyDef = 0;
static int g_EnemyExpReward = 0;

// 敵攻撃パターン用
static double g_EnemyAttackInterval = 2.0;
static float  g_EnemyBulletSpeed = 8.0f;
static int    g_EnemyBurstCount = 1;
static int    g_EnemyBurstRemaining = 0;
static double g_EnemyBurstDelay = 0.3;
static double g_EnemyBurstTimer = 0.0;

static double g_ShootCooldown = 0.0;
static constexpr double SHOOT_INTERVAL = 0.0;  // クールタイムなし（連打で連射）
static bool g_PrevRT = false;  // RTの前フレーム状態（連打検出用）

static NormalEmitter* g_HitEffect = nullptr;

static int g_WhiteTexture = -1;
static int g_BattleBgTex = -1;

XMFLOAT3 g_PlayerSavePosition;

// ボスフラグ
static bool g_IsBossBattle = false;
bool g_BossDefeated = false;

// バトル用3Dモデル（一度ロードしたら保持）
static MODEL* g_pBattleSpiderModel = nullptr;
static MODEL* g_pBattleWolfModel   = nullptr;
static MODEL* g_pBattleDragonModel = nullptr;
static MODEL* g_pBattleRobotModel  = nullptr;
static MODEL* g_pBattleEyeballModel = nullptr;
static MODEL* g_pBattlePlayerModel  = nullptr;  // プレイヤー用スライムモデル
// スケール定数（モデルサイズに合わせて調整可）
static constexpr float SPIDER_MODEL_SCALE  = 0.15f;
static constexpr float WOLF_MODEL_SCALE    = 3.0f;
static constexpr float DRAGON_MODEL_SCALE  = 0.1f;
static constexpr float ROBOT_MODEL_SCALE   = 0.3f;
static constexpr float EYEBALL_MODEL_SCALE = 0.3f;

// テキスト描画オブジェクト（HP数値のみ DebugText を維持）
static hal::DebugText* g_pPlayerHpNumText  = nullptr;  // HP数値

// バトルフェーズ管理
enum BattlePhase { PHASE_BATTLE = 0, PHASE_WIN, PHASE_LOSE, PHASE_SWAP };
static BattlePhase     g_BattlePhase   = PHASE_BATTLE;
static double          g_ResultTimer   = 0.0;
static int             g_LastExpReward = 0;
static bool            g_CapturedThisBattle = false;  // 仲間になったか

// 仲間確率（%）
static int GetCaptureRate(MonsterKind kind)
{
	switch (kind) {
	case MONSTER_KIND_SPIDER:  return 30;
	case MONSTER_KIND_WOLF:   return 20;
	case MONSTER_KIND_DRAGON: return 0;   // ラスボスは捕獲不可
	case MONSTER_KIND_ROBOT:  return 15;
	case MONSTER_KIND_EYEBALL: return 25;
	default:                  return 0;
	}
}

// モンスター名 -> バトル表示用日本語名
static const char* GetEnemyDisplayName(MonsterKind kind)
{
	switch (kind) {
	case MONSTER_KIND_SPIDER:  return "スパイダー";
	case MONSTER_KIND_WOLF:   return "オオカミ";
	case MONSTER_KIND_DRAGON: return "ドラゴン";
	case MONSTER_KIND_ROBOT:  return "ロボット";
	case MONSTER_KIND_EYEBALL: return "目玉";
	default:                  return "???";
	}
}

void Battle_SetBossFlag(bool is_boss)
{
	g_IsBossBattle = is_boss;
	g_BossDefeated = false;
}

void Battle_SetEnemy(Monster* enemy)
{
	g_pBattleEnemy = enemy;
	g_pOriginalEnemy = enemy;

	g_EnemyHp    = enemy->GetHp();
	g_EnemyHpMax = enemy->GetHpMax();
	g_EnemyLevel = enemy->GetLevel();
	g_EnemyKind  = enemy->GetKind();
	g_EnemyAtk   = enemy->GetAtk();
	g_EnemyDef   = enemy->GetDef();

	// 獲得EXP（base_exp x レベル係数、高レベルほど多い）
	const MonsterBaseParam* base = Monster_GetBaseParam(g_EnemyKind);
	float exp_growth = 1.0f + (enemy->GetLevel() - 1) * 0.2f;
	g_EnemyExpReward = (int)(base->base_exp * exp_growth);

	// 敵攻撃パターン設定
	switch (g_EnemyKind) {
	case MONSTER_KIND_SPIDER:
		g_EnemyAttackInterval = 1.2;
		g_EnemyBulletSpeed = 6.0f;
		g_EnemyBurstCount = 1;
		break;
	case MONSTER_KIND_WOLF:
		g_EnemyAttackInterval = 2.5;
		g_EnemyBulletSpeed = 8.0f;
		g_EnemyBurstCount = 2;
		break;
	case MONSTER_KIND_DRAGON:
		g_EnemyAttackInterval = 3.5;
		g_EnemyBulletSpeed = 10.0f;
		g_EnemyBurstCount = 1;
		break;
	case MONSTER_KIND_ROBOT:
		g_EnemyAttackInterval = 1.8;
		g_EnemyBulletSpeed = 9.0f;
		g_EnemyBurstCount = 2;
		break;
	case MONSTER_KIND_EYEBALL:
		g_EnemyAttackInterval = 1.5;
		g_EnemyBulletSpeed = 7.0f;
		g_EnemyBurstCount = 1;
		break;
	default:
		g_EnemyAttackInterval = 2.0;
		g_EnemyBulletSpeed = 8.0f;
		g_EnemyBurstCount = 1;
		break;
	}
}

void Battle_Initialize()
{
	// モンスターのコピーが無い場合ゲームシーンへ戻る
	if (g_EnemyHp <= 0 && g_EnemyHpMax <= 0) {
		Scene_Change(SCENE_GAME);
		return;
	}

	g_PlayerSavePosition = Player_GetPosition();

	// 専用カメラの初期化（固定カメラ）
	Camera_Initialize(
		{ 0.0f, 2.0f, -10.0f },  // カメラ位置（上、後、前）
		{ 0.0f, 0.0f, 1.0f },    // 前方向（Z+方向）
		{ 1.0f, 0.0f, 0.0f }     // 右方向
	);

	// 専用プレイヤーと敵の配置
	Player_SetPosition({ PLAYER_BATTLE_X, PLAYER_BATTLE_Y, PLAYER_BATTLE_Z });
	//g_pBattleEnemy->SetPosition({ ENEMY_BATTLE_X, ENEMY_BATTLE_Y, ENEMY_BATTLE_Z });

	// 弾の全消去（Finalize/Initialize を使わず）
	for (int i = Bullet_GetBulletsCount() - 1; i >= 0; i--) {
		Bullet_Destroy(i);
	}

	if (g_WhiteTexture < 0) {
		g_WhiteTexture = Texture_Load(L"resource/texture/white.png");
	}
	if (g_BattleBgTex < 0) {
		g_BattleBgTex = Texture_Load(L"resource/texture/battlehaikei.png");
	}

	// 3Dモデルのロード（スケール変更時は再ロードのため解放→再読込）
	if (g_pBattleSpiderModel) { ModelRelease(g_pBattleSpiderModel); g_pBattleSpiderModel = nullptr; }
	if (g_pBattleWolfModel) { ModelRelease(g_pBattleWolfModel); g_pBattleWolfModel = nullptr; }
	if (g_pBattleDragonModel) { ModelRelease(g_pBattleDragonModel); g_pBattleDragonModel = nullptr; }
	if (g_pBattleRobotModel) { ModelRelease(g_pBattleRobotModel); g_pBattleRobotModel = nullptr; }
	if (g_pBattleEyeballModel) { ModelRelease(g_pBattleEyeballModel); g_pBattleEyeballModel = nullptr; }
	if (g_pBattlePlayerModel) { ModelRelease(g_pBattlePlayerModel); g_pBattlePlayerModel = nullptr; }
	g_pBattleSpiderModel  = ModelLoad("resource/model/sp.fbx", SPIDER_MODEL_SCALE, false);
	g_pBattleWolfModel    = ModelLoad("resource/model/Wolf.fbx", WOLF_MODEL_SCALE, true);
	g_pBattleDragonModel  = ModelLoad("resource/model/Dragon.fbx", DRAGON_MODEL_SCALE, true);
	g_pBattleRobotModel   = ModelLoad("resource/model/robot.fbx", ROBOT_MODEL_SCALE, true);
	g_pBattleEyeballModel = ModelLoad("resource/model/eyeball.fbx", EYEBALL_MODEL_SCALE, true);
	g_pBattlePlayerModel  = ModelLoad("resource/model/slime.fbx", 0.65f);

	for (int i = 0; i < 10; i++) g_BattleBullets[i].active = false;
	for (int i = 0; i < 5;  i++) g_EnemyBullets[i].active = false;
	g_ShootCooldown = 0.0;
	g_PrevRT = false;
	g_EnemyAttackTimer = g_EnemyAttackInterval;
	g_EnemyBurstRemaining = 0;
	g_EnemyBurstTimer = 0.0;

	g_HitEffect = new NormalEmitter(500, { 2.0f, 0.0f, 5.0f }, 50.0, false);

	// フェーズリセット
	g_BattlePhase   = PHASE_BATTLE;
	g_ResultTimer   = 0.0;
	g_LastExpReward = 0;

	// HPバー位置の計算と設定（Draw の DebugText 配置にも使用）
	float screenW = (float)Direct3D_GetBackBufferWidth();
	float screenH = (float)Direct3D_GetBackBufferHeight();
	g_PlayerBarX = screenW * 0.25f - BATTLE_BAR_WIDTH / 2;
	g_PlayerBarY = screenH * 0.75f;
	g_EnemyBarX  = screenW * 0.75f - BATTLE_BAR_WIDTH / 2;
	g_EnemyBarY  = screenH * 0.75f;

	// DebugText 設定（HP数値のみ）
	const float TEXT_LINE_H  = 20.0f;
	const float TEXT_CHAR_W  = 12.0f;

	auto* dev = Direct3D_GetDevice();
	auto* ctx = Direct3D_GetContext();
	const wchar_t* fontTex = L"resource/texture/consolab_ascii_512.png";
	UINT sw = (UINT)screenW;
	UINT sh = (UINT)screenH;

	// プレイヤー HP 数値（バー下 1 行）
	delete g_pPlayerHpNumText;
	g_pPlayerHpNumText = new hal::DebugText(
		dev, ctx, fontTex, sw, sh,
		g_PlayerBarX, g_PlayerBarY + BATTLE_BAR_HEIGHT + 4.0f,
		1, 0, TEXT_LINE_H, TEXT_CHAR_W
	);
}

void Battle_Finalize()
{
	delete g_HitEffect;
	g_HitEffect = nullptr;

	// DebugText オブジェクト解放
	delete g_pPlayerHpNumText;  g_pPlayerHpNumText  = nullptr;

	// テキストテクスチャキャッシュクリア
	TextTexture_ClearCache();

	Player_SetPosition(g_PlayerSavePosition);

	// プレイヤーが勝利した場合：モンスター削除（EXPはWINカウント時に付与済み）
	if (g_pOriginalEnemy && g_EnemyHp <= 0) {
		Monster_Remove(g_pOriginalEnemy);
	}
	g_pOriginalEnemy = nullptr;
	g_pBattleEnemy = nullptr;

	for (int i = 0; i < 10; i++) g_BattleBullets[i].active = false;
	for (int i = 0; i < 5;  i++) g_EnemyBullets[i].active = false;
	g_EnemyAttackTimer = 0.0;

	g_EnemyHp = 0;
	g_EnemyHpMax = 0;
}

void Battle_Update(double elapsed_time)
{
	// リザルトフェーズ中：タイマー進行→ゲームシーンへ戻る
	if (g_BattlePhase == PHASE_WIN || g_BattlePhase == PHASE_LOSE) {
		g_ResultTimer -= elapsed_time;
		if (g_ResultTimer <= 0.0) {
			if (g_BattlePhase == PHASE_LOSE) {
				Scene_Change(SCENE_GAMEOVER);
			} else {
				Scene_Change(SCENE_GAME);
			}
		}
		return;
	}

	// 交代フェーズ：Tab/RBで切替、Space/Aで戦闘再開
	if (g_BattlePhase == PHASE_SWAP) {
		if (KeyLogger_IsTrigger(KK_TAB) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
			Party_CycleActiveFighter();
		}
		if (KeyLogger_IsTrigger(KK_SPACE) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_A)) {
			g_BattlePhase = PHASE_BATTLE;
			g_EnemyAttackTimer = g_EnemyAttackInterval;  // 敵攻撃タイマーリセット
		}
		return;
	}

	g_ShootCooldown -= elapsed_time;
	if (g_ShootCooldown < 0.0) g_ShootCooldown = 0.0;

	// RT連打検出（アナログなので前フレームと比較）
	bool curRT = PadLogger_GetRightTrigger(0) > 0.5f;
	bool rtTrigger = curRT && !g_PrevRT;
	g_PrevRT = curRT;

	if ((KeyLogger_IsTrigger(KK_SPACE) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_A) || rtTrigger) && g_ShootCooldown <= 0.0) {
		// 空きスロットを探す
		int slot = -1;
		for (int i = 0; i < 10; i++) {
			if (!g_BattleBullets[i].active) { slot = i; break; }
		}

		// 弾生成
		if (slot >= 0) {
			g_BattleBullets[slot].x = -2.5f;  // プレイヤー位置
			g_BattleBullets[slot].active = true;
			g_BattleBullets[slot].lifetime = 0.0;
		}
		g_ShootCooldown = SHOOT_INTERVAL;
	}

	// 弾更新
	for (int i = 0; i < 10; i++) {
		if (g_BattleBullets[i].active) {
			g_BattleBullets[i].x += 10.0f * (float)elapsed_time;
			g_BattleBullets[i].lifetime += elapsed_time;

			// 3秒で自動消滅
			if (g_BattleBullets[i].lifetime > 3.0) {
				g_BattleBullets[i].active = false;
				continue;
			}

			// 当たり判定
			if (g_EnemyHp > 0 && g_BattleBullets[i].x >= 1.5f && g_BattleBullets[i].x <= 2.5f) {
				int dmg = Party_GetFighterAtk() - g_EnemyDef / 2;
				if (dmg < 1) dmg = 1;
				g_EnemyHp -= dmg;
				if (g_EnemyHp < 0) g_EnemyHp = 0;
				g_BattleBullets[i].active = false;

				// ヒットパーティクル発生（位置をリセット）
				if (g_HitEffect) {
					g_HitEffect->SetPosition({ 2.0f, 0.0f, 5.0f });
				}
			}

			// 画面外
			if (g_BattleBullets[i].x > 5.0f) {
				g_BattleBullets[i].active = false;
			}
		}
	}

	// 敵の攻撃（種別パターン準拠）
	if (g_EnemyHp > 0) {
		// バースト処理中
		if (g_EnemyBurstRemaining > 0) {
			g_EnemyBurstTimer -= elapsed_time;
			if (g_EnemyBurstTimer <= 0.0) {
				for (int i = 0; i < 5; i++) {
					if (!g_EnemyBullets[i].active) {
						g_EnemyBullets[i].x = 2.0f;
						g_EnemyBullets[i].active = true;
						g_EnemyBullets[i].lifetime = 0.0;
						break;
					}
				}
				g_EnemyBurstRemaining--;
				g_EnemyBurstTimer = g_EnemyBurstDelay;
			}
		}
		else {
			// 攻撃タイマー
			g_EnemyAttackTimer -= elapsed_time;
			if (g_EnemyAttackTimer <= 0.0) {
				g_EnemyBurstRemaining = g_EnemyBurstCount;
				g_EnemyBurstTimer = 0.0;
				g_EnemyAttackTimer = g_EnemyAttackInterval;
			}
		}
	}

	// 敵弾の更新と当たり判定（敵ATK - プレイヤーDEF計算含む）
	for (int i = 0; i < 5; i++) {
		if (g_EnemyBullets[i].active) {
			g_EnemyBullets[i].x -= g_EnemyBulletSpeed * (float)elapsed_time;
			g_EnemyBullets[i].lifetime += elapsed_time;

			if (g_EnemyBullets[i].lifetime > 3.0) {
				g_EnemyBullets[i].active = false;
				continue;
			}

			if (g_EnemyBullets[i].x <= -2.0f && g_EnemyBullets[i].x >= -3.0f) {
				int dmg = g_EnemyAtk - Party_GetFighterDef() / 2;
				if (dmg < 1) dmg = 1;
				Party_FighterTakeDamage(dmg);
				g_EnemyBullets[i].active = false;
			}

			if (g_EnemyBullets[i].x < -5.0f) {
				g_EnemyBullets[i].active = false;
			}
		}
	}

	// パーティクル更新
	if (g_HitEffect) {
		g_HitEffect->Update(elapsed_time);
	}

	// 戦闘キャラ死亡 -> 交代 or ゲームオーバー
	if (Party_GetFighterHp() <= 0) {
		// 残弾を全消去
		for (int i = 0; i < 10; i++) g_BattleBullets[i].active = false;
		for (int i = 0; i < 5;  i++) g_EnemyBullets[i].active = false;

		// 他に生存者がいるか確認
		if (Party_SwitchToNextAlive()) {
			// 次の生存者に自動切替 → 交代フェーズへ
			g_BattlePhase = PHASE_SWAP;
		} else {
			// 全滅 → ゲームオーバー
			g_BattlePhase = PHASE_LOSE;
			g_ResultTimer = 2.5;
		}
		return;
	}

	// モンスター撃破 -> 経験値付与・仲間判定・勝利フェーズへ
	if (g_EnemyHp <= 0) {
		g_LastExpReward = g_EnemyExpReward;
		Party_AllGainExp(g_EnemyExpReward);  // プレイヤー＋全パーティモンスターにEXP付与

		// ボス撃破フラグ
		if (g_IsBossBattle) {
			g_BossDefeated = true;
		}

		// 仲間判定（ドラゴン以外、パーティ空きあり）
		g_CapturedThisBattle = false;
		int capture_rate = GetCaptureRate(g_EnemyKind);
		if (capture_rate > 0 && Party_GetCount() < Party_GetMax()) {
			if (rand() % 100 < capture_rate) {
				Party_Add(g_EnemyKind, g_EnemyLevel);
				g_CapturedThisBattle = true;
			}
		}

		// 残弾を全消去（リザルト表示を隠さないように）
		for (int i = 0; i < 10; i++) g_BattleBullets[i].active = false;
		for (int i = 0; i < 5;  i++) g_EnemyBullets[i].active = false;
		g_BattlePhase = PHASE_WIN;
		g_ResultTimer = 3.0;
		return;
	}
}

void Battle_Draw()
{
	Direct3D_SetBackBuffer();
	Direct3D_ClearBackBuffer();

	// バトル背景画像を描画
	{
		Direct3D_SetDepthEnable(false);
		Sprite_Begin();
		float sw = (float)Direct3D_GetBackBufferWidth();
		float sh = (float)Direct3D_GetBackBufferHeight();
		Sprite_Draw(g_BattleBgTex, 0.0f, 0.0f, sw, sh);
	}

	Direct3D_SetDepthEnable(true);

	// カメラ設定（PlayerCamera を使用）
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

	extern int g_MonsterTexSpider;
	extern int g_MonsterTexWolf;
	extern int g_MonsterTexDragon;

	// プレイヤー側（戦闘キャラ）の描画
	{
		MonsterKind fighterKind = Party_GetFighterKind();
		if (fighterKind == MONSTER_KIND_MAX) {
			// プレイヤー本人: スライムモデル（フィールドと同じ）
			XMMATRIX rot = XMMatrixRotationY(XMConvertToRadians(270.0f));
			XMMATRIX trans = XMMatrixTranslation(-2.5f, -0.5f, 5.0f);
			if (g_pBattlePlayerModel) {
				ModelDraw(g_pBattlePlayerModel, rot * trans);
			}
		} else {
			// パーティモンスター: 対応3Dモデル（敵と左右反転）
			switch (fighterKind) {
			case MONSTER_KIND_SPIDER:
			{
				XMMATRIX rot = XMMatrixRotationX(XM_PIDIV2) * XMMatrixRotationY(XM_PIDIV2);
				XMMATRIX trans = XMMatrixTranslation(-2.5f, -0.5f, 5.0f);
				if (g_pBattleSpiderModel) ModelDraw(g_pBattleSpiderModel, rot * trans, { 0.1f, 0.1f, 0.1f, 1.0f });
				break;
			}
			case MONSTER_KIND_WOLF:
			{
				XMMATRIX rot = XMMatrixRotationY(-XM_PIDIV2);
				XMMATRIX trans = XMMatrixTranslation(-2.5f, -1.0f, 5.0f);
				if (g_pBattleWolfModel) ModelDraw(g_pBattleWolfModel, rot * trans);
				break;
			}
			case MONSTER_KIND_ROBOT:
			{
				XMMATRIX trans = XMMatrixTranslation(-2.5f, 0.0f, 5.0f);
				if (g_pBattleRobotModel) ModelDraw(g_pBattleRobotModel, trans, { 0.4f, 0.4f, 0.5f, 1.0f });
				break;
			}
			case MONSTER_KIND_EYEBALL:
			{
				XMMATRIX rot = XMMatrixRotationZ(-XM_PIDIV2);
				XMMATRIX trans = XMMatrixTranslation(-2.5f, 0.5f, 5.0f);
				if (g_pBattleEyeballModel) ModelDraw(g_pBattleEyeballModel, rot * trans, { 0.8f, 0.1f, 0.1f, 1.0f });
				break;
			}
			default:
			{
				XMMATRIX worldPlayer = XMMatrixTranslation(-2.5f, 0.0f, 5.0f);
				Cube_Draw(g_MonsterTexSpider, worldPlayer);
				break;
			}
			}
		}
	}

	// 敵モンスター（種別ごとに3Dモデルで描画）
	// ボス戦時は2.0倍スケール
	float bossScale = g_IsBossBattle ? 2.0f : 1.0f;
	XMMATRIX bossScaleMtx = XMMatrixScaling(bossScale, bossScale, bossScale);

	switch (g_EnemyKind) {
	case MONSTER_KIND_WOLF:
	{
		Light_SetSpecularWorld({ 0.0f, 2.0f, -10.0f }, 0.5f, { 0.3f, 0.25f, 0.15f, 1.0f });
		// オオカミ
		XMMATRIX rotWolf = XMMatrixRotationY(XM_PIDIV2);
		XMMATRIX transWolf = XMMatrixTranslation(2.0f, -1.0f, 5.0f);
		XMMATRIX worldWolf = bossScaleMtx * rotWolf * transWolf;
		if (g_pBattleWolfModel) {
			ModelDraw(g_pBattleWolfModel, worldWolf);
		} else {
			Cube_Draw(g_MonsterTexWolf, transWolf);
		}
		break;
	}
	case MONSTER_KIND_DRAGON:
	{
		Light_SetSpecularWorld({ 0.0f, 2.0f, -10.0f }, 0.5f, { 0.1f, 0.1f, 0.2f, 1.0f });
		// ドラゴン（ボスは常に大きいのでさらに拡大）
		XMMATRIX rotDragon = XMMatrixRotationY(XM_PIDIV2);
		XMMATRIX transDragon = XMMatrixTranslation(5.0f, -4.5f, 5.0f);
		XMMATRIX worldDragon = bossScaleMtx * rotDragon * transDragon;
		if (g_pBattleDragonModel) {
			ModelDraw(g_pBattleDragonModel, worldDragon, { 0.0f, 0.0f, 1.0f, 1.0f });
		} else {
			Cube_Draw(g_MonsterTexDragon, transDragon);
		}
		break;
	}
	case MONSTER_KIND_SPIDER:
	{
		Light_SetSpecularWorld({ 0.0f, 2.0f, -10.0f }, 0.5f, { 0.1f, 0.1f, 0.1f, 1.0f });
		// クモ
		XMMATRIX rotSpider = XMMatrixRotationX(XM_PIDIV2) * XMMatrixRotationY(-XM_PIDIV2);
		XMMATRIX transSpider = XMMatrixTranslation(2.0f, -0.5f, 5.0f);
		XMMATRIX worldSpider = bossScaleMtx * rotSpider * transSpider;
		if (g_pBattleSpiderModel) {
			ModelDraw(g_pBattleSpiderModel, worldSpider, { 0.1f, 0.1f, 0.1f, 1.0f });
		} else {
			Cube_Draw(g_MonsterTexSpider, transSpider);
		}
		break;
	}
	case MONSTER_KIND_ROBOT:
	{
		Light_SetSpecularWorld({ 0.0f, 2.0f, -10.0f }, 0.5f, { 0.15f, 0.15f, 0.2f, 1.0f });
		// ロボット（プレイヤー方向 = -X を向く）
		XMMATRIX rotRobot = XMMatrixRotationY(XM_PI);
		XMMATRIX transRobot = XMMatrixTranslation(2.0f, 0.0f, 5.0f);
		XMMATRIX worldRobot = bossScaleMtx * rotRobot * transRobot;
		if (g_pBattleRobotModel) {
			ModelDraw(g_pBattleRobotModel, worldRobot, { 0.4f, 0.4f, 0.5f, 1.0f });
		} else {
			extern int g_MonsterTexRobot;
			Cube_Draw(g_MonsterTexRobot, transRobot);
		}
		break;
	}
	case MONSTER_KIND_EYEBALL:
	{
		Light_SetSpecularWorld({ 0.0f, 2.0f, -10.0f }, 0.5f, { 0.25f, 0.05f, 0.05f, 1.0f });
		// 目玉
		XMMATRIX rotEye = XMMatrixRotationZ(XM_PIDIV2);
		XMMATRIX transEye = XMMatrixTranslation(2.0f, 0.5f, 5.0f);
		XMMATRIX worldEye = bossScaleMtx * rotEye * transEye;
		if (g_pBattleEyeballModel) {
			ModelDraw(g_pBattleEyeballModel, worldEye, { 0.8f, 0.1f, 0.1f, 1.0f });
		} else {
			extern int g_MonsterTexEyeball;
			Cube_Draw(g_MonsterTexEyeball, transEye);
		}
		break;
	}
	default:
	{
		XMMATRIX transEnemy = XMMatrixTranslation(2.0f, 0.0f, 5.0f);
		Cube_Draw(g_MonsterTexSpider, transEnemy);
		break;
	}
	}

	// ここから2D描画（HPバー）
	Direct3D_SetDepthEnable(false);
	Sprite_Begin();

	// ==== プレイヤー（戦闘キャラ）HPバー =======================================
	float player_hp_ratio = (Party_GetFighterHpMax() > 0)
		? (float)Party_GetFighterHp() / (float)Party_GetFighterHpMax() : 0.0f;

	// HP枠
	Sprite_Draw(g_WhiteTexture,
		g_PlayerBarX - 2, g_PlayerBarY - 2,
		BATTLE_BAR_WIDTH + 4, BATTLE_BAR_HEIGHT + 4,
		{ 1.0f, 1.0f, 1.0f, 1.0f });
	// HP背景
	Sprite_Draw(g_WhiteTexture,
		g_PlayerBarX, g_PlayerBarY,
		BATTLE_BAR_WIDTH, BATTLE_BAR_HEIGHT,
		{ 0.0f, 0.0f, 0.0f, 1.0f });
	// HP（色変化：緑→黄→赤）
	XMFLOAT4 player_hp_color = { 0.0f, 1.0f, 0.0f, 1.0f };
	if (player_hp_ratio <= 0.5f)  player_hp_color = { 1.0f, 1.0f, 0.0f, 1.0f };
	if (player_hp_ratio <= 0.25f) player_hp_color = { 1.0f, 0.0f, 0.0f, 1.0f };
	if (Party_GetFighterHp() > 0) {
		Sprite_Draw(g_WhiteTexture,
			g_PlayerBarX, g_PlayerBarY,
			BATTLE_BAR_WIDTH * player_hp_ratio, BATTLE_BAR_HEIGHT,
			player_hp_color);
	}

	// ==== 敵 HPバー ============================================================
	float enemy_hp_ratio = (g_EnemyHpMax > 0)
		? (float)g_EnemyHp / (float)g_EnemyHpMax : 0.0f;

	// HP枠
	Sprite_Draw(g_WhiteTexture,
		g_EnemyBarX - 2, g_EnemyBarY - 2,
		BATTLE_BAR_WIDTH + 4, BATTLE_BAR_HEIGHT + 4,
		{ 1.0f, 1.0f, 1.0f, 1.0f });
	// HP背景
	Sprite_Draw(g_WhiteTexture,
		g_EnemyBarX, g_EnemyBarY,
		BATTLE_BAR_WIDTH, BATTLE_BAR_HEIGHT,
		{ 0.1f, 0.1f, 0.1f, 1.0f });
	// HP（色変化：緑→黄→赤）
	XMFLOAT4 hp_color = { 0.0f, 1.0f, 0.0f, 1.0f };
	if (enemy_hp_ratio <= 0.5f)  hp_color = { 1.0f, 1.0f, 0.0f, 1.0f };
	if (enemy_hp_ratio <= 0.25f) hp_color = { 1.0f, 0.0f, 0.0f, 1.0f };
	if (g_EnemyHp > 0) {
		Sprite_Draw(g_WhiteTexture,
			g_EnemyBarX, g_EnemyBarY,
			BATTLE_BAR_WIDTH * enemy_hp_ratio, BATTLE_BAR_HEIGHT,
			hp_color);
	}

	// ==== HPバー テキスト表示 ==================================================
	char buf[64];
	int texId;
	const int HEADER_FONT = 20;
	const int RESULT_FONT = 28;

	// 戦闘キャラ：Lv + 名前（バー上）TextTexture で描画
	{
		snprintf(buf, sizeof(buf), "Lv.%d", Party_GetFighterLevel());
		texId = TextTexture_Create(buf, HEADER_FONT);
		if (texId >= 0) {
			Sprite_Draw(texId, g_PlayerBarX, g_PlayerBarY - 46.0f);
		}
		texId = TextTexture_Create(Party_GetFighterName(), HEADER_FONT);
		if (texId >= 0) {
			Sprite_Draw(texId, g_PlayerBarX, g_PlayerBarY - 24.0f);
		}
	}

	// 戦闘キャラ：現在HP / 最大HP（バー下）DebugText で描画
	if (g_pPlayerHpNumText) {
		g_pPlayerHpNumText->Clear();
		snprintf(buf, sizeof(buf), "%d/%d", Party_GetFighterHp(), Party_GetFighterHpMax());
		g_pPlayerHpNumText->SetText(buf, { 1.0f, 1.0f, 1.0f, 1.0f });
		g_pPlayerHpNumText->Draw();
	}

	// 敵：Lv + 名前（バー上）TextTexture で描画
	{
		snprintf(buf, sizeof(buf), "Lv.%d", g_EnemyLevel);
		texId = TextTexture_Create(buf, HEADER_FONT);
		if (texId >= 0) {
			Sprite_Draw(texId, g_EnemyBarX, g_EnemyBarY - 46.0f);
		}
		texId = TextTexture_Create(GetEnemyDisplayName(g_EnemyKind), HEADER_FONT);
		if (texId >= 0) {
			Sprite_Draw(texId, g_EnemyBarX, g_EnemyBarY - 24.0f);
		}
	}

	// ==== 交代フェーズオーバーレイ ============================================
	if (g_BattlePhase == PHASE_SWAP) {
		float ow = (float)Direct3D_GetBackBufferWidth();
		float oh = (float)Direct3D_GetBackBufferHeight();
		Sprite_Draw(g_WhiteTexture,
			ow * 0.25f, oh * 0.35f,
			ow * 0.5f, oh * 0.3f,
			{ 0.0f, 0.0f, 0.0f, 0.75f });

		char rbuf[128];
		float cx = ow * 0.5f - 120.0f;
		float cy = oh * 0.5f - 50.0f;

		snprintf(rbuf, sizeof(rbuf), "せんとうふのう！");
		texId = TextTexture_Create(rbuf, RESULT_FONT);
		if (texId >= 0) Sprite_Draw(texId, cx, cy, { 1.0f, 0.8f, 0.0f, 1.0f });

		snprintf(rbuf, sizeof(rbuf), "つぎ: %s Lv.%d", Party_GetFighterName(), Party_GetFighterLevel());
		texId = TextTexture_Create(rbuf, RESULT_FONT);
		if (texId >= 0) Sprite_Draw(texId, cx, cy + 34.0f, { 1.0f, 0.8f, 0.0f, 1.0f });

		texId = TextTexture_Create("[Tab]こうたい [Space]すすむ", RESULT_FONT);
		if (texId >= 0) Sprite_Draw(texId, cx, cy + 68.0f, { 1.0f, 0.8f, 0.0f, 1.0f });
	}

	// ==== 勝敗リザルトオーバーレイ ============================================
	if (g_BattlePhase == PHASE_WIN || g_BattlePhase == PHASE_LOSE) {
		float ow = (float)Direct3D_GetBackBufferWidth();
		float oh = (float)Direct3D_GetBackBufferHeight();
		// 半透明パネル（暗い背景）
		Sprite_Draw(g_WhiteTexture,
			ow * 0.25f, oh * 0.35f,
			ow * 0.5f, oh * 0.3f,
			{ 0.0f, 0.0f, 0.0f, 0.75f });

		char rbuf[128];
		XMFLOAT4 rcolor;
		float cx = ow * 0.5f - 120.0f;
		float cy = oh * 0.5f - 50.0f;

		if (g_BattlePhase == PHASE_WIN) {
			rcolor = { 1.0f, 1.0f, 0.0f, 1.0f };  // 黄色

			snprintf(rbuf, sizeof(rbuf), "%s をたおした！", GetEnemyDisplayName(g_EnemyKind));
			texId = TextTexture_Create(rbuf, RESULT_FONT);
			if (texId >= 0) Sprite_Draw(texId, cx, cy, rcolor);

			snprintf(rbuf, sizeof(rbuf), "けいけんち +%d", g_LastExpReward);
			texId = TextTexture_Create(rbuf, RESULT_FONT);
			if (texId >= 0) Sprite_Draw(texId, cx, cy + 34.0f, rcolor);

			if (g_CapturedThisBattle) {
				snprintf(rbuf, sizeof(rbuf), "%s がなかまになった！", GetEnemyDisplayName(g_EnemyKind));
				texId = TextTexture_Create(rbuf, RESULT_FONT);
				if (texId >= 0) Sprite_Draw(texId, cx, cy + 68.0f, rcolor);
			}
		} else {
			rcolor = { 1.0f, 0.3f, 0.3f, 1.0f };  // 赤
			texId = TextTexture_Create("まけてしまった...", RESULT_FONT);
			if (texId >= 0) Sprite_Draw(texId, cx, cy, rcolor);
		}
	}

	Direct3D_SetDepthEnable(true);

	// プレイヤー弾の描画
	for (int i = 0; i < 10; i++) {
		if (g_BattleBullets[i].active) {
			XMMATRIX scale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
			XMMATRIX trans = XMMatrixTranslation(g_BattleBullets[i].x, -0.3f, 5.0f);
			Cube_Draw(g_MonsterTexDragon, scale * trans);
		}
	}

	// 敵弾の描画
	for (int i = 0; i < 5; i++) {
		if (g_EnemyBullets[i].active) {
			XMMATRIX scale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
			XMMATRIX trans = XMMatrixTranslation(g_EnemyBullets[i].x, -0.3f, 5.0f);
			Cube_Draw(g_MonsterTexWolf, scale * trans);
		}
	}

	// パーティクル描画
	if (g_HitEffect) {
		Direct3D_SetDepthWriteDisable();
		Direct3D_SetAlphaBlendAdd();
		g_HitEffect->Draw();
		Direct3D_SetDepthEnable(true);
		Direct3D_SetAlphaBlendTransparent();  // 元に戻す
	}
}
