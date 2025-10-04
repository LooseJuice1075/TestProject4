#pragma once

#include <iostream>
#include <windows.h>
#include "Omni.h"
#include "domain/Domain.h"
#include "scene/ScriptableEntity.h"
#include "core/Application.h"
#include "UserComponents.h"
#include "domain/DomainMap.h"
using namespace Omni;

#include "RendererDomain.h"

inline static float s_RotSpeed = 15;

class TestDomain : Omni::Domain
{
public:
	void Init(DomainConfig& domainConfig) override
	{
		std::cout << "Hello Omni! :D" << std::endl;
		m_Counter = 0;
		m_FixedUpdateCounter = 0;
		m_EnemyMoveSpeed = 3;
	}

	void OnSceneInit(Scene* scene) override
	{
		m_Scene = scene;

		//Entity player = m_Scene->GetEntity("Player");
		//m_PlayerTC = &player.GetComponent<TransformComponent>();

		//RenderCommand::SetClearColor({1.0f, 0.0f, 0.0f, 1.0f});
	}

	void Shutdown() override
	{
		std::cout << "Goodbye, cruel world!" << std::endl;
	}

	void OnSceneShutdown() override
	{
		//RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
	}

	void OnUpdateStart(Omni::Timestep ts)
	{
	}

	void OnUpdate(Omni::Timestep ts) override
	{
		//auto view = m_Scene->m_Registry.view<TransformComponent, EnemyComponent>();
		//for (auto entity : view)
		//{
		//	auto [transform, ec] = view.get<TransformComponent, EnemyComponent>(entity);
		//	Omni::Entity e = { entity, m_Scene };

		//	if (transform.Translation.x < m_PlayerTC->Translation.x)
		//	{
		//		transform.Translation.x += m_EnemyMoveSpeed * ts;
		//	}
		//	else if (transform.Translation.x > m_PlayerTC->Translation.x)
		//	{
		//		transform.Translation.x -= m_EnemyMoveSpeed * ts;
		//	}

		//	if (transform.Translation.y < m_PlayerTC->Translation.y)
		//	{
		//		transform.Translation.y += m_EnemyMoveSpeed * ts;
		//	}
		//	else if (transform.Translation.y > m_PlayerTC->Translation.y)
		//	{
		//		transform.Translation.y -= m_EnemyMoveSpeed * ts;
		//	}
		//}

		m_Counter++;
	}

	void OnUpdateBeforeRender(Omni::Timestep ts)
	{
		//DomainMap::GetActiveRendererDomain()->DrawSprite(m_PlayerTC->ToMat4(), { 1.0f, 1.0f, 1.0f, 1.0f });
	}

	void OnUpdateAfterRender(Omni::Timestep ts)
	{
	}

	void OnFixedUpdate() override
	{
		if (m_FixedUpdateCounter % 60 == 0)
		{
			for (auto entity : (*m_Scene->GetSceneOrder()))
			{
				if (entity.HasComponent<EnemyComponent>())
				{
					auto ec = &entity.GetComponent<EnemyComponent>();
				}
			}
		}

		m_FixedUpdateCounter++;
	}

	void OnImGuiRender() override
	{
		//ImGui::ShowDemoWindow();

		//ImGui::Begin("TestDomain");

		//ImGui::InputFloat("Rotation Speed", &s_RotSpeed);

		//ImGui::End();
	}

private:
	Scene* m_Scene;

	uint64_t m_Counter;
	uint64_t m_FixedUpdateCounter;

	TransformComponent* m_PlayerTC;
	int m_EnemyMoveSpeed;
};

class PhysicsDomain : Domain
{
public:
	void Init(DomainConfig& domainConfig) override
	{
	}

	void Shutdown() override
	{
	}

	void OnUpdate(Omni::Timestep ts) override
	{
	}

	void OnFixedUpdate() override
	{
	}

	void OnImGuiRender() override
	{
	}

	void OnSceneInit(Scene* scene) override {};
	void OnSceneShutdown() override {};
	void OnUpdateStart(Omni::Timestep ts) override {};
	void OnUpdateBeforeRender(Omni::Timestep ts) override {};
	void OnUpdateAfterRender(Omni::Timestep ts) override {};
};

