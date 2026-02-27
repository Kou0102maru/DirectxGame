#include "easing_cube.h"
#include "cube.h"
using namespace DirectX;
#include<algorithm>

void EasingCube::Update(double elapsed_time)
{
	if (!m_IsStart)return;

	m_AccumulaedTime += elapsed_time;
}

void EasingCube::Draw() const
{
	XMVECTOR strat = XMLoadFloat3(&m_StartPosition);
	XMVECTOR end = XMLoadFloat3(&m_EndPosition);
	XMVECTOR v = end - strat;

	double ratio = static_cast<float>(std::min(m_AccumulaedTime / m_MoveTime, 1.0));

	float ratiof = static_cast<float>(easeOutCubic(ratio));//イージング
	//v *= ratio; //線形補間
	v *= ratiof;
	v += strat;

	XMMATRIX world = XMMatrixTranslationFromVector(v);

	Cube_Draw(2, world);
}
