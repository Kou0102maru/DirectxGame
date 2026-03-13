/*==============================================================================

   ゲーム本体 [game.cpp]
														 Author : Kotaro Marugami
														 Date   : 2025/06/27
--------------------------------------------------------------------------------

==============================================================================*/
#include "game.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "key_logger.h"
#include "sampler.h"
#include "light.h"
#include "camera.h"
#include "player_camera.h"
#include "player.h"
#include "bullet.h"
#include "map.h"
#include "billboard.h"
#include "sprite_anim.h"
#include "bullet_hit_effect.h"
#include "trajectory3d.h"
#include "sky.h"
#include "enemy.h"
#include"monster.h"
#include"monster_spider.h"
#include "mouse.h"
#include "cube.h"
#include "sprite.h"
#include"scene.h"
#include "debug_text.h"
#include"texture.h"
#include"mapcamera.h"
#include"light_camera.h"
#include"cirel_shadow.h"
#include"easing_cube.h"
#include"battle.h"
#include"particle_test.h"
#include"party.h"
#include"pad_logger.h"
#include"text_texture.h"
#include <cstdlib>

static double g_AccumulatedTime = 0.0;
static bool g_IsDebug = false;
static int g_TestTex01{ -1 };

// ステータス画面
static bool g_ShowStatus = false;
static hal::DebugText* g_pStatusText = nullptr;
static int g_StatusWhiteTex = -1;
static int g_StatusCursor = -1;    // -1=カーソルなし, 0~=パーティモンスターインデックス
static bool g_StatusConfirm = false; // 削除確認中フラグ

// フィールドHUD（パーティアイコン表示）
static int g_IconSlimeTex = -1;    // プレイヤー（スライム）
static int g_IconSpiderTex = -1;
static int g_IconWolfTex = -1;
static int g_IconDragonTex = -1;
static int g_IconRobotTex = -1;
static int g_IconEyeballTex = -1;

static int GetMonsterIconTex(MonsterKind kind)
{
	switch (kind) {
	case MONSTER_KIND_SPIDER:  return g_IconSpiderTex;
	case MONSTER_KIND_WOLF:    return g_IconWolfTex;
	case MONSTER_KIND_DRAGON:  return g_IconDragonTex;
	case MONSTER_KIND_ROBOT:   return g_IconRobotTex;
	case MONSTER_KIND_EYEBALL: return g_IconEyeballTex;
	default: return -1;
	}
}

// ステータス画面用ASCII名（DebugTextはASCIIフォントなので日本語不可）
static const char* GetMonsterNameAscii(MonsterKind kind)
{
	switch (kind) {
	case MONSTER_KIND_SPIDER:  return "Spider";
	case MONSTER_KIND_WOLF:    return "Wolf";
	case MONSTER_KIND_DRAGON:  return "Dragon";
	case MONSTER_KIND_ROBOT:   return "Robot";
	case MONSTER_KIND_EYEBALL: return "Eyeball";
	default:                   return "???";
	}
}

static const char* GetFighterNameAscii()
{
	int af = Party_GetActiveFighter();
	if (af == -1) return "Player";
	const PartyMonster* pm = Party_Get(af);
	if (pm) return GetMonsterNameAscii(pm->kind);
	return "???";
}

//=============================================================================
// 距離ベース敵出現システム（ドラゴン除外）
//=============================================================================
static constexpr float SPAWN_DISTANCE = 20.0f;  // この距離以内に近づくと出現
static constexpr float RESET_DISTANCE = 35.0f;  // この距離以上離れると再出現可能になる

// 出現ポイントの状態
enum SpawnState {
    SPAWN_READY,     // 出現可能（プレイヤー接近待ち）
    SPAWN_ACTIVE,    // モンスター出現中
    SPAWN_DEFEATED   // 撃破済み（プレイヤーが離れるまで待ち）
};

struct SpawnPoint {
    MonsterKind kind;        // モンスター種別
    int         level;       // レベル
    XMFLOAT3    position;    // 固定出現位置
    SpawnState  state;       // 現在の状態
    Monster*    pMonster;    // 出現中のモンスターポインタ
    bool        is_boss;     // ボスフラグ
};

static constexpr int SPAWN_POINT_MAX = 9;
static SpawnPoint g_SpawnPoints[SPAWN_POINT_MAX];
static int g_SpawnPointCount = 0;

// 現在のステージ番号（1 or 2）
static int g_CurrentStage = 1;

