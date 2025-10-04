#include "RendererDomain.h"

#include "renderer/RendererAPI.h"
#include "platform/OpenGL/OpenGLRendererAPI.h"
#include "project/Project.h"
#include "core/Application.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct SpriteVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoord;
	float TexIndex;
	float TilingFactor;

	// Editor Only
	int EntityID;
};

// TEMP
struct TestVertex
{
	glm::vec3 Position;
};
// TEMP

struct RendererData
{
	static const uint32_t MaxSprites = 20000;
	static const uint32_t MaxVertices = MaxSprites * 4;
	static const uint32_t MaxIndices = MaxSprites * 6;
	static const uint32_t MaxTextureSlots = 32;

	Ref<VertexArray> SpriteVertexArray;
	Ref<VertexBuffer> SpriteVertexBuffer;
	Ref<Shader> SpriteShader;
	Ref<Texture2D> WhiteTexture;

	uint32_t SpriteIndexCount = 0;
	SpriteVertex* SpriteVertexBufferBase = nullptr;
	SpriteVertex* SpriteVertexBufferPtr = nullptr;

	std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
	uint32_t TextureSlotIndex = 1; // 0 = white texture

	glm::vec4 SpriteVertexPositions[4];

	RendererStatistics Stats;

	struct CameraData
	{
		glm::mat4 ViewProjection;
	};
	CameraData CameraBuffer;
	Ref<UniformBuffer> CameraUniformBuffer;
};

static RendererData s_Data;

void CustomRendererDomain::Init()
{
	printf("Hello, custom renderer!\n");

	RenderCommand::Init();

	// Set current working directory to project asset directory
	std::filesystem::current_path(Project::GetActive()->GetAssetDirectory());

	s_Data.SpriteVertexArray = VertexArray::Create();

	s_Data.SpriteVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(SpriteVertex));
	s_Data.SpriteVertexBuffer->SetLayout({
		{ ShaderDataType::Float3, "a_Position"     },
		{ ShaderDataType::Float4, "a_Color"        },
		{ ShaderDataType::Float2, "a_TexCoord"     },
		{ ShaderDataType::Float,  "a_TexIndex"     },
		{ ShaderDataType::Float,  "a_TilingFactor" },
		{ ShaderDataType::Int,    "a_EntityID"     },
		});
	s_Data.SpriteVertexArray->AddVertexBuffer(s_Data.SpriteVertexBuffer);

	s_Data.SpriteVertexBufferBase = new SpriteVertex[s_Data.MaxVertices];

	uint32_t* spriteIndices = new uint32_t[s_Data.MaxIndices];

	uint32_t offset = 0;
	for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
	{
		spriteIndices[i + 0] = offset + 0;
		spriteIndices[i + 1] = offset + 1;
		spriteIndices[i + 2] = offset + 2;

		spriteIndices[i + 3] = offset + 2;
		spriteIndices[i + 4] = offset + 3;
		spriteIndices[i + 5] = offset + 0;

		offset += 4;
	}

	Ref<IndexBuffer> spriteIB = IndexBuffer::Create(spriteIndices, s_Data.MaxIndices);
	s_Data.SpriteVertexArray->SetIndexBuffer(spriteIB);
	delete[] spriteIndices;

	s_Data.WhiteTexture = Texture2D::Create(TextureSpecification());
	uint32_t whiteTextureData = 0xffffffff;
	s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

	int32_t samplers[s_Data.MaxTextureSlots];
	for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
		samplers[i] = i;

	s_Data.SpriteShader = Shader::Create("shaders/Omni_Sprite.glsl");

	// Set first texture slot to 0
	s_Data.TextureSlots[0] = s_Data.WhiteTexture;

	s_Data.SpriteVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
	s_Data.SpriteVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
	s_Data.SpriteVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
	s_Data.SpriteVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

	s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData), 0);

	TestShader = Shader::Create("shaders/RayMarch.glsl");
	
	TestVA = VertexArray::Create();
	float testVertices[] =
	{
		-2.0f, -2.0f, 0.0f,
		2.0f, -2.0f, 0.0f,
		2.0f,  2.0f, 0.0f,
		-2.0f, 2.0f, 0.0f,

	};
	TestVB = VertexBuffer::Create(testVertices, 4 * sizeof(TestVertex));
	TestVB->SetLayout({
		{ ShaderDataType::Float3, "a_Position" },
		});
	TestVA->AddVertexBuffer(TestVB);
	//TestVB->SetData(testVertices, sizeof(SpriteVertex) * 3);

	uint32_t testIndices[] = { 0, 1, 2, 2, 3, 0 };
	TestIB = IndexBuffer::Create(testIndices, 6);
	TestVA->SetIndexBuffer(TestIB);
	
	Resolution = { 1280, 720 };

	Objects = new float[MaxObjects * ObjectSize];
	memset(Objects, 0, MaxObjects * (ObjectSize * sizeof(float)));

	ObjectBufferTexData = new Texture2DData();
	ObjectBufferTexData->Memory = static_cast<void*>(Objects);
	ObjectBufferTexData->Width = ObjectSize;
	ObjectBufferTexData->Height = MaxObjects;
	ObjectBufferTexData->Format = ImageFormat::R32F;
	ObjectBufferTexData->Channels = 1;

	ObjectBufferTexData->MinFilter = TextureFilteringMode::Nearest;
	ObjectBufferTexData->MagFilter = TextureFilteringMode::Nearest;
	ObjectBufferTexData->WrapMode = TextureWrapMode::Clamp;
	ObjectBufferTexData->GenerateMips = false;

	ObjectBufferTex = Texture2D::Generate(ObjectBufferTexData);

	CameraAnimationData = AnimationData("data.json");

	StartTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
}

