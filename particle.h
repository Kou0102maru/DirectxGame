/*==============================================================================

  パーティクル（粒子）[particle.h]
														 Author : Kotaro Marugami
														 Date   : 2025/02/04
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef PARTICLE_H
#define PARTICLE_H

//インスタンシング
// ジオメトリシェーダー
//GPUパーティクル
#include<DirectXMath.h>



class Particle
{
private:
	DirectX::XMVECTOR m_position{};
	DirectX::XMVECTOR m_velocity{};
	//float m_mass;
	double m_life_time{};
	double m_accumlated_time{};

public:
	Particle(const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& velocity, double life_time)
		:m_position(position)
		, m_velocity(velocity)
		, m_life_time(life_time) {
	}

	virtual ~Particle() = default;

	virtual bool IsDestroy() const {
		return m_life_time <= 0.0;
	}

	virtual  void Update(double elapsed_time) {
		m_accumlated_time += elapsed_time;
		if (m_accumlated_time > m_life_time) {
			Destroy();
		}
	}
	virtual void Draw() const = 0;

protected:
	virtual void Destroy() {
		m_life_time = 0.0;
	}

	void SetPosition(const DirectX::XMVECTOR& position) {
		m_position = position;
	}

	void SetVelocity(const DirectX::XMVECTOR& velocity) {
		m_velocity = velocity;
	}

	const DirectX::XMVECTOR& GetPosition() const {
		return m_position;
	}

	const DirectX::XMVECTOR& GetVelocity() const {
		return m_velocity;
	}

	void AddPosition(const DirectX::XMVECTOR& v) {
		m_position = DirectX::XMVectorAdd(m_position, v);
	}

	void AddVelocity(const DirectX::XMVECTOR& v) {
		m_velocity = DirectX::XMVectorAdd(m_velocity, v);
	}

	float GetAccumulatedTime() const {
		return m_accumlated_time;
	}

	float GetLifeTime() const {
		return m_life_time;
	}

};


class Emitter //噴出機
{
private:
	DirectX::XMVECTOR m_position{};
	double m_particles_per_second{};
	double m_accumlated_time = 0.0; //累積時間
	bool m_is_emmit{};
	size_t m_capacity{}; //パーティクル管理最大量
	size_t m_count{}; //現在のパーティクル数
	Particle** m_particles{};

protected:
	virtual Particle* createParticle() = 0;

	const DirectX::XMVECTOR& GetPosition() const {
		return m_position;
	}

	double GetParticlesPerSecond() const {
		return m_particles_per_second;
	}

	double GetAccumlatedTime() const {
		return m_accumlated_time;
	}

	void SetParticlesPerSecond(double particlespersecond){
		m_particles_per_second = particlespersecond;
	}

	void SetAccumlatedTime(double accumlatedtime){
		m_accumlated_time = accumlatedtime;
	}
public:
	Emitter(size_t capacity, const DirectX::XMVECTOR& position, double particles_per_second, bool is_emmit = false)
		:m_capacity(capacity)
		,m_position(position)
		, m_particles_per_second(particles_per_second)
		, m_is_emmit(is_emmit) {

		m_particles = new Particle*[m_capacity];

		for (int i = 0;i < m_capacity;i++) {
			m_particles[i] = nullptr;
		}
}

	virtual ~Emitter() {
		delete[] m_particles;
	}

	virtual void Update(double elapsed_time);
	virtual void Draw()const;

	void SetPosition(const DirectX::XMVECTOR& position) {
		m_position = position;
	}

	void Emmit(bool isEmmit) { m_is_emmit = isEmmit; }
	bool IsEmmit()const { return m_is_emmit; }
};

#endif // PARTICLE_H