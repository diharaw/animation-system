#pragma once

#include "skeletal_mesh.h"
#include "anim_sample.h"
#include "anim_blend.h"
#include <vector>
#include <algorithm>

class Blendspace1D
{
public:
	struct Node
	{
		float						value;
		Animation*					anim;
		std::unique_ptr<AnimSample> sampler;

		Node(Skeleton* _skeleton, Animation* _anim, float _value)
		{
			anim = _anim;
			sampler = std::make_unique<AnimSample>(_skeleton, _anim);
			value = _value;
		}
	};

public:
	Blendspace1D(Skeleton* _skeleton, std::vector<Node*> _nodes);
	~Blendspace1D();
	void set_value(float value);
	float max();
	float min();
	float value();
	Pose* evaluate(float dt);

private:
	float m_value = 0.0f;
	float m_min = 0.0f;
	float m_max = 0.0f;
	std::vector<Node*> m_nodes;
	std::unique_ptr<AnimBlend> m_blend;
};