// 出現ポイントの初期化（ステージ別）
static void InitSpawnPoints()
{
    g_SpawnPointCount = SPAWN_POINT_MAX;

    if (g_CurrentStage == 1) {
        // ステージ1: 遺跡迷路内に配置
        // ジグザグ通路: 入口→右→左→右→ボス部屋
        //
        // Room1 (Z=-40?-15): 入口エリア
        g_SpawnPoints[0] = { MONSTER_KIND_SPIDER, 1, { 15.0f, 0.0f, -30.0f }, SPAWN_READY, nullptr, false };
        g_SpawnPoints[1] = { MONSTER_KIND_SPIDER, 1, {-15.0f, 0.0f, -28.0f }, SPAWN_READY, nullptr, false };
        g_SpawnPoints[2] = { MONSTER_KIND_SPIDER, 2, {  0.0f, 0.0f, -22.0f }, SPAWN_READY, nullptr, false };
        // Room2 (Z=-15?+15): 中央エリア
        g_SpawnPoints[3] = { MONSTER_KIND_SPIDER, 3, {-15.0f, 0.0f,  -5.0f }, SPAWN_READY, nullptr, false };
        g_SpawnPoints[4] = { MONSTER_KIND_WOLF,   2, { 15.0f, 0.0f,   5.0f }, SPAWN_READY, nullptr, false };
        // Room3 (Z=+16?+39): 奥エリア
        g_SpawnPoints[5] = { MONSTER_KIND_WOLF,   3, {  0.0f, 0.0f,  25.0f }, SPAWN_READY, nullptr, false };
        g_SpawnPoints[6] = { MONSTER_KIND_WOLF,   4, {-15.0f, 0.0f,  35.0f }, SPAWN_READY, nullptr, false };
        // Room4 (Z=+41?+58): ボス部屋
        g_SpawnPoints[7] = { MONSTER_KIND_WOLF,   3, { 20.0f, 0.0f,  48.0f }, SPAWN_READY, nullptr, false };
        // ボス目玉 Lv7
        g_SpawnPoints[8] = { MONSTER_KIND_EYEBALL, 7, {  0.0f, 2.5f,  53.0f }, SPAWN_READY, nullptr, true };
    }
    else {
        // ステージ2: 遺跡迷路内に配置（強敵）
        // Room1
        g_SpawnPoints[0] = { MONSTER_KIND_EYEBALL, 4, { 15.0f, 2.5f, -30.0f }, SPAWN_READY, nullptr, false };
        g_SpawnPoints[1] = { MONSTER_KIND_EYEBALL, 5, {-15.0f, 2.5f, -28.0f }, SPAWN_READY, nullptr, false };
        // Room2
        g_SpawnPoints[2] = { MONSTER_KIND_EYEBALL, 6, {-10.0f, 2.5f,   0.0f }, SPAWN_READY, nullptr, false };
        g_SpawnPoints[3] = { MONSTER_KIND_ROBOT,   5, { 15.0f, 0.8f,   5.0f }, SPAWN_READY, nullptr, false };
        // Room3
        g_SpawnPoints[4] = { MONSTER_KIND_ROBOT,   5, {  0.0f, 0.8f,  25.0f }, SPAWN_READY, nullptr, false };
        g_SpawnPoints[5] = { MONSTER_KIND_ROBOT,   6, {-15.0f, 0.8f,  35.0f }, SPAWN_READY, nullptr, false };
        g_SpawnPoints[6] = { MONSTER_KIND_ROBOT,   7, { 20.0f, 0.8f,  30.0f }, SPAWN_READY, nullptr, false };
        // Room4: ボス部屋
        g_SpawnPoints[7] = { MONSTER_KIND_ROBOT,   6, {-10.0f, 0.8f,  48.0f }, SPAWN_READY, nullptr, false };
        // ボスドラゴン Lv15
        g_SpawnPoints[8] = { MONSTER_KIND_DRAGON, 15, {  0.0f, 0.0f,  53.0f }, SPAWN_READY, nullptr, true };
    }
}

// 出現ポイントにモンスターを生成
static Monster* SpawnAtPoint(MonsterKind kind, int level, const XMFLOAT3& pos)
{
    int count_before = Monster_GetCount();

    switch (kind) {
    case MONSTER_KIND_SPIDER:
        Monster_CreateSpider(pos, level);
        break;
    case MONSTER_KIND_WOLF:
        Monster_CreateWolf(pos, level);
        break;
    case MONSTER_KIND_ROBOT:
        Monster_CreateRobot(pos, level);
        break;
    case MONSTER_KIND_EYEBALL:
        Monster_CreateEyeball(pos, level);
        break;
    case MONSTER_KIND_DRAGON:
        Monster_CreateDragon(pos, level);
        break;
    default:
        return nullptr;
    }

    if (Monster_GetCount() > count_before) {
        return Monster_Get(Monster_GetCount() - 1);
    }
    return nullptr;
}

