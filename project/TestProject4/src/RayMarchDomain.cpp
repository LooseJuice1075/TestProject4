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

bool RayMarchDomain::Cull(Camera* camera, BoundingBox* bb)
{
	//OM_TRACE("----------------------------");
	for (int i = 0; i < 8; i++)
	{
		//OM_TRACE("{0}, {1}, {2}", bb->Points[i].x, bb->Points[i].y, bb->Points[i].z);
		if (camera->IsPointVisible(bb->Points[i]))
		{
			return true;
		}
	}
	//OM_TRACE("----------------------------");
	return false;
	//return true;
}

void RayMarchDomain::OnUpdate(Timestep ts)
{
	//return;

	//CameraComponent* mainCamera = m_Scene->GetMainCamera();

	auto sphereView = m_Scene->m_Registry.view<TransformComponent, Sphere>();
	for (auto entity : sphereView)
	{
		auto [transform, sphere] = sphereView.get<TransformComponent, Sphere>(entity);
		Entity e = { entity, m_Scene };

		bool visible = true;
		if (e.HasComponent<BoundingBox>() && m_Renderer->MainCamera)
		{
			BoundingBox* bb = &e.GetComponent<BoundingBox>();
			if (bb->Active)
			{
				visible = Cull(m_Renderer->MainCamera, bb);
			}
		}

		if (e.IsActive() && visible)
			m_Renderer->DrawSphere(transform.Translation, glm::vec4(sphere.Red, sphere.Green, sphere.Blue, 1.0f), sphere.Blending, (int)(uint32_t)e, 0.5f);
	}

	auto boxView = m_Scene->m_Registry.view<TransformComponent, Box>();
	for (auto entity : boxView)
	{
		auto [transform, cube] = boxView.get<TransformComponent, Box>(entity);
		Entity e = { entity, m_Scene };

		bool visible = true;
		if (e.HasComponent<BoundingBox>() && m_Renderer->MainCamera)
		{
			BoundingBox* bb = &e.GetComponent<BoundingBox>();
			if (bb->Active)
			{
				visible = Cull(m_Renderer->MainCamera, bb);
			}
		}

		if (e.IsActive() && visible)
			m_Renderer->DrawBox(transform.Translation, transform.Rotation, transform.Scale, glm::vec4(cube.Red, cube.Green, cube.Blue, 1.0f), cube.Blending, (int)(uint32_t)e);
	}

	auto texturedBoxView = m_Scene->m_Registry.view<TransformComponent, TexturedBox>();
	for (auto entity : texturedBoxView)
	{
		auto [transform, cube] = texturedBoxView.get<TransformComponent, TexturedBox>(entity);
		Entity e = { entity, m_Scene };

		bool visible = true;
		if (e.HasComponent<BoundingBox>() && m_Renderer->MainCamera)
		{
			BoundingBox* bb = &e.GetComponent<BoundingBox>();
			if (bb->Active)
			{
				visible = Cull(m_Renderer->MainCamera, bb);
			}
		}

		if (e.IsActive() && visible)
			m_Renderer->DrawTexturedBox(transform.Translation, transform.Rotation, transform.Scale,
				glm::vec4(cube.Red, cube.Green, cube.Blue, 1.0f), cube.Blending, (int)(uint32_t)e, cube.TextureIndex, cube.TextureScale);
	}

	auto cylinderView = m_Scene->m_Registry.view<TransformComponent, Cylinder>();
	for (auto entity : cylinderView)
	{
		auto [transform, cylinder] = cylinderView.get<TransformComponent, Cylinder>(entity);
		Entity e = { entity, m_Scene };

		bool visible = true;
		if (e.HasComponent<BoundingBox>() && m_Renderer->MainCamera)
		{
			BoundingBox* bb = &e.GetComponent<BoundingBox>();
			if (bb->Active)
			{
				visible = Cull(m_Renderer->MainCamera, bb);
			}
		}

		if (e.IsActive() && visible)
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
	ImGui::Begin("RayMarchDomain");

	if (ImGui::Button("BB Visual"))
	{
		Entity marker = m_Scene->GetEntity("Marker");
		if (marker.HasComponent<BoundingBox>())
		{
			auto& bb = marker.GetComponent<BoundingBox>();
			for (int i = 0; i < 8; i++)
			{
				Entity point = m_Scene->CreateEntity("Point " + std::to_string(i));
				auto& tc = point.GetComponent<TransformComponent>();
				tc.Translation = bb.Points[i];
				tc.Scale = { 0.1f, 0.1f, 0.1f };
				auto& box = point.AddComponent<TexturedBox>();
			}
		}
	}

	ImGui::End();
}
