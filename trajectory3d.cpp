/*==============================================================================

   軌跡エフェクトの描画 [trajectory3d.cpp]
														 Author : Youhei Sato
														 Date   : 2025/11/21
--------------------------------------------------------------------------------

==============================================================================*/
#include "trajectory3d.h"
#include "texture.h"
#include "billboard.h"
#include "direct3d.h"
using namespace DirectX;
#include "shader_billboard.h"


struct Trajectory3d
{
	XMFLOAT3 position;
	XMFLOAT4 color;
	float size;
	double lifeTime;
	double birthTime; // 0.0だったら未使用
};

static constexpr unsigned int TRAJECTORY_MAX = 4096;
static Trajectory3d g_Trajectorys[TRAJECTORY_MAX] = {};
static int g_TrajectoryTexId = -1;
static double g_Time = 0.0;


void Trajectory3d_Initialize()
{
	for (Trajectory3d& t : g_Trajectorys) {
		t.birthTime = 0.0;
	}

	g_Time = 0.0;

	g_TrajectoryTexId = Texture_Load(L"resource/texture/effect000.jpg");
}

void Trajectory3d_Finalize()
{
}

void Trajectory3d_Update(double elapsed_time)
{
	for (Trajectory3d& t : g_Trajectorys) {

		if (t.birthTime == 0.0) continue;

		double time = g_Time - t.birthTime;

		if (time > t.lifeTime) {
			t.birthTime = 0.0; // 寿命が付きた
		}
	}

	g_Time += elapsed_time; // 時間の更新
}

void Trajectory3d_Draw()
{
	Direct3D_SetAlphaBlendAdd();

	for (const Trajectory3d& t : g_Trajectorys) {

		if (t.birthTime == 0.0) continue;

		double time = g_Time - t.birthTime;
		float ratio = (float)(time / t.lifeTime);
		float size = t.size * (1.0f - ratio);
		XMFLOAT4 color = t.color;
		color.w = t.color.w * (1.0f - ratio);

		Billboard_Draw(g_TrajectoryTexId, t.position, { size, size }, color);
	}

	Direct3D_SetAlphaBlendTransparent();
}

void Trajectory3d_Create(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color, float size, double lifeTime)
{
	for (Trajectory3d& t : g_Trajectorys) {

		if (t.birthTime != 0.0) continue;

		t.birthTime = g_Time;
		t.lifeTime = lifeTime;
		t.color = color;
		t.position = position;
		t.size = size;
		break;
	}
}
