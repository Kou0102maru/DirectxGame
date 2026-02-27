/*==============================================================================

   ìGÇÃêßå‰ [enemy.h]
														 Author : Youhei Sato
														 Date   : 2025/11/26
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef ENEMY_H
#define ENEMY_H

#include <DirectXMath.h>
#include "collision.h"


class Enemy
{
protected:
	class State
	{
	public:
		virtual ~State() = default;
		virtual void Update(double elapsed_time) = 0;
		virtual void Draw() const = 0;
		virtual void DepthDraw() const = 0;
	};

private:
	State* m_pState{};
	State* m_pNextState{};

public:
	virtual ~Enemy() = default;
	virtual void Update(double elapsed_time);
	virtual void Draw() const;
	virtual void DepthDraw() const;
	void UpdateState();
	virtual void Damage(int) {}
	virtual bool IsDestroy() const = 0;
	virtual Sphere GetCollision() const { return {}; }

protected:
	void ChangeState(State* pNext);
};

void Enemy_Initialize();
void Enemy_Finalize();
void Enemy_Update(double elapsed_time);
void Enemy_Draw();
void Enemy_DepthDraw();
void Enemy_Create(const DirectX::XMFLOAT3& position);
int Enemy_GetEnemyCount();
Enemy* Enemy_GetEnemy(int index);

#endif // ENEMY_H
