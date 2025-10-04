#include "RayMarchDomain.h"

using namespace Omni;

void RayMarchDomain::Init(DomainConfig& domainConfig)
{
	domainConfig.RunInEditMode = true;
}

void RayMarchDomain::OnSceneInit(Scene* scene)
{
	m_Scene = scene;
	DomainMap::SetActiveRendererDomain(DomainMap::GetRendererDomains()->begin()->first);
	m_Renderer = static_cast<CustomRendererDomain*>(DomainMap::GetActiveRendererDomain());
}

void RayMarchDomain::Shutdown()
{
}

void RayMarchDomain::OnSceneShutdown()
{
}

void RayMarchDomain::OnUpdateStart(Timestep ts)
{
}

bool RayMarchDomain::Cull(CameraComponent* camera, glm::mat4 Transform, glm::vec3 Min, glm::vec3 Max)
{
	glm::mat4 vp = camera->Camera.GetViewProjectionMatrix();
	glm::mat4 mvp = vp * Transform;

	/*glm::vec4 corners[8] =
	{
		{min.x, min.y, }
	}*/
	return false;
}

void RayMarchDomain::OnUpdate(Timestep ts)
{
	//return;

	//m_Visible.clear();
	CameraComponent* mainCamera = m_Scene->GetMainCamera();

	auto sphereView = m_Scene->m_Registry.view<TransformComponent, Sphere>();
	for (auto entity : sphereView)
	{
		auto [transform, sphere] = sphereView.get<TransformComponent, Sphere>(entity);
		Entity e = { entity, m_Scene };

		if (e.IsActive())
			m_Renderer->DrawSphere(transform.Translation, glm::vec4(sphere.Red, sphere.Green, sphere.Blue, 1.0f), sphere.Blending, (int)(uint32_t)e, 0.5f);
	}

	auto boxView = m_Scene->m_Registry.view<TransformComponent, Box>();
	for (auto entity : boxView)
	{
		auto [transform, cube] = boxView.get<TransformComponent, Box>(entity);
		Entity e = { entity, m_Scene };

		if (e.IsActive())
			m_Renderer->DrawBox(transform.Translation, transform.Rotation, transform.Scale, glm::vec4(cube.Red, cube.Green, cube.Blue, 1.0f), cube.Blending, (int)(uint32_t)e);
	}

	auto texturedBoxView = m_Scene->m_Registry.view<TransformComponent, TexturedBox>();
	for (auto entity : texturedBoxView)
	{
		auto [transform, cube] = texturedBoxView.get<TransformComponent, TexturedBox>(entity);
		Entity e = { entity, m_Scene };

		/bool cull = false;
		//if (e.IsActive() && mainCamera)
		//{
		//	cull = Cull(mainCamera, transform.ToMat4(), cube.AABB_Min, cube.AABB_Max);
		//}

		if (e.IsActive() && !cull)
			m_Renderer->DrawTexturedBox(transform.Translation, transform.Rotation, transform.Scale,
				glm::vec4(cube.Red, cube.Green, cube.Blue, 1.0f), cube.Blending, (int)(uint32_t)e, cube.TextureIndex, cube.TextureScale);
	}

	auto cylinderView = m_Scene->m_Registry.view<TransformComponent, Cylinder>();
	for (auto entity : cylinderView)
	{
		auto [transform, cylinder] = cylinderView.get<TransformComponent, Cylinder>(entity);
		Entity e = { entity, m_Scene };

		if (e.IsActive())
			m_Renderer->DrawCylinder(transform.Translation, transform.Rotation, transform.Scale,
				glm::vec4(cylinder.Red, cylinder.Green, cylinder.Blue, 1.0f), cylinder.Blending, (int)(uint32_t)e, cylinder.Height);
	}
}

void RayMarchDomain::OnUpdateBeforeRender(Timestep ts)
{
}

void RayMarchDomain::OnUpdateAfterRender(Timestep ts)
{
}

void RayMarchDomain::OnFixedUpdate()
{
}

void RayMarchDomain::OnImGuiRender()
{
}
