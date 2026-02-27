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
#include"monster_slime.h"
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

static double g_AccumulatedTime = 0.0;
static bool g_IsDebug = false;
static int g_TestTex01{ -1 };
//static int g_BillboardTex{ -1 };

void mapRendering();
void lightRendering();

static EasingCube g_ecube({ -10.0f,1.0f,10.0f }, { 10.0f,1.0f,10.0f }, 2.0);
static NormalEmitter* g_Emitter;

void Game_Initialize()
{
	// ★戦闘から戻る時は、保存した位置で初期化★
	extern XMFLOAT3 g_PlayerSavePosition;  // battle.cpp で保存した位置
	extern bool g_ReturnFromBattle;

	if (g_ReturnFromBattle) {
		Player_Initialize(g_PlayerSavePosition, { 0.0f, 0.0f, 1.0f });
	}
	else {
		Player_Initialize({ 0.0f, 0.5f, 5.0f }, { 0.0f, 0.0f, 1.0f });
	}

	Enemy_Initialize();
	Monster_Initialize();
	Camera_Initialize({ 8.2f, 8.4f, -12.7f }, { -0.5f, -0.3f, 0.7f }, { 0.8f, 0.0f, 0.5f });
	PlayerCamera_Initialize();	
	Map_Initialize();
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

	// スライム（左右にパトロール）
	Monster_CreateSlime({ 5.0f, 0.5f, 5.0f }, 1);
	Monster_CreateSlime({ -5.0f, 0.5f, 10.0f }, 1);
	Monster_CreateSlime({ 10.0f, 0.5f, 15.0f }, 1);

	// オオカミ（速く走り回る）
	Monster_CreateWolf({ -10.0f, 0.5f, 5.0f }, 3);
	Monster_CreateWolf({ 15.0f, 0.5f, 20.0f }, 3);

	// ドラゴン（空中を旋回）
	Monster_CreateDragon({ 0.0f, 5.0f, 30.0f }, 5);

	//g_TestTex01 = Texture_Load(L"resource/texture/knight.png");
	//g_BillboardTex = Texture_Load(L"resource/texture/knight.png");
	g_IsDebug = false;

	g_ReturnFromBattle = false;
}


void Game_Finalize()
{
	//delete g_Emitter;
	//g_Emitter = nullptr;

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
			Battle_SetEnemy(monster);
			Scene_Change(SCENE_BATTLE);
			return; 
		}
	}

	Enemy_Update(elapsed_time);
	Monster_Update(elapsed_time);
	Sky_SetPosition(Player_GetPosition());

	if (g_IsDebug) {
		Camera_Update(elapsed_time);
	}
	else {
		PlayerCamera_Update(elapsed_time);
	}

	Bullet_Update(elapsed_time);

	// 弾とマップとの当たり判定（AABB vs AABB）
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

	// 敵と弾との当たり判定
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
	mapRendering();//オフスクリーンへレンダリング
	//lightRendering();
	//Direct3D_SetDepthTexture(2);
	//レンダーターゲットをバックバッファへ
	
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

	// yが0になる場所を求める
	XMVECTOR vtest = XMLoadFloat3(&test_far) - XMLoadFloat3(&test_near); // nearからfarのベクトルを求める
	vtest = XMVector3Normalize(vtest); // 単位ベクトルにする

	// nearのyをnearからfar向きの単位ベクトルのyで割って
	// y = 0になる比率（何倍すれば0になるか）を算出する※下向きなのでマイナスにしておく
	float ratio = -XMVectorGetY(XMLoadFloat3(&test_near)) / XMVectorGetY(vtest); 
	vtest = XMLoadFloat3(&test_near) + vtest * ratio; // near座標 + farへの単位ベクトル * y = 0になる長さ（比率）

	// カメラに関する行列をシェーダーに設定する
	Camera_SetMatrix(view, proj);

	// ビルボードにカメラの行列を設定する
	Billboard_SetViewMatrix(mtxView);

	// テクスチャーサンプラーの設定
	Sampler_SetFilterAnisotropic();
	
	// 空の表示
	Direct3D_SetDepthEnable(false);
	Sky_Draw();
	Direct3D_SetDepthEnable(true);

	// 各種ライトの設定
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

	// ★ここにビルボード描画を追加★
	Direct3D_SetDepthWriteDisable();

	// プレイヤー頭上にビルボード
	//XMFLOAT3 playerPos = Player_GetPosition();
	//XMFLOAT3 billPos = { playerPos.x, playerPos.y + 3.0f, playerPos.z };
	//Billboard_Draw(g_BillboardTex, billPos, { 2.0f, 2.0f });

	Direct3D_SetDepthEnable(true);

	//エフェクト類の描画実験
	// Direct3D_SetAlphaBlendAdd();
	Direct3D_SetDepthWriteDisable();
	BulletHitEffect_Draw();
	Trajectory3d_Draw();
	Direct3D_SetDepthEnable(true);
	// Direct3D_SetAlphaBlendTransparent();
	
	//緑のサークルパーティクル
	//Direct3D_SetDepthWriteDisable();
	//Direct3D_SetAlphaBlendAdd();
	//g_Emitter->Draw();

	/*ここから２DにUI描画*/
	Direct3D_SetDepthEnable(false);

	Sprite_Begin();

	//マウス座標に表示しているキューブの座標に2Dスプライトの描画実験
	XMFLOAT3 cube_pos;
	XMStoreFloat3(&cube_pos, vtest);
	//XMFLOAT2 pos = Direct3D_WorldToScreen(cube_pos,mtxView,PlayerCamera_GetPerspectiveMatrix());
	//Sprite_Draw(g_TestTex01, pos.x, pos.y, 128, 128);

	Direct3D_SetAlphaBlendTransparent();

	//マップの描画
	Direct3D_SetOffscreenTexture(0);
	//Sprite_DrawCircle(Direct3D_GetBackBufferWidht() * 0.8f, Direct3D_GetBackBufferHeight() * 0.04f, 256, 256);
	Sprite_Draw(Direct3D_GetBackBufferWidth() * 0.8f, Direct3D_GetBackBufferHeight() * 0.04f, 256, 256);
	
	Direct3D_SetDepthEnable(true);

	//デバック文字の描画
	if (g_IsDebug) {
		//Camera_DebugDraw();
	}
}