class EnemyScript : Omni::ScriptableEntity
{
public:
	void OnCreate()
	{
		m_TC = &m_Entity.GetComponent<TransformComponent>();
		Scene* scene = m_Entity.GetScene();
		m_Player = scene->GetEntity("Player");
		m_PlayerTC = &m_Player.GetComponent<TransformComponent>();
	}

	void OnDestroy()
	{
	}

	void OnUpdate(Timestep ts)
	{
		if (m_TC->Translation.x < m_PlayerTC->Translation.x)
		{
			m_TC->Translation.x += m_MoveSpeed * ts;
		}
		else if (m_TC->Translation.x > m_PlayerTC->Translation.x)
		{
			m_TC->Translation.x -= m_MoveSpeed * ts;
		}

		if (m_TC->Translation.y < m_PlayerTC->Translation.y)
		{
			m_TC->Translation.y += m_MoveSpeed * ts;
		}
		else if (m_TC->Translation.y > m_PlayerTC->Translation.y)
		{
			m_TC->Translation.y -= m_MoveSpeed * ts;
		}
	}

	void OnFixedUpdate()
	{
	}

	OM_PARAM;
	int m_MoveSpeed;

	TransformComponent* m_TC;

	Entity m_Player;
	TransformComponent* m_PlayerTC;
};

class TestScript : Omni::ScriptableEntity
{
public:
	void OnCreate()
	{
		std::cout << "Hello from TestScript!" << std::endl;
	}

	void OnDestroy()
	{
	}

	void OnUpdate(Timestep ts)
	{
		TransformComponent* tc = &m_Entity.GetComponent<TransformComponent>();

		if (Input::IsKeyPressed(OM_KEY_J))
		{
			std::cout << m_Entity.GetName() << std::endl;
		}

		if (Input::IsKeyPressed(OM_KEY_UP))
		{
			tc->Translation.y += (2 * ts);
		}

		if (Input::IsKeyPressed(OM_KEY_DOWN))
		{
			tc->Translation.y -= (2 * ts);
		}

		if (Input::IsKeyPressed(OM_KEY_RIGHT))
		{
			tc->Translation.x += (2 * ts);
		}

		if (Input::IsKeyPressed(OM_KEY_LEFT))
		{
			tc->Translation.x -= (2 * ts);
		}

		//tc->Rotation.x += m_TestVar * ts;
		//tc->Rotation.y += m_TestVar * ts;
		//tc->Rotation.z += m_TestVar * ts;

		//if (tc->Rotation.x >= 180.0f)
		//{
		//	tc->Rotation.x = 0.0f;
		//}
		//if (tc->Rotation.y >= 180.0f)
		//{
		//	tc->Rotation.y = 0.0f;
		//}
		//if (tc->Rotation.z >= 180.0f)
		//{
		//	tc->Rotation.z = 0.0f;
		//}

		tc->Translation.x += (float)m_TestVar * ts;

		//printf("m_TestVar: %i\n", m_TestVar);
	}

	void OnFixedUpdate()
	{
	}

	OM_PARAM
	int m_TestVar = 2;

	OM_PARAM
	int m_SuperVar;

	OM_PARAM;
	int m_TestVar3;

	OM_BEGIN_PARAM_GROUP;
	uint32_t m_Testuint32 = 200;
	uint64_t m_Testuint64 = 1986;
	uint32_t m_Super32; uint64_t m_Super64;
	float m_TestFloat = 0.0f;
	double m_TestDouble = 2.0f;
	bool m_TestBool = false;
	std::string m_TestString = "B)";
	std::string m_SuperString;
	OM_END_PARAM_GROUP;
};

class TestScript2 : Omni::ScriptableEntity
{
public:
	void OnCreate()
	{
		std::cout << "Hello from TestScript2!" << std::endl;
		std::srand(std::time(0));
	}

	void OnDestroy()
	{
	}