// プレイヤーとの距離を計算（XZ平面）
static float CalcDistanceXZ(const XMFLOAT3& a, const XMFLOAT3& b)
{
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

// 距離ベース出現更新処理
static void UpdateSpawnSystem()
{
    XMFLOAT3 player_pos = Player_GetPosition();

    for (int i = 0; i < g_SpawnPointCount; i++) {
        SpawnPoint& sp = g_SpawnPoints[i];
        float dist = CalcDistanceXZ(player_pos, sp.position);

        switch (sp.state) {
        case SPAWN_READY:
            // プレイヤーが接近したら出現
            if (dist < SPAWN_DISTANCE) {
                sp.pMonster = SpawnAtPoint(sp.kind, sp.level, sp.position);
                if (sp.pMonster) {
                    sp.state = SPAWN_ACTIVE;
                    // ボスは2.5倍サイズで表示
                    if (sp.is_boss) {
                        sp.pMonster->SetFieldScale(1.5f);
                    }
                }
            }
            break;

        case SPAWN_ACTIVE:
            // モンスターが倒されたか確認
            if (sp.pMonster == nullptr || !Monster_Exists(sp.pMonster)) {
                sp.pMonster = nullptr;
                sp.state = SPAWN_DEFEATED;
            }
            break;

        case SPAWN_DEFEATED:
            // ボスは再出現しない
            if (sp.is_boss) break;
            // プレイヤーが十分離れたら再出現可能に戻す
            if (dist > RESET_DISTANCE) {
                sp.state = SPAWN_READY;
            }
            break;
        }
    }
}
//static int g_BillboardTex{ -1 };

void mapRendering();
void lightRendering();

static EasingCube g_ecube({ -10.0f,1.0f,10.0f }, { 10.0f,1.0f,10.0f }, 2.0);
static NormalEmitter* g_Emitter;

void Game_Initialize()
{
	// 戦闘から復帰時、保存した位置から開始する
	extern XMFLOAT3 g_PlayerSavePosition;  // battle.cpp で定義された保存位置
	extern bool g_ReturnFromBattle;

	if (g_ReturnFromBattle) {
		Player_Initialize(g_PlayerSavePosition, { 0.0f, 0.0f, 1.0f });
	}
	else {
		g_CurrentStage = 1;
		Player_ResetStats();  // ステータスを初期値にリセット（ゲームクリア/ゲームオーバー後の再開用）
		Player_Initialize({ 0.0f, 0.5f, -28.0f }, { 0.0f, 0.0f, 1.0f });
		Party_Initialize();  // パーティは初回のみ初期化（戦闘復帰時は保持）
	}

	Enemy_Initialize();
	Monster_Initialize();
	Camera_Initialize({ 8.2f, 8.4f, -12.7f }, { -0.5f, -0.3f, 0.7f }, { 0.8f, 0.0f, 0.5f });
	PlayerCamera_Initialize();
	Map_Initialize();
	Map_SetStage(g_CurrentStage);
	Bullet_Initialize();
	Sky_Initialize();
	Billboard_Initialize();
	BulletHitEffect_Initialize();
	Trajectory3d_Initialize();
	LightCamera_Initialize({ -1.0f, -1.0f, 1.0f }, { 0.0f,20.0f,0.0f });
	CircleShadow_Initialize();

	//g_Emitter = new NormalEmitter(2000, { 5.0f,1.0f,0.0f }, 200.0, true);

	//Enemy_Create({-3.0f, 1.0f, 5.0f});
	//Enemy_Create({ 8.0f, 3.0f, 20.0f });

	// 距離ベース出現ポイント初期化（ステージに応じてモンスター配置）
	InitSpawnPoints();

	//g_TestTex01 = Texture_Load(L"resource/texture/knight.png");
	//g_BillboardTex = Texture_Load(L"resource/texture/knight.png");
	g_IsDebug = false;

	g_ReturnFromBattle = false;

	// ステータス画面の初期化
	g_ShowStatus = false;
	if (g_StatusWhiteTex < 0) {
		g_StatusWhiteTex = Texture_Load(L"resource/texture/white.png");
	}
	delete g_pStatusText;
	{
		float sw = (float)Direct3D_GetBackBufferWidth();
		float sh = (float)Direct3D_GetBackBufferHeight();
		g_pStatusText = new hal::DebugText(
			Direct3D_GetDevice(), Direct3D_GetContext(),
			L"resource/texture/consolab_ascii_512.png",
			(UINT)sw, (UINT)sh,
			sw * 0.5f - 185.0f, sh * 0.5f - 140.0f,
			32, 0, 20.0f, 11.0f
		);

	}

	// パーティアイコンテクスチャのロード
	g_IconSlimeTex   = Texture_Load(L"resource/texture/slimeicon.png");
	g_IconSpiderTex  = Texture_Load(L"resource/texture/spidericon.png");
	g_IconWolfTex    = Texture_Load(L"resource/texture/wolficon.png");
	g_IconDragonTex  = Texture_Load(L"resource/texture/dragonicon.png");
	g_IconRobotTex   = Texture_Load(L"resource/texture/roboticon.png");
	g_IconEyeballTex = Texture_Load(L"resource/texture/eyeicon.png");
}


void Game_Finalize()
{
	//delete g_Emitter;
	//g_Emitter = nullptr;

	delete g_pStatusText;
	g_pStatusText = nullptr;

	CircleShadow_Finalize();
	Trajectory3d_Finalize();
	BulletHitEffect_Finalize();
	Billboard_Finalize();
	Sky_Finalize();
	Bullet_Finalize();
	Map_Finalize();
	PlayerCamera_Finalize();
	Camera_Finalize();
	Monster_Finalize();
	Enemy_Finalize();
	Player_Finalize();
}

void Game_Update(double elapsed_time)
{
	// バトル復帰後のボス撃破チェック
	extern bool g_BossDefeated;
	if (g_BossDefeated) {
		g_BossDefeated = false;
		if (g_CurrentStage == 1) {
			// ステージ1ボス撃破 → ステージ2へ
			g_CurrentStage = 2;
			Map_SetStage(2);
			// 全モンスター削除してステージ2のスポーン再初期化
			Monster_Finalize();
			Monster_Initialize();
			InitSpawnPoints();
			// プレイヤー位置リセット・HP全回復
			Player_SetPosition({ 0.0f, 0.5f, -28.0f });
			Player_TakeDamage(-(Player_GetHpMax() - Player_GetHp()));
			return;
		}
		else {
			// ステージ2ボス撃破 → ゲームクリア
			Scene_Change(SCENE_GAMECLEAR);
			return;
		}
	}

	// Mキー / Startボタン：ステータス画面のトグル
	if (KeyLogger_IsTrigger(KK_M) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_START)) {
		g_ShowStatus = !g_ShowStatus;
		g_StatusCursor = -1;       // 開閉時カーソルリセット
		g_StatusConfirm = false;
	}

	// Tabキー / RBボタン：戦闘キャラ切替
	if (KeyLogger_IsTrigger(KK_TAB) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
		Party_CycleActiveFighter();
	}

	// ステータス画面表示中の操作
	if (g_ShowStatus) {
		int party_count = Party_GetCount();

		if (g_StatusConfirm) {
			// 削除確認中: Yキー/Aボタンで確定、Nキー/Bボタンでキャンセル
			if (KeyLogger_IsTrigger(KK_Y) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_A)) {
				Party_Remove(g_StatusCursor);
				g_StatusConfirm = false;
				// カーソル補正
				if (g_StatusCursor >= Party_GetCount()) {
					g_StatusCursor = Party_GetCount() - 1;
				}
				if (Party_GetCount() == 0) g_StatusCursor = -1;
			}
			if (KeyLogger_IsTrigger(KK_N) || KeyLogger_IsTrigger(KK_ESCAPE) ||
				PadLogger_IsTrigger(0, XINPUT_GAMEPAD_B)) {
				g_StatusConfirm = false;
			}
		}
		else {
			// 上下キー / 十字キーでカーソル移動
			if (party_count > 0) {
				if (KeyLogger_IsTrigger(KK_UP) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_DPAD_UP)) {
					if (g_StatusCursor < 0) g_StatusCursor = party_count - 1;
					else if (g_StatusCursor > 0) g_StatusCursor--;
					else g_StatusCursor = -1;  // 一番上からさらに上→カーソル解除
				}
				if (KeyLogger_IsTrigger(KK_DOWN) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_DPAD_DOWN)) {
					if (g_StatusCursor < 0) g_StatusCursor = 0;
					else if (g_StatusCursor < party_count - 1) g_StatusCursor++;
					else g_StatusCursor = -1;  // 一番下からさらに下→カーソル解除
				}
			}
			// Xキー / Xボタン：選択中のモンスターを外す確認
			if (g_StatusCursor >= 0 && g_StatusCursor < party_count) {
				if (KeyLogger_IsTrigger(KK_X) || PadLogger_IsTrigger(0, XINPUT_GAMEPAD_X)) {
					g_StatusConfirm = true;
				}
			}
		}
		return;  // ステータス画面表示中はゲーム停止
	}

	g_AccumulatedTime += elapsed_time;

	Player_Update(elapsed_time);

	XMFLOAT3 player_pos = Player_GetPosition();
	for (int i = 0; i < Monster_GetCount(); i++) {
		Monster* monster = Monster_Get(i);
		if (monster->IsDead()) continue;

		XMFLOAT3 monster_pos = monster->GetPosition();
		XMVECTOR to_monster = XMLoadFloat3(&monster_pos) - XMLoadFloat3(&player_pos);
		float distance = XMVectorGetX(XMVector3Length(to_monster));

		if (distance < 2.0f) {
			// スポーンポイントからボスフラグを取得
			bool is_boss = false;
			for (int j = 0; j < g_SpawnPointCount; j++) {
				if (g_SpawnPoints[j].pMonster == monster) {
					is_boss = g_SpawnPoints[j].is_boss;
					break;
				}
			}
			Battle_SetBossFlag(is_boss);
			Battle_SetEnemy(monster);
			Scene_Change(SCENE_BATTLE);
			return;
		}
	}

	Enemy_Update(elapsed_time);
	Monster_Update(elapsed_time);
	UpdateSpawnSystem();
	Sky_SetPosition(Player_GetPosition());

	if (g_IsDebug) {
		Camera_Update(elapsed_time);
	}
	else {
		PlayerCamera_Update(elapsed_time);
	}

	Bullet_Update(elapsed_time);

	// 弾とマップの当たり判定（AABB vs AABB）
	for (int j = 0; j < Map_GetObjectsCount(); j++) {
		for (int i = 0; i < Bullet_GetBulletsCount(); i++) {
			AABB bullet = Bullet_GetAABB(i);
			AABB object = Map_GetObject(j)->Aabb;
			if (Collision_IsOverlapAABB(bullet, object)) {
				BulletHitEffect_Create(Bullet_GetPosition(i));
				Bullet_Destroy(i);
			}
		}
	}

	// 敵と弾の当たり判定
	for (int j = 0; j < Enemy_GetEnemyCount(); j++) {
		for (int i = 0; i < Bullet_GetBulletsCount(); i++) {
			Sphere bullet = Bullet_GetSphere(i);
			Sphere enemy = Enemy_GetEnemy(j)->GetCollision();
			if (Collision_IsOverlapSphere(bullet, enemy)) {
				BulletHitEffect_Create(Bullet_GetPosition(i));
				Bullet_Destroy(i);
				Enemy_GetEnemy(j)->Damage(50);
			}
		}
	}

	SpriteAnim_Update(elapsed_time);
	BulletHitEffect_Update();
	Trajectory3d_Update(elapsed_time);

	if (KeyLogger_IsTrigger(KK_L)) {
		g_IsDebug = !g_IsDebug;
	}

	//if (KeyLogger_IsTrigger(KK_G)) {
	//	g_ecude.Strat();
	//}
	//g_Emitter->Update(elapsed_time);

}

