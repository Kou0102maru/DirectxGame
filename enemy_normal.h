/*==============================================================================

   ïÅí ÇÃìGêßå‰ [enemy_normal.h]
														 Author : Youhei Sato
														 Date   : 2025/11/26
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef ENEMY_NORMAL_H
#define ENEMY_NORMAL_H

#include "enemy.h"
#include <DirectXMath.h>
#include "texture.h"


class EnemyNormal : public Enemy
{
private:
	DirectX::XMFLOAT3 m_Position{};
	float m_DetectionRadius{ 10.0f };
	int m_Hp{ 100 };
	int m_TexWhiteId{};
	int m_TexRedId{};

public:
	EnemyNormal(const DirectX::XMFLOAT3& position) : m_Position(position) {
		m_TexWhiteId = Texture_Load(L"resource/texture/white.png");
		m_TexRedId = Texture_Load(L"resource/texture/red.png");
		ChangeState(new EnemyNormalStatePatrol(this));
	}

	void Damage(int damage) override {
		m_Hp -= damage;
	}

	bool IsDestroy() const override {
		return m_Hp <= 0;
	}

	Sphere GetCollision() const {
		return { m_Position, 3.0f };
	}

private:
	class EnemyNormalStatePatrol : public State
	{
	private:
		EnemyNormal* m_pOwner{};
		float m_PointX{};
		double g_AccumulatedTime{};

	public:
		EnemyNormalStatePatrol(EnemyNormal* pOwner) 
			: m_pOwner(pOwner) 
			, m_PointX(pOwner->m_Position.x)
		{
		}

		void Update(double elapsed_time) override;
		void Draw() const override;
		void DepthDraw() const override;
	};

	class EnemyNormalStateChase: public State
	{
	private:
		EnemyNormal* m_pOwner{};
		double g_AccumulatedTime{};

	public:
		EnemyNormalStateChase(EnemyNormal* pOwner)
			: m_pOwner(pOwner)
		{
		}

		void Update(double elapsed_time) override;
		void Draw() const override;
		void DepthDraw() const override;
	};
};




#endif // ENEMY_NORMAL_H
