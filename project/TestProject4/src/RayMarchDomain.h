#pragma once

#include <iostream>

#include "RendererDomain.h"

#include "Omni.h"
#include "domain/Domain.h"
#include "scene/ScriptableEntity.h"
#include "core/Application.h"
#include "UserComponents.h"
#include "domain/DomainMap.h"
using namespace Omni;

class RayMarchDomain : Domain
{
public:
	void Init(DomainConfig& domainConfig) override;
	void OnSceneInit(Scene* scene) override;

	void Shutdown() override;
	void OnSceneShutdown() override;

	void OnUpdateStart(Timestep ts) override;
	void OnUpdate(Timestep ts) override;
	void OnUpdateBeforeRender(Timestep ts) override;
	void OnUpdateAfterRender(Timestep ts) override;

	void OnFixedUpdate() override;

	void OnImGuiRender() override;

	bool Cull(CameraComponent* camera, glm::mat4 Transform, glm::vec3 Min, glm::vec3 Max);

private:
	Scene* m_Scene;
	CustomRendererDomain* m_Renderer;

	std::vector<Entity> m_Visible;
};