void Game_Draw()
{
	Direct3D_SetDepthEnable(true);
	Direct3D_SetAlphaBlendTransparent();
	mapRendering();// オフスクリーンへのレンダリング
	//lightRendering();
	//Direct3D_SetDepthTexture(2);
	// レンダーターゲットをバックバッファへ

	Direct3D_SetBackBuffer();
	Direct3D_ClearBackBuffer();

	Mouse_State ms{};
	Mouse_GetState(&ms);

	XMFLOAT4X4 mtxView = g_IsDebug ? Camera_GetMatrix() : PlayerCamera_GetViewMatrix();
	XMMATRIX view = XMLoadFloat4x4(&mtxView);
	XMMATRIX proj = g_IsDebug ? XMLoadFloat4x4(&Camera_GetPerspectiveMatrix()) : XMLoadFloat4x4(&PlayerCamera_GetPerspectiveMatrix());
	XMFLOAT3 camera_position = g_IsDebug ? Camera_GetPosition() : PlayerCamera_GetPosition();

	XMFLOAT3 test_near = Direct3D_ScreenToWorld(ms.x, ms.y, 0.0f, mtxView, PlayerCamera_GetPerspectiveMatrix());
	XMFLOAT3 test_far = Direct3D_ScreenToWorld(ms.x, ms.y, 1.0f, mtxView, PlayerCamera_GetPerspectiveMatrix());

	// y=0の平面との交点を求める
	XMVECTOR vtest = XMLoadFloat3(&test_far) - XMLoadFloat3(&test_near); // nearからfarへのベクトルを求める
	vtest = XMVector3Normalize(vtest); // 単位ベクトルにする

	// nearのyとnearからfarへの単位ベクトルのyで比率を求める
	// y = 0の距離（基本的に0以上）を算出してマイナスの場合はマイナスで割る
	float ratio = -XMVectorGetY(XMLoadFloat3(&test_near)) / XMVectorGetY(vtest);
	vtest = XMLoadFloat3(&test_near) + vtest * ratio; // near座標 + farへの単位ベクトル * y = 0までの距離（比率）

	// カメラのビュー行列をシェーダーに設定
	Camera_SetMatrix(view, proj);

	// ビルボードにカメラのビュー行列を設定
	Billboard_SetViewMatrix(mtxView);

	// テクスチャーサンプラーの設定
	Sampler_SetFilterAnisotropic();

	// 空の描画
	Direct3D_SetDepthEnable(false);
	Sky_Draw();
	Direct3D_SetDepthEnable(true);

	// 平行ライト設定
	Light_SetAmbient({0.3f, 0.3f, 0.3f});
	XMVECTOR v{ -1.0f, -1.0f, 1.0f };
	v = XMVector3Normalize(v);
	XMFLOAT4 dir;
	XMStoreFloat4(&dir, v);
	Light_SetDirectionalWorld(dir, {0.8f, 0.8f, 0.8f, 1.0f});

	Light_SetPointLightCount(0);
	XMMATRIX rot = XMMatrixRotationY((float)g_AccumulatedTime);
	// XMMATRIX rot = XMMatrixIdentity();
	XMFLOAT3 pp0, pp1, pp2;
	XMStoreFloat3(&pp0, XMVector3Transform({ 0.0f, 1.0f, -3.0f }, rot));
	XMStoreFloat3(&pp1, XMVector3Transform({ 0.0f, 1.0f, -5.0f }, rot));
	XMStoreFloat3(&pp2, XMVector3Transform({ 0.0f, 1.0f, -7.0f }, rot));

	Light_SetPointLight(0, pp0, 5.0f, { 1.0f, 0.0f, 0.0f });
	Light_SetPointLight(1, pp1, 6.0f, { 0.0f, 1.0f, 0.0f });
	Light_SetPointLight(2, pp2, 7.0f, { 0.0f, 0.0f, 1.0f });

	Enemy_Draw();
	Monster_Draw();
	Map_Draw();
	Bullet_Draw();
	//Cube_Draw(2, XMMatrixTranslationFromVector(vtest));
	Light_SetLimLight({ 1.0f, 0.8f, 0.8f }, 3.2f);
	Player_Draw();
	Light_SetLimLight({ 0.0f, 0.0f, 0.0f }, 0.0f);

	// 半透明ビルボード描画の準備
	Direct3D_SetDepthWriteDisable();

	// プレイヤー上にビルボード
	//XMFLOAT3 playerPos = Player_GetPosition();
	//XMFLOAT3 billPos = { playerPos.x, playerPos.y + 3.0f, playerPos.z };
	//Billboard_Draw(g_BillboardTex, billPos, { 2.0f, 2.0f });

	Direct3D_SetDepthEnable(true);

	// エフェクトの描画処理
	// Direct3D_SetAlphaBlendAdd();
	Direct3D_SetDepthWriteDisable();
	BulletHitEffect_Draw();
	Trajectory3d_Draw();
	Direct3D_SetDepthEnable(true);
	// Direct3D_SetAlphaBlendTransparent();

	// リソースパーティクル
	//Direct3D_SetDepthWriteDisable();
	//Direct3D_SetAlphaBlendAdd();
	//g_Emitter->Draw();

	/* ここから2DのUI描画 */
	Direct3D_SetDepthEnable(false);

	Sprite_Begin();

	// マウス座標の表示やキューブ座標の2Dスプライト描画処理
	XMFLOAT3 cube_pos;
	XMStoreFloat3(&cube_pos, vtest);
	//XMFLOAT2 pos = Direct3D_WorldToScreen(cube_pos,mtxView,PlayerCamera_GetPerspectiveMatrix());
	//Sprite_Draw(g_TestTex01, pos.x, pos.y, 128, 128);

	Direct3D_SetAlphaBlendTransparent();

	// マップの描画
	Direct3D_SetOffscreenTexture(0);
	//Sprite_DrawCircle(Direct3D_GetBackBufferWidht() * 0.8f, Direct3D_GetBackBufferHeight() * 0.04f, 256, 256);
	Sprite_Draw(Direct3D_GetBackBufferWidth() * 0.8f, Direct3D_GetBackBufferHeight() * 0.04f, 256, 256);

	// ステータス画面オーバーレイ（Mキーで表示）
	if (g_ShowStatus && g_StatusWhiteTex >= 0) {
		float sw = (float)Direct3D_GetBackBufferWidth();
		float sh = (float)Direct3D_GetBackBufferHeight();

		int party_count = Party_GetCount();
		int activeFighter = Party_GetActiveFighter();

		// パネルサイズ計算（戦闘キャラ詳細 + プレイヤー簡易 + パーティ一覧 + 操作ガイド）
		float panel_h = 200.0f;  // 戦闘キャラステータス基本部分（余裕あり）
		if (activeFighter >= 0) panel_h += 55.0f;  // プレイヤー簡易表示分
		if (party_count > 0) panel_h += 35.0f + party_count * 24.0f;  // パーティ一覧
		if (g_StatusConfirm) panel_h += 60.0f;  // 確認ダイアログ分
		else panel_h += 35.0f;  // 操作ガイド分

		// 背景パネル（半透明ダークブルー）
		Sprite_Draw(g_StatusWhiteTex,
			sw * 0.5f - 220.0f, sh * 0.5f - 155.0f,
			440.0f, panel_h,
			{ 0.0f, 0.0f, 0.15f, 0.88f });
		// ステータステキスト（TextTextureで日本語対応）
		{
			int fsize = 18;
			float lineH = 24.0f;
			float textX = sw * 0.5f - 185.0f;
			float textY = sh * 0.5f - 135.0f;
			char buf[256];
			int tid;
			XMFLOAT4 white = { 1.0f, 1.0f, 1.0f, 1.0f };

			// 戦闘キャラ名（日本語）
			snprintf(buf, sizeof(buf), "-- %s --", Party_GetFighterName());
			tid = TextTexture_Create(buf, fsize);
			if (tid >= 0) Sprite_Draw(tid, textX, textY, white);
			textY += lineH;

			snprintf(buf, sizeof(buf), "Lv  : %d", Party_GetFighterLevel());
			tid = TextTexture_Create(buf, fsize);
			if (tid >= 0) Sprite_Draw(tid, textX, textY, white);
			textY += lineH;

			snprintf(buf, sizeof(buf), "HP  : %d/%d", Party_GetFighterHp(), Party_GetFighterHpMax());
			tid = TextTexture_Create(buf, fsize);
			if (tid >= 0) Sprite_Draw(tid, textX, textY, white);
			textY += lineH;

			snprintf(buf, sizeof(buf), "ATK : %d", Party_GetFighterAtk());
			tid = TextTexture_Create(buf, fsize);
			if (tid >= 0) Sprite_Draw(tid, textX, textY, white);
			textY += lineH;

			snprintf(buf, sizeof(buf), "DEF : %d", Party_GetFighterDef());
			tid = TextTexture_Create(buf, fsize);
			if (tid >= 0) Sprite_Draw(tid, textX, textY, white);
			textY += lineH;

			// EXP表示
			if (activeFighter >= 0) {
				const PartyMonster* pm = Party_Get(activeFighter);
				if (pm) {
					snprintf(buf, sizeof(buf), "EXP : %d/%d", pm->exp, pm->exp_next);
					tid = TextTexture_Create(buf, fsize);
					if (tid >= 0) Sprite_Draw(tid, textX, textY, white);
					textY += lineH;
				}
			} else {
				snprintf(buf, sizeof(buf), "EXP : %d/%d", Player_GetExp(), Player_GetExpNext());
				tid = TextTexture_Create(buf, fsize);
				if (tid >= 0) Sprite_Draw(tid, textX, textY, white);
				textY += lineH;
			}

			// プレイヤーが戦闘キャラでない場合
			if (activeFighter >= 0) {
				snprintf(buf, sizeof(buf), "-- プレイヤー --");
				tid = TextTexture_Create(buf, fsize);
				if (tid >= 0) Sprite_Draw(tid, textX, textY, white);
				textY += lineH;

				snprintf(buf, sizeof(buf), "Lv%d HP:%d/%d ATK:%d DEF:%d",
					Player_GetLevel(), Player_GetHp(), Player_GetHpMax(),
					Player_GetAtk(), Player_GetDef());
				tid = TextTexture_Create(buf, fsize);
				if (tid >= 0) Sprite_Draw(tid, textX, textY, white);
				textY += lineH;
			}

			// パーティ一覧
			if (party_count > 0) {
				snprintf(buf, sizeof(buf), "-- パーティ (%d/%d) --", party_count, Party_GetMax());
				tid = TextTexture_Create(buf, fsize);
				if (tid >= 0) Sprite_Draw(tid, textX, textY, white);
				textY += lineH;

				for (int i = 0; i < party_count; i++) {
					const PartyMonster* pm = Party_Get(i);
					if (pm) {
						// カーソル選択中は黄色ハイライト
						bool isCursorOn = (i == g_StatusCursor);
						XMFLOAT4 textColor = isCursorOn ? XMFLOAT4{ 1.0f, 1.0f, 0.3f, 1.0f } : white;

						const char* mark = isCursorOn ? ">" : (i == activeFighter) ? "*" : " ";
						snprintf(buf, sizeof(buf), "%s%s Lv%d HP%d/%d",
							mark, Party_GetKindName(pm->kind), pm->level,
							pm->hp, pm->hp_max);
						tid = TextTexture_Create(buf, fsize);
						if (tid >= 0) Sprite_Draw(tid, textX, textY, textColor);
						textY += lineH;
					}
				}
			}

			// 削除確認ダイアログ
			if (g_StatusConfirm && g_StatusCursor >= 0) {
				const PartyMonster* selPm = Party_Get(g_StatusCursor);
				if (selPm) {
					textY += 4.0f;
					XMFLOAT4 red = { 1.0f, 0.4f, 0.4f, 1.0f };
					snprintf(buf, sizeof(buf), "%sを逃がしますか？", Party_GetKindName(selPm->kind));
					tid = TextTexture_Create(buf, fsize);
					if (tid >= 0) Sprite_Draw(tid, textX, textY, red);
					textY += lineH;
					snprintf(buf, sizeof(buf), "[Y] はい  [N] いいえ");
					tid = TextTexture_Create(buf, fsize);
					if (tid >= 0) Sprite_Draw(tid, textX, textY, red);
					textY += lineH;
				}
			} else {
				// 操作ガイド
				textY += 4.0f;
				XMFLOAT4 gray = { 0.7f, 0.7f, 0.7f, 1.0f };
				snprintf(buf, sizeof(buf), "[↑↓] 選択 [X] 逃がす [M] とじる");
				tid = TextTexture_Create(buf, fsize);
				if (tid >= 0) Sprite_Draw(tid, textX, textY, gray);
			}
		}
	}

	// フィールドHUD: パーティアイコン表示（画面左上）
	{
		float iconSize = 48.0f;
		float iconPad = 6.0f;
		float hudX = 10.0f;
		float hudY = 35.0f;
		int activeFighter = Party_GetActiveFighter();

		// プレイヤー（スライム）アイコン
		bool playerActive = (activeFighter == -1);
		if (playerActive) {
			Sprite_Draw(g_StatusWhiteTex, hudX - 3.0f, hudY - 3.0f,
				iconSize + 6.0f, iconSize + 6.0f, { 1.0f, 1.0f, 0.0f, 0.8f });
		}
		if (Player_GetHp() <= 0)
			Sprite_Draw(g_IconSlimeTex, hudX, hudY, iconSize, iconSize, { 0.3f, 0.3f, 0.3f, 0.5f });
		else if (playerActive)
			Sprite_Draw(g_IconSlimeTex, hudX, hudY, iconSize, iconSize, { 1.0f, 1.0f, 1.0f, 1.0f });
		else
			Sprite_Draw(g_IconSlimeTex, hudX, hudY, iconSize, iconSize, { 0.6f, 0.6f, 0.6f, 0.8f });
		hudX += iconSize + iconPad;

		// パーティメンバーアイコン
		for (int i = 0; i < Party_GetCount(); i++) {
			const PartyMonster* pm = Party_Get(i);
			if (!pm) continue;
			int iconTex = GetMonsterIconTex(pm->kind);
			bool isActive = (i == activeFighter);

			// アクティブ枠（黄色ハイライト）
			if (isActive) {
				Sprite_Draw(g_StatusWhiteTex, hudX - 3.0f, hudY - 3.0f,
					iconSize + 6.0f, iconSize + 6.0f, { 1.0f, 1.0f, 0.0f, 0.8f });
			}

			if (iconTex >= 0) {
				float alpha = (pm->hp > 0) ? 1.0f : 0.5f;
				if (isActive)
					Sprite_Draw(iconTex, hudX, hudY, iconSize, iconSize, { 1.0f, 1.0f, 1.0f, alpha });
				else
					Sprite_Draw(iconTex, hudX, hudY, iconSize, iconSize, { 0.6f, 0.6f, 0.6f, alpha * 0.8f });
			} else {
				// アイコン未作成時の代替表示
				Sprite_Draw(g_StatusWhiteTex, hudX, hudY, iconSize, iconSize, { 0.4f, 0.4f, 0.4f, 0.7f });
			}
			hudX += iconSize + iconPad;
		}
	}

	Direct3D_SetDepthEnable(true);

	// デバッグカメラの描画
	if (g_IsDebug) {
		//Camera_DebugDraw();
	}
}