void mapRendering()
{
	//レンダーターゲットをテクスチャへ
	Direct3D_SetOffscreen();
	Direct3D_ClearOffscreen();

	//マップ用カメラ（行列）の設定
	XMFLOAT3 position = Player_GetPosition();
	position.y = 150.0f;
	MapCamera_SetPosition(position);
	MapCamera_SetFront(Player_GetFront());
	XMFLOAT4X4 mtxView = MapCamera_GetViewMatrix();
	XMFLOAT4X4 mtxProj = MapCamera_GetProjectionMatrix();
	XMMATRIX view = XMLoadFloat4x4(&mtxView);
	XMMATRIX proj = XMLoadFloat4x4(&mtxProj);

	//カメラに関する行列をシェーダーに設定
	Camera_SetMatrix(view, proj);

	//テクスチャーサンプラーの設定
	Sampler_SetFilterAnisotropic();

	//マップ用ライト
	//※ディレクショナルライトの色を真っ黒にしてしまって、アンビエントライトのみにするか
	// 　ディレクショナルライトを真下に向けるか、ライティングはゲームそのままにするか
	Light_SetAmbient({ 0.3f,0.3f,0.3f });
	Light_SetDirectionalWorld({ 0.0f,-1.0f,0.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f });
	Light_SetLimLight({ 0.0f,0.0f,0.0f }, 0.0f);

	//深度有効
	Direct3D_SetDepthEnable(true);
	// ミニマップモードON（影を描画しない）
	CircleShadow_SetMinimapMode(true);

	Enemy_Draw();
	Monster_Draw();
	Player_Draw();
	Map_Draw();

	// ミニマップモードOFF（通常描画に戻す）
	CircleShadow_SetMinimapMode(false);
}


void lightRendering()
{
	//レンダーターゲットをテクスチャへ
	Direct3D_SetDepth();
	Direct3D_ClearDepth();

	//ライトカメラ（行列）の設定
	XMFLOAT4X4 mtxView = LightCamera_GetViewMatrix();
	XMFLOAT4X4 mtxProj = LightCamera_GetProjectionMatrix();
	XMMATRIX view = XMLoadFloat4x4(&mtxView);
	XMMATRIX proj = XMLoadFloat4x4(&mtxProj);

	//カメラに関する行列をシェーダーに設定
	Camera_SetMatrix(view, proj);

	//深度有効
	Direct3D_SetDepthEnable(true);

	//キャスト（影を落とすオブジェクト）
	//Enemy_DepthDraw();
	//Player_DepthDraw();
	//Map_Draw();
}
	
