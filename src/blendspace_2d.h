#pragma once

#include "skeletal_mesh.h"
#include "anim_sample.h"
#include "anim_blend.h"

class Blendspace2D
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

	struct Row
	{
		float value;
		std::vector<Node*> nodes;
	};

public:
	Blendspace2D(Skeleton* skeleton, const std::vector<Row>& rows);
	~Blendspace2D();
	void set_x_value(float value);
	float max_x();
	float min_x();
	float value_x();
	void set_y_value(float value);
	float max_y();
	float min_y();
	float value_y();
	Pose* evaluate(float dt);

private:
	Pose* blended_pose_from_row(const Row& row, AnimBlend* blend, float dt);
	Pose* blended_pose_from_nodes(Node* low, Node* high, AnimBlend* blend, float dt);

private:
	float m_x_max;
	float m_x_min;
	float m_y_max;
	float m_y_min;
	float m_x_value;
	float m_y_value;
	std::vector<Row> m_rows;
	std::unique_ptr<AnimBlend> m_blend_1;
	std::unique_ptr<AnimBlend> m_blend_2;
	std::unique_ptr<AnimBlend> m_blend_3;
};