void CustomRendererDomain::Shutdown()
{
	delete[] s_Data.SpriteVertexBufferBase;

	printf("Custom renderer shutdown...\n");
}

void CustomRendererDomain::OnWindowResize(uint32_t width, uint32_t height)
{
	RenderCommand::SetViewport(0, 0, width, height);
}

uint32_t cameraAnimScene = 0;
long long elapsedSceneTime = 0;
long long animSceneDuration = 6283;
long long timeSinceLastAnimScene = 0;

void CustomRendererDomain::BeginScene(Camera& camera)
{
	s_Data.CameraBuffer.ViewProjection = camera.GetViewProjectionMatrix();
	s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));

	Resolution = { camera.GetResolutionX(),  camera.GetResolutionY() };

	glm::vec3 cameraPos = camera.GetPosition();
	RayOrigin = { cameraPos.x, cameraPos.y, cameraPos.z, 0.0f };
	//RayOrigin.y = -cameraPos.y;
	RayDirection = { camera.GetPitch(), camera.GetYaw(), camera.GetRoll() };
	//RayDirection.x = camera.GetPitch();
	//RayDirection.z = camera.GetRoll();

	//RayOrigin = { CameraAnimationData.TranslationX[cameraAnimFrame], CameraAnimationData.TranslationZ[cameraAnimFrame] + 2.0f, CameraAnimationData.TranslationY[cameraAnimFrame], 0.0f };
	//RayDirection = { CameraAnimationData.RotationX[cameraAnimFrame], CameraAnimationData.RotationZ[cameraAnimFrame], CameraAnimationData.RotationY[cameraAnimFrame] };
	
	long long currentTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();

	switch (cameraAnimScene)
	{
	case 0:
		//RayOrigin.x = glm::sin((float)(currentTime - StartTime) * 0.001f) * 5.0f;
		//RayOrigin.z = glm::cos((float)(currentTime - StartTime) * 0.001f) * 5.0f;

		float test = std::atan2(0.0f, 0.0f) - std::atan2(RayOrigin.z, RayOrigin.x);
		//OM_TRACE("{0}", test);
		//RayDirection.y = (glm::sin((float)(currentTime - StartTime) * 0.0001f) * 114.0f);
		break;
	}

	elapsedSceneTime = currentTime - timeSinceLastAnimScene;

	if (elapsedSceneTime >= animSceneDuration)
	{
		/*switch (cameraAnimScene)
		{
		case 0:
			animSceneDuration = 1000;
			break;
		}*/

		//OM_TRACE("Scene Complete! X: {0} Z: {0}", RayOrigin.x, RayOrigin.z);

		cameraAnimScene++;
		if (cameraAnimScene >= 1)
		{
			cameraAnimScene = 0;
		}

		timeSinceLastAnimScene = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
	}

	StartBatch();
}

void CustomRendererDomain::EndScene()
{
	Flush();
}

float framecounter = 0.0f;
uint64_t Frame = 0;

