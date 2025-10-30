#pragma once

#include <iostream>
#include "Omni.h"
#include "domain/Domain.h"
#include "scene/ScriptableEntity.h"
#include "core/Application.h"
using namespace Omni;

OM_COMPONENT;
struct EnemyComponent
{
	OM_BEGIN_PARAM_GROUP;
	int State;
	OM_END_PARAM_GROUP;
};

OM_COMPONENT;
struct Sphere
{
	OM_BEGIN_PARAM_GROUP;
	float Blending = 0.0f;
	float Radius;

	float Red = 1.0f;
	float Green = 1.0f;
	float Blue = 1.0f;
	OM_END_PARAM_GROUP;
};

OM_COMPONENT;
struct Box
{
	OM_BEGIN_PARAM_GROUP;
	float Blending = 0.0f;

	float Red = 1.0f;
	float Green = 1.0f;
	float Blue = 1.0f;
	OM_END_PARAM_GROUP;
};

OM_COMPONENT;
struct TexturedBox
{
	OM_BEGIN_PARAM_GROUP;
	float Blending = 0.0f;

	float Red = 1.0f;
	float Green = 1.0f;
	float Blue = 1.0f;

	float TextureIndex = 0.0f;
	float TextureScale = 1.0f;
	OM_END_PARAM_GROUP;
};

OM_COMPONENT;
struct Cylinder
{
	OM_BEGIN_PARAM_GROUP;
	float Blending = 0.0f;

	float Height;

	float Red = 1.0f;
	float Green = 1.0f;
	float Blue = 1.0f;
	OM_END_PARAM_GROUP;
};
