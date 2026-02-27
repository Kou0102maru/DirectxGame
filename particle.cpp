#include "particle.h"
using namespace DirectX;

void Emitter::Update(double elapsed_time)
{
	m_accumlated_time += elapsed_time;

	const double sec_per_particle = 1.0 / m_particles_per_second;

	//•¬oˆ—
	while (m_accumlated_time >= sec_per_particle) {
		if (m_count >= m_capacity)break;
		if (m_is_emmit) {
			m_particles[m_count++] = createParticle();
		}
		m_accumlated_time -= sec_per_particle;
	}

	//XVˆ—
	for (int i = 0;i < m_count;i++) {
		m_particles[i]->Update(elapsed_time);
	}

	//õ–½ˆ—
	for (int i = m_count - 1;i >= 0;i--) {
		if (m_particles[i]->IsDestroy()) {
			delete m_particles[i];
			m_particles[i] = m_particles[--m_count];
		}
	}
}

void Emitter::Draw() const
{

	for (int i = 0;i < m_count;i++) {
		m_particles[i]->Draw();
	}
}