void CustomRendererDomain::Flush()
{
	if (s_Data.SpriteIndexCount && false)
	{
		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.SpriteVertexBufferPtr - (uint8_t*)s_Data.SpriteVertexBufferBase);
		s_Data.SpriteVertexBuffer->SetData(s_Data.SpriteVertexBufferBase, dataSize);

		// Bind textures
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		s_Data.SpriteShader->Bind();
		RenderCommand::DrawIndexed(s_Data.SpriteVertexArray, s_Data.SpriteIndexCount);
		s_Data.Stats.DrawCalls++;
	}

	ObjectBufferTexData->Memory = static_cast<void*>(Objects);
	ObjectBufferTex = Texture2D::Generate(ObjectBufferTexData);
	ObjectBufferTex->Bind();

	TestShader->Bind();
	TestShader->SetFloat2("u_resolution", Resolution);
	long long time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
	TestShader->SetInt("u_time", (int)(time - StartTime));
	TestShader->SetFloatArray("u_objects", Objects, ObjectIndex);
	TestShader->SetInt("u_object_elements_count", ObjectIndex);
	TestShader->SetFloat4("u_ray_origin", RayOrigin);
	TestShader->SetFloat3("u_ray_direction", RayDirection);
	TestShader->SetFloat("u_fov", FOV);
	TestShader->SetFloat("u_light_angle", LightAngle);
	TestShader->SetFloat4("u_floor_color", FloorColor);

	framecounter += 0.01f;
	RenderCommand::DrawIndexed(TestVA, 6);

	// Reset for next draw call
	ObjectIndex = 0;
}

void CustomRendererDomain::SetClearColor(const glm::vec4& clearColor)
{
	RenderCommand::SetClearColor(clearColor);
	s_ClearColor = clearColor;
}

uint64_t currentTime = 0;
float speed = 0.025f;

void CustomRendererDomain::OnFixedUpdate()
{
	return;

	// 251

	RayOrigin.x = glm::sin((float)currentTime * speed) * 5.0f;
	RayOrigin.z = glm::cos((float)currentTime * speed) * 5.0f;

	uint64_t rotTime = currentTime % 252;
	//RayDirection.y = ((float)currentTime * 0.00626f) * 114.0f;
	RayDirection.y = -((float)rotTime * 0.4542f) + (114.0f / 2.0f);
	
	currentTime++;
}

void CustomRendererDomain::OnImGuiRender()
{
	ImGui::Begin("CustomRendererDomain");

	if (ImGui::Button("Reload Shaders"))
	{
		ReloadShaders();
	}

	ImGui::Separator();

	ImGui::DragFloat("Light", &LightAngle);

	ImGui::Separator();

	ImGui::DragFloat("FOV", &FOV, 0.01f, 0.0f, 999.0f);

	ImGui::Separator();

	ImGui::InputFloat("X", &RayOrigin.x);
	ImGui::InputFloat("Y", &RayOrigin.y);
	ImGui::InputFloat("Z", &RayOrigin.z);
	ImGui::InputFloat("W", &RayOrigin.w);

	ImGui::Separator();

	ImGui::InputFloat("Rot X", &RayDirection.x);
	ImGui::InputFloat("Rot Y", &RayDirection.y);
	ImGui::InputFloat("Rot Z", &RayDirection.z);

	ImGui::Separator();

	ImGui::ColorEdit4("Floor Color", glm::value_ptr(FloorColor));

	/*for (int i = 0; i < sizeof(Objects) / sizeof(float); i += 4)
	{
		ImGui::InputFloat(std::to_string(i).c_str(), &Objects[i]);
		ImGui::InputFloat(std::to_string(i + 1).c_str(), &Objects[i + 1]);
		ImGui::InputFloat(std::to_string(i + 2).c_str(), &Objects[i + 2]);
		ImGui::InputFloat(std::to_string(i + 3).c_str(), &Objects[i + 3]);

		ImGui::Separator();
	}*/

	ImGui::End();
}

void CustomRendererDomain::DrawSphere(
	const glm::vec3& position,
	const glm::vec4& color,
	float blending,
	int entity,
	float radius)
{
	Objects[ObjectIndex] = 0.0f;

	Objects[ObjectIndex + 1] = position.x;
	Objects[ObjectIndex + 2] = position.y;
	Objects[ObjectIndex + 3] = position.z;

	Objects[ObjectIndex + 4] = 0.0f;
	Objects[ObjectIndex + 5] = 0.0f;
	Objects[ObjectIndex + 6] = 0.0f;

	Objects[ObjectIndex + 7] = 0.0f;
	Objects[ObjectIndex + 8] = 0.0f;
	Objects[ObjectIndex + 9] = 0.0f;

	Objects[ObjectIndex + 10] = color.r;
	Objects[ObjectIndex + 11] = color.g;
	Objects[ObjectIndex + 12] = color.b;
	Objects[ObjectIndex + 13] = color.a;

	Objects[ObjectIndex + 14] = blending;
	Objects[ObjectIndex + 15] = entity;

	Objects[ObjectIndex + 16] = radius;

	Objects[ObjectIndex + 17] = 0.0f;

	ObjectIndex += ObjectSize;
}