void mapRendering()
{
	// レンダーターゲットをテクスチャへ
	Direct3D_SetOffscreen();
	Direct3D_ClearOffscreen();

	// マップ用カメラ（行列）設定
	XMFLOAT3 position = Player_GetPosition();
	position.y = 150.0f;
	MapCamera_SetPosition(position);
	MapCamera_SetFront({0.0f, 0.0f, 1.0f});  // ミニマップは常に北向き固定
	XMFLOAT4X4 mtxView = MapCamera_GetViewMatrix();
	XMFLOAT4X4 mtxProj = MapCamera_GetProjectionMatrix();
	XMMATRIX view = XMLoadFloat4x4(&mtxView);
	XMMATRIX proj = XMLoadFloat4x4(&mtxProj);

	// カメラのビュー行列をシェーダーへ設定
	Camera_SetMatrix(view, proj);

	// テクスチャーサンプラー設定
	Sampler_SetFilterAnisotropic();

	// マップ用ライト
	// ディレクショナルライト（色と方向を設定する必要あり、アンビエントライト設定含む）
	// ディレクショナルライトを与えるだけでは、ライティングはゲーム中と同じにならない
	Light_SetAmbient({ 0.3f,0.3f,0.3f });
	Light_SetDirectionalWorld({ 0.0f,-1.0f,0.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f });
	Light_SetLimLight({ 0.0f,0.0f,0.0f }, 0.0f);

	// 深度有効
	Direct3D_SetDepthEnable(true);
	// ミニマップモードON（影の描画を無効化、天井スキップ）
	CircleShadow_SetMinimapMode(true);
	Map_SetMinimapMode(true);

	Enemy_Draw();
	Monster_Draw();
	Player_Draw();
	Map_Draw();

	// ミニマップモードOFF（通常描画に戻す）
	Map_SetMinimapMode(false);
	CircleShadow_SetMinimapMode(false);
}


void lightRendering()
{
	// レンダーターゲットをテクスチャへ
	Direct3D_SetDepth();
	Direct3D_ClearDepth();

	// ライトカメラ（行列）設定
	XMFLOAT4X4 mtxView = LightCamera_GetViewMatrix();
	XMFLOAT4X4 mtxProj = LightCamera_GetProjectionMatrix();
	XMMATRIX view = XMLoadFloat4x4(&mtxView);
	XMMATRIX proj = XMLoadFloat4x4(&mtxProj);

	// カメラのビュー行列をシェーダーへ設定
	Camera_SetMatrix(view, proj);

	// 深度有効
	Direct3D_SetDepthEnable(true);

	// キャスト（影を落とすオブジェクト）
	//Enemy_DepthDraw();
	//Player_DepthDraw();
	//Map_Draw();
}

int Game_GetStage()
{
	return g_CurrentStage;
}

