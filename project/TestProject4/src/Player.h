#pragma once

#include "Omni.h"

using namespace Omni;

class Player : ScriptableEntity
{
public:
	void OnCreate() override;

	void OnDestroy() override;

	void OnUpdate(Timestep ts) override;

	void OnFixedUpdate() override;

public:
	TransformComponent* m_TC;
	CameraComponent* m_Camera;

	OM_BEGIN_PARAM_GROUP;
	float m_MoveSpeed;
	float m_LookSpeed;
	OM_END_PARAM_GROUP;

private:
	glm::vec2 m_CursorPrevPos;
};