	void OnUpdate(Timestep ts)
	{
		TransformComponent* tc = &m_Entity.GetComponent<TransformComponent>();

		if (Input::IsKeyPressed(OM_KEY_J))
		{
			std::cout << m_Entity.GetName() << std::endl;
		}

		if (Input::IsKeyPressed(OM_KEY_UP))
		{
			tc->Translation.y += (2 * ts);
		}

		if (Input::IsKeyPressed(OM_KEY_DOWN))
		{
			tc->Translation.y -= (2 * ts);
		}

		if (Input::IsKeyPressed(OM_KEY_RIGHT))
		{
			tc->Translation.x += (2 * ts);
		}

		if (Input::IsKeyPressed(OM_KEY_LEFT))
		{
			tc->Translation.x -= (2 * ts);
		}
	}

	float randomFloat()
	{
		return (float)(rand()) / (float)(RAND_MAX);
	}

	int randomInt(int a, int b)
	{
		if (a > b)
			return randomInt(b, a);
		if (a == b)
			return a;
		return a + (rand() % (b - a));
	}

	float randomFloat(int a, int b)
	{
		if (a > b)
			return randomFloat(b, a);
		if (a == b)
			return a;

		return (float)randomInt(a, b) + randomFloat();
	}

	void OnFixedUpdate()
	{
		SpriteRendererComponent* sprite = &m_Entity.GetComponent<SpriteRendererComponent>();

		if (m_FixedUpdateCounter % 15 == 0)
		{
			sprite->Color.x = randomFloat(0.50f, 1.0f);
			sprite->Color.y = randomFloat(0.50f, 1.0f);
			sprite->Color.z = randomFloat(0.50f, 1.0f);
		}

		m_FixedUpdateCounter++;
	}

public:

	OM_PARAM
	uint64_t m_FixedUpdateCounter = 0;
};

class MagicScript : Omni::ScriptableEntity
{
public:
	void OnCreate()
	{
	}

	void OnDestroy()
	{
	}

	void OnUpdate(Timestep ts)
	{

	}

	void OnFixedUpdate()
	{
		TransformComponent* tc = &m_Entity.GetComponent<TransformComponent>();

		tc->Translation.x = 0 + std::cos(angle) * 2;
		tc->Translation.y = 0 + std::sin(angle) * 2;
		angle += 0.06f;

		m_FixedUpdateCounter++;
	}

	double angle = 0.0f;
	uint64_t m_FixedUpdateCounter = 0;
};

class MagicScript2 : Omni::ScriptableEntity
{
public:
	void OnCreate()
	{
	}

	void OnDestroy()
	{
	}

	void OnUpdate(Timestep ts)
	{

	}

	void OnFixedUpdate()
	{
		TransformComponent* tc = &m_Entity.GetComponent<TransformComponent>();

		tc->Translation.x = 0 + std::cos(angle) * 2;
		tc->Translation.y = 0 + std::sin(angle) * 2;
		angle += 0.06f;

		m_FixedUpdateCounter++;
	}

	double angle = 1.5708f;
	uint64_t m_FixedUpdateCounter = 0;
};

class MagicScript3 : Omni::ScriptableEntity
{
public:
	void OnCreate()
	{
	}

	void OnDestroy()
	{
	}

	void OnUpdate(Timestep ts)
	{

	}

	void OnFixedUpdate()
	{
		TransformComponent* tc = &m_Entity.GetComponent<TransformComponent>();

		tc->Translation.x = 0 + std::cos(angle) * 2;
		tc->Translation.y = 0 + std::sin(angle) * 2;
		angle += 0.06f;

		m_FixedUpdateCounter++;
	}

	double angle = 3.14159f;
	uint64_t m_FixedUpdateCounter = 0;
};

class MagicScript4 : Omni::ScriptableEntity
{
public:
	void OnCreate()
	{
	}

	void OnDestroy()
	{
	}

	void OnUpdate(Timestep ts)
	{

	}

	void OnFixedUpdate()
	{
		TransformComponent* tc = &m_Entity.GetComponent<TransformComponent>();

		tc->Translation.x = 0 + std::cos(angle) * 2;
		tc->Translation.y = 0 + std::sin(angle) * 2;
		angle += 0.06f;

		m_FixedUpdateCounter++;
	}

	double angle = 4.71239f;
	uint64_t m_FixedUpdateCounter = 0;
};
