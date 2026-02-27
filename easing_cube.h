#pragma once
#include<DirectXMath.h>

class EasingCube
{
private:
	double m_AccumulaedTime{};
	double m_MoveTime{};
	DirectX::XMFLOAT3 m_StartPosition{};
	DirectX::XMFLOAT3 m_EndPosition{};
	bool m_IsStart{};

private:
	double easeOutCubic(double t)const {
		return 1 + (--t) * t * t;
	}

public:
	EasingCube(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, double time)
		: m_StartPosition(start), m_EndPosition(end), m_MoveTime(time)
	{
	}

	void Start() { m_IsStart = true; }
	void Reset() { m_IsStart = false, m_AccumulaedTime = 0.0; }
	void Update(double elapsed_time);
	void Draw() const;
};
