#pragma once

#include "domain/Domain.h"
#include "core/Core.h"
#include "renderer/RenderCommand.h"

#include "renderer/Camera.h"
#include "renderer/Shader.h"
#include "renderer/VertexArray.h"
#include "renderer/UniformBuffer.h"

#include "domain/RendererDomains.h"

#include <filesystem>
#include <fstream>
#include <glad/glad.h>

#include <lib/json/json.hpp>
using json = nlohmann::json;

using namespace Omni;

class CustomRendererDomain : public Omni::RendererDomain
{
public:
	void Init() override;
	void Shutdown() override;

	void OnWindowResize(uint32_t width, uint32_t height) override;

	void BeginScene(Camera& camera) override;
	void EndScene() override;
	void Flush() override;

	void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f)) override;

	RendererAPI::API GetAPI() override { return RendererAPI::GetAPI(); }

	glm::vec4 GetClearColor() override { return s_ClearColor; }
	void SetClearColor(const glm::vec4& clearColor) override;

	void OnFixedUpdate() override;

	void OnImGuiRender() override;

	// RayMarch Rendering

	void DrawSphere(
		const glm::vec3& position,
		const glm::vec4& color,
		float blending,
		int entity,
		float radius);

	void DrawBox(
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale,
		const glm::vec4& color,
		float blending,
		int entity);

	void DrawTexturedBox(
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale,
		const glm::vec4& color,
		float blending,
		int entity,
		float textureIndex,
		float textureScale);

	void DrawCylinder(
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale,
		const glm::vec4& color,
		float blending,
		int entity,
		float rounding);

	// RayMarch Rendering

	// 2D Rendering

	void DrawSprite(
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale,
		const glm::vec4& color,
		int entityID = -1) override;
	void DrawSprite(
		const glm::vec3& position,
		const glm::vec3& rotation,
		const glm::vec3& scale,
		const Ref<Texture2D>& texture,
		const glm::vec4& color,
		float tilingFactor,
		const glm::vec2& spriteSize,
		const glm::vec2& spritePosition,
		int entityID = -1) override;

	void DrawSprite(
		const glm::mat4& transform,
		const glm::vec4& color,
		int entityID = -1) override;
	void DrawSprite(
		const glm::mat4& transform,
		const Ref<Texture2D>& texture,
		const glm::vec4& color,
		float tilingFactor,
		const glm::vec2& spriteSize,
		const glm::vec2& spritePosition,
		int entityID = -1) override;

	// 2D Rendering

	// Stats

	void ResetStats() override;
	RendererStatistics GetStats() override;

	// Stats

	void ReloadShaders();

private:
	void StartBatch() override;
	void NextBatch() override;

public:
	struct SceneData
	{
		glm::mat4 ViewProjectionMatrix;
	};

	static Scope<SceneData> s_SceneData;
	inline static glm::vec4 s_ClearColor;

	Camera* MainCamera = nullptr;

	long long StartTime;

	Ref<Shader> TestShader;

	Ref<VertexArray> TestVA;
	Ref<VertexBuffer> TestVB;
	Ref<IndexBuffer> TestIB;

	glm::vec2 Resolution;

	float* Objects;
	uint64_t ObjectIndex = 0;
	uint64_t MaxObjects = 30;
	uint32_t ObjectSize = 18;

	Ref<Texture2D> ObjectBufferTex;
	Texture2DData* ObjectBufferTexData;

	glm::vec4 RayOrigin = { 0.0f, 4.0f, -3.5f, 1.0f };
	glm::vec3 RayDirection = { 0.2f, 0.0f, 0.0f };
	float FOV = 0.8f;

	float LightAngle = 175.0f;

	glm::vec4 FloorColor = { 0.525f, 0.416f, 1.0f, 1.0f };

	struct AnimationData
	{
		std::vector<float> TranslationX;
		std::vector<float> TranslationY;
		std::vector<float> TranslationZ;

		std::vector<float> RotationX;
		std::vector<float> RotationY;
		std::vector<float> RotationZ;

		AnimationData()
		{
			TranslationX = std::vector<float>();
			TranslationY = std::vector<float>();
			TranslationZ = std::vector<float>();

			RotationX = std::vector<float>();
			RotationY = std::vector<float>();
			RotationZ = std::vector<float>();
		}

		AnimationData(const std::string& path)
		{
			std::ifstream animationFile(path.c_str());
			json animData = json::parse(animationFile);
			std::map<int, std::string> animChannels =
			{
				{0, "location channel 0"},
				{1, "location channel 1"},
				{2, "location channel 2"},

				{3, "rotation_euler channel 0"},
				{4, "rotation_euler channel 1"},
				{5, "rotation_euler channel 2"}
			};

			for (int i = 0; i < 6; i++)
			{
				if (animData[animChannels[i]].is_array())
				{
					json arr = animData[animChannels[i]];
					int j = 1;
					//std::cout << animChannels[i] << std::endl;
					for (const auto& element : arr)
					{
						std::string index = std::to_string(j) + ".0";
						switch (i)
						{
						case 0:
							TranslationX.push_back(element[index]);
							break;
						case 1:
							TranslationY.push_back(element[index]);
							break;
						case 2:
							TranslationZ.push_back(element[index]);
							break;

						case 3:
							RotationX.push_back(element[index]);
							break;
						case 4:
							RotationY.push_back(element[index]);
							break;
						case 5:
							RotationZ.push_back(element[index]);
							break;
						}
						//std::cout << element[index] << std::endl;
						j++;
					}
				}
			}
		}
	};

	AnimationData CameraAnimationData;
};
