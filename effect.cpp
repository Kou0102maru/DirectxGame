/*==============================================================================

   エフェクトの描画 [effect.cpp]
														 Author : Youhei Sato
														 Date   : 2025/07/04
--------------------------------------------------------------------------------

==============================================================================*/
#include "effect.h"
#include <DirectXMath.h>
using namespace DirectX;
#include "texture.h"
#include "sprite_anim.h"
#include "Audio.h"


struct Effect
{
	XMFLOAT2 position;
	// XMFLOAT2 velocity;
	// double lifeTime;
	int sprite_anim_id;
	bool isEnable;
};


static constexpr int EFFECT_MAX = 256;
static Effect g_Effects[EFFECT_MAX]{};
static int g_EffectTexId = -1;
static int g_AnimPatternId = -1;
static int g_EffectSoundId = -1;


void Effect_Initialize()
{
	for (Effect& e : g_Effects) {
		e.isEnable = false;
	}

	// g_EffectTexId = Texture_Load(L"resource/texture/explosion.png");
	// g_AnimPatternId = SpriteAnim_RegisterPattern(g_EffectTexId, 16, 4, 0.02, { 256, 256 }, { 0, 0 }, false);
	g_EffectTexId = Texture_Load(L"resource/texture/ObjBase.png");
	g_EffectSoundId = LoadAudio("resource/sound/explosion000.wav");
	g_AnimPatternId = SpriteAnim_RegisterPattern(g_EffectTexId, 6, 8, 0.1, { 32, 32 }, { 0, 64 }, false);
}

void Effect_Finalize()
{
	UnloadAudio(g_EffectSoundId);
}

void Effect_Update()
{
	for (Effect& e : g_Effects) {

		if (!e.isEnable) continue;

  		if (SpriteAnim_IsStopped(e.sprite_anim_id)) {
			SpriteAnim_DestroyPlayer(e.sprite_anim_id);
  			e.isEnable = false;
		}
	}
}

void Effect_Draw()
{
	for (Effect& e : g_Effects) {

		if (!e.isEnable) continue;

		SpriteAnim_Draw(e.sprite_anim_id, e.position.x, e.position.y, 64.0f, 64.0f);
	}
}

void Effect_Create(const DirectX::XMFLOAT2& position)
{
	for (Effect& e : g_Effects) {

		if (e.isEnable) continue;

		// 空き領域発見
		e.isEnable = true;
		e.position = position;
		e.sprite_anim_id = SpriteAnim_CreatePlayer(g_AnimPatternId);
		PlayAudio(g_EffectSoundId);
		break;
	}
}
