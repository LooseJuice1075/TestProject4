#include "Player.h"

void Player::OnCreate()
{
	m_TC = &m_Entity.GetComponent<TransformComponent>();
	if (!m_Entity.HasComponent<CameraComponent>())
	{
		OM_ERROR("Player script could not find camera!");
		return;
	}

	m_Camera = &m_Entity.GetComponent<CameraComponent>();
	m_Camera->IsMainCamera = true;

	m_CursorPrevPos = glm::vec2(0.0f);
}

void Player::OnDestroy()
{
}

void Player::OnUpdate(Timestep ts)
{
	// TODO : Normalize movement vector

	if (Input::IsKeyPressed(OM_KEY_W))
	{
		m_TC->Translation.x += (m_MoveSpeed * glm::sin(glm::radians(m_TC->Rotation.y))) * (float)ts;
		m_TC->Translation.z += -(m_MoveSpeed * glm::cos(glm::radians(m_TC->Rotation.y))) * (float)ts;
	}

	if (Input::IsKeyPressed(OM_KEY_S))
	{
		m_TC->Translation.x -= (m_MoveSpeed * glm::sin(glm::radians(m_TC->Rotation.y))) * (float)ts;
		m_TC->Translation.z -= -(m_MoveSpeed * glm::cos(glm::radians(m_TC->Rotation.y))) * (float)ts;
	}

	if (Input::IsKeyPressed(OM_KEY_D))
	{
		m_TC->Translation.x += (m_MoveSpeed * glm::sin(glm::radians(m_TC->Rotation.y + 90.0f))) * (float)ts;
		m_TC->Translation.z += -(m_MoveSpeed * glm::cos(glm::radians(m_TC->Rotation.y + 90.0f))) * (float)ts;
	}

	if (Input::IsKeyPressed(OM_KEY_A))
	{
		m_TC->Translation.x -= (m_MoveSpeed * glm::sin(glm::radians(m_TC->Rotation.y + 90.0f))) * (float)ts;
		m_TC->Translation.z -= -(m_MoveSpeed * glm::cos(glm::radians(m_TC->Rotation.y + 90.0f))) * (float)ts;
	}

	if (Input::IsKeyPressed(OM_KEY_RIGHT))
	{
		m_TC->Rotation.y += m_LookSpeed * ts;
	}

	if (Input::IsKeyPressed(OM_KEY_LEFT))
	{
		m_TC->Rotation.y -= m_LookSpeed * ts;
	}

	if (Input::IsKeyPressed(OM_KEY_UP))
	{
		m_TC->Rotation.x -= m_LookSpeed * ts;
	}

	if (Input::IsKeyPressed(OM_KEY_DOWN))
	{
		m_TC->Rotation.x += m_LookSpeed * ts;
	}

	//glm::vec2 delta = { (Input::GetMouseX() - m_CursorPrevPos.x) * 0.003f, (Input::GetMouseY() - m_CursorPrevPos.y) * 0.003f };

	//float yawSign = m_Camera->Camera.GetUpDirection().y < 0 ? -1.0f : 1.0f;
	//m_TC->Rotation = glm::vec3(m_TC->Rotation.x + (delta.y * m_LookSpeed), m_TC->Rotation.x + (yawSign * delta.x * m_LookSpeed), m_TC->Rotation.z);

	//m_CursorPrevPos.x = Input::GetMouseX();
	//m_CursorPrevPos.y = Input::GetMouseY();
}

void Player::OnFixedUpdate()
{
}