void CustomRendererDomain::DrawBox(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	const glm::vec4& color,
	float blending,
	int entity)
{
	Objects[ObjectIndex] = 1.0f;

	Objects[ObjectIndex + 1] = position.x;
	Objects[ObjectIndex + 2] = position.y;
	Objects[ObjectIndex + 3] = position.z;

	Objects[ObjectIndex + 4] = rotation.x;
	Objects[ObjectIndex + 5] = rotation.y;
	Objects[ObjectIndex + 6] = rotation.z;

	Objects[ObjectIndex + 7] = scale.x;
	Objects[ObjectIndex + 8] = scale.y;
	Objects[ObjectIndex + 9] = scale.z;

	Objects[ObjectIndex + 10] = color.r;
	Objects[ObjectIndex + 11] = color.g;
	Objects[ObjectIndex + 12] = color.b;
	Objects[ObjectIndex + 13] = color.a;

	Objects[ObjectIndex + 14] = blending;
	Objects[ObjectIndex + 15] = entity;

	Objects[ObjectIndex + 16] = 0.0f;
	Objects[ObjectIndex + 17] = 0.0f;

	ObjectIndex += ObjectSize;
}

void CustomRendererDomain::DrawTexturedBox(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	const glm::vec4& color,
	float blending,
	int entity,
	float textureIndex,
	float textureScale)
{
	Objects[ObjectIndex] = 2.0f;

	Objects[ObjectIndex + 1] = position.x;
	Objects[ObjectIndex + 2] = position.y;
	Objects[ObjectIndex + 3] = position.z;

	Objects[ObjectIndex + 4] = rotation.x;
	Objects[ObjectIndex + 5] = rotation.y;
	Objects[ObjectIndex + 6] = rotation.z;

	Objects[ObjectIndex + 7] = scale.x;
	Objects[ObjectIndex + 8] = scale.y;
	Objects[ObjectIndex + 9] = scale.z;

	Objects[ObjectIndex + 10] = color.r;
	Objects[ObjectIndex + 11] = color.g;
	Objects[ObjectIndex + 12] = color.b;
	Objects[ObjectIndex + 13] = color.a;

	Objects[ObjectIndex + 14] = blending;
	Objects[ObjectIndex + 15] = entity;

	Objects[ObjectIndex + 16] = textureIndex;
	Objects[ObjectIndex + 17] = textureScale;

	ObjectIndex += ObjectSize;
}

void CustomRendererDomain::DrawCylinder(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	const glm::vec4& color,
	float blending,
	int entity,
	float rounding)
{
	Objects[ObjectIndex] = 3.0f;

	Objects[ObjectIndex + 1] = position.x;
	Objects[ObjectIndex + 2] = position.y;
	Objects[ObjectIndex + 3] = position.z;

	Objects[ObjectIndex + 4] = rotation.x;
	Objects[ObjectIndex + 5] = rotation.y;
	Objects[ObjectIndex + 6] = rotation.z;

	Objects[ObjectIndex + 7] = scale.x;
	Objects[ObjectIndex + 8] = scale.y;
	Objects[ObjectIndex + 9] = scale.z;

	Objects[ObjectIndex + 10] = color.r;
	Objects[ObjectIndex + 11] = color.g;
	Objects[ObjectIndex + 12] = color.b;
	Objects[ObjectIndex + 13] = color.a;

	Objects[ObjectIndex + 14] = blending;
	Objects[ObjectIndex + 15] = entity;

	Objects[ObjectIndex + 16] = rounding;

	Objects[ObjectIndex + 17] = 0.0f;

	ObjectIndex += ObjectSize;
}

void CustomRendererDomain::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
{
	shader->Bind();

	vertexArray->Bind();
	RenderCommand::DrawIndexed(vertexArray);
}

void CustomRendererDomain::DrawSprite(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	const glm::vec4& color,
	int entityID)
{
	glm::mat4 calculatedRotation = glm::toMat4(glm::quat(-glm::radians(rotation)));
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
		* calculatedRotation
		* glm::scale(glm::mat4(1.0f), scale);

	DrawSprite(transform, color, entityID);
}

void CustomRendererDomain::DrawSprite(
	const glm::vec3& position,
	const glm::vec3& rotation,
	const glm::vec3& scale,
	const Ref<Texture2D>& texture,
	const glm::vec4& color,
	float tilingFactor,
	const glm::vec2& spriteSize,
	const glm::vec2& spritePosition,
	int entityID)
{
	glm::mat4 calculatedRotation = glm::toMat4(glm::quat(-glm::radians(rotation)));
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
		* calculatedRotation
		* glm::scale(glm::mat4(1.0f), scale);

	DrawSprite(transform, texture, color, tilingFactor, spriteSize, spritePosition, entityID);
}

