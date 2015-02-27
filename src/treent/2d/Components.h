
#pragma once

#include "treent/TreeComponent.h"

namespace treent
{

struct TransformComponent : public TreeComponent<TransformComponent>
{
	void compose (const TransformComponent &other) { _position += other._position; _rotation += other._rotation; }

	ci::vec2	_position;
	float			_rotation;
};

struct StyleComponent : public TreeComponent<StyleComponent>
{
	void compose(const StyleComponent &other) { _alpha *= other._alpha; }

	float 		_alpha;
	ci::vec3	_color;
};

} // namespace treent
