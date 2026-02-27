/*==============================================================================

  パーティクルのテスト[particle_test.h]
														 Author : Kotaro Marugami
														 Date   : 2025/02/04
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef PARTICLE_TEST_H
#define PARTICLE_TEST_H

#include"particle.h"
#include"texture.h"
#include<random>

class NormalParticle : public Particle
{
private:
	int m_texture_id{ -1 };
	float m_scale{ 1.0f };
	float m_alpha{ 0.0f };
public:
	NormalParticle(int texture_id, const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& velocity, double life_time)
		:m_texture_id(texture_id), Particle(position, velocity, life_time) {
	}

	void Update(double elapsed_time)override;
	void Draw()const override;
};

class NormalEmitter :public Emitter
{
private:
	int m_texture_id{ -1 };
	std::mt19937 m_mt;


public:
	NormalEmitter(size_t capacity, const DirectX::XMVECTOR& position, double particles_per_second, double is_emmit)
	:Emitter(capacity, position, particles_per_second, is_emmit)
	, m_texture_id(Texture_Load(L"resource/texture/effect000.jpg"))
	{
	}

protected:
	Particle* createParticle()override;
};

#endif // PARTICLE_TEST_H