void CustomRendererDomain::DrawSprite(const glm::mat4& transform, const glm::vec4& color, int entityID)
{
	constexpr size_t spriteVertexCount = 4;
	const float textureIndex = 0.0f; // This is the White Texture ID
	constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
	const float tilingFactor = 1.0f;

	if (s_Data.SpriteIndexCount >= RendererData::MaxIndices)
		NextBatch();

	for (size_t i = 0; i < spriteVertexCount; i++)
	{
		s_Data.SpriteVertexBufferPtr->Position = transform * s_Data.SpriteVertexPositions[i];
		s_Data.SpriteVertexBufferPtr->Color = color;
		s_Data.SpriteVertexBufferPtr->TexCoord = textureCoords[i];
		s_Data.SpriteVertexBufferPtr->TexIndex = textureIndex;
		s_Data.SpriteVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.SpriteVertexBufferPtr->EntityID = entityID;
		s_Data.SpriteVertexBufferPtr++;
	}

	s_Data.SpriteIndexCount += 6;
	s_Data.Stats.SpriteCount++;
}

void CustomRendererDomain::DrawSprite(
	const glm::mat4& transform,
	const Ref<Texture2D>& texture,
	const glm::vec4& color,
	float tilingFactor,
	const glm::vec2& spriteSize,
	const glm::vec2& spritePosition,
	int entityID)
{
	float sheetWidth = texture->GetWidth();
	float sheetHeight = texture->GetHeight();

	// Adding this offset to the uv coords fixes the apparent texture bleeding
	float offset = 0.0000001f;

	constexpr size_t spriteVertexCount = 4;
	glm::vec2 textureCoords[] = {
		{ ((spritePosition.x * spriteSize.x) / sheetWidth) + offset, ((spritePosition.y * spriteSize.y) / sheetHeight) + offset },
		{ (((spritePosition.x + 1) * spriteSize.x) / sheetWidth) + offset, ((spritePosition.y * spriteSize.y) / sheetHeight) + offset },
		{ (((spritePosition.x + 1) * spriteSize.x) / sheetWidth) + offset, (((spritePosition.y + 1) * spriteSize.y) / sheetHeight) + offset },
		{ ((spritePosition.x * spriteSize.x) / sheetWidth) + offset, (((spritePosition.y + 1) * spriteSize.y) / sheetHeight) + offset }
	};

	if (s_Data.SpriteIndexCount >= RendererData::MaxIndices)
		NextBatch();

	float textureIndex = 0.0f;
	for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
	{
		if (*s_Data.TextureSlots[i] == *texture)
		{
			textureIndex = (float)i;
			break;
		}
	}

	if (textureIndex == 0.0f)
	{
		if (s_Data.TextureSlotIndex >= RendererData::MaxTextureSlots)
			NextBatch();

		textureIndex = (float)s_Data.TextureSlotIndex;
		s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
		s_Data.TextureSlotIndex++;
	}

	for (size_t i = 0; i < spriteVertexCount; i++)
	{
		s_Data.SpriteVertexBufferPtr->Position = transform * s_Data.SpriteVertexPositions[i];
		s_Data.SpriteVertexBufferPtr->Color = color;
		s_Data.SpriteVertexBufferPtr->TexCoord = textureCoords[i];
		s_Data.SpriteVertexBufferPtr->TexIndex = textureIndex;
		s_Data.SpriteVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.SpriteVertexBufferPtr->EntityID = entityID;
		s_Data.SpriteVertexBufferPtr++;
	}

	s_Data.SpriteIndexCount += 6;

	s_Data.Stats.SpriteCount++;
}

void CustomRendererDomain::ResetStats()
{
	memset(&s_Data.Stats, 0, sizeof(RendererStatistics));
}

RendererStatistics CustomRendererDomain::GetStats()
{
	return s_Data.Stats;
}

void CustomRendererDomain::ReloadShaders()
{
	TestShader = Shader::Create("shaders/RayMarch.glsl");
}

void CustomRendererDomain::StartBatch()
{
	s_Data.SpriteIndexCount = 0;
	s_Data.SpriteVertexBufferPtr = s_Data.SpriteVertexBufferBase;

	s_Data.TextureSlotIndex = 1;
}

void CustomRendererDomain::NextBatch()
{
	Flush();
	StartBatch();
}
