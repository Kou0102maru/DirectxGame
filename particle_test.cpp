#include "particle_test.h"
#include"billboard.h"
using namespace DirectX;

double easeInCirc(double t) {
	return 1 - sqrt(1 - t);
}

void NormalParticle::Update(double elapsed_time)
{
	float ratio = std::fminf(GetAccumulatedTime() / GetLifeTime(), 1.0f);

	m_scale = (1.0f - ratio) * 0.5f;
	m_alpha = (1.0f - easeInCirc(ratio));

	AddPosition(GetVelocity() * elapsed_time);
	//AddVelocity(XMVECTOR{ 0.0f,-5.0f,0.0f }*elapsed_time);


	Particle::Update(elapsed_time);
}

void NormalParticle::Draw() const
{
	XMFLOAT3 position{};
	XMStoreFloat3(&position, GetPosition());

	Billboard_Draw(m_texture_id, position, { m_scale,m_scale }, XMFLOAT4{ 0.3f,1.0f,0.1f,m_alpha });
}

Particle* NormalEmitter::createParticle()
{
	std::uniform_real_distribution<float> m_dir_dist{ -DirectX::XM_PI, DirectX::XM_PI };
	std::uniform_real_distribution<float>m_length_dist{ 2.0f,2.1f };
	std::uniform_real_distribution<float> m_speed_dist{ 0.05f,0.1f };
	std::uniform_real_distribution<float> m_lifetime_dist{ 0.5f,1.0f };

	float angle = m_dir_dist(m_mt);
	XMVECTOR c{ cosf(angle),0.0f,sinf(angle) };
	c *= m_length_dist(m_mt);
	c += GetPosition();
	XMVECTOR velocity{ 0.0f,1.0f,0.0f };

	/*float angle = m_dir_dist(m_mt);
	float ey = XMVectorGetY(GetPosition());
	XMVECTOR c{ cosf(angle),0.0f,sinf(angle) };
	c += GetPosition();
	c *= m_length_dist(m_mt);
	c += {0.0f, 10.0f, 0.0f};
	XMVECTOR direction{ c - GetPosition() };
	direction = XMVector3Normalize(direction);
	XMVECTOR velocity{ direction * m_speed_dist(m_mt) };
	*/

	return new NormalParticle(m_texture_id, c, velocity, m_lifetime_dist(m_mt));
}

