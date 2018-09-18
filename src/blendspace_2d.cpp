#include "blendspace_2d.h"
#include <algorithm>

Blendspace2D::Blendspace2D(Skeleton* skeleton, const std::vector<Row>& rows) : m_rows(rows)
{
	m_blend_1 = std::make_unique<AnimBlend>(skeleton);
	m_blend_2 = std::make_unique<AnimBlend>(skeleton);
	m_blend_3 = std::make_unique<AnimBlend>(skeleton);

	assert(m_rows.size() > 0);
	assert(m_rows[0].nodes.size() > 0);

	m_y_max = m_rows[0].value;
	m_y_min = m_rows[0].value;

	m_x_max = m_rows[0].nodes[0]->value;
	m_x_min = m_rows[0].nodes[0]->value;

	for (const auto& row : m_rows)
	{
		m_y_max = std::max(m_y_max, row.value);
		m_y_min = std::min(m_y_min, row.value);

		assert(row.nodes.size() > 0);

		for (const auto& node : row.nodes)
		{
			assert(node != nullptr);

			m_x_max = std::max(m_x_max, node->value);
			m_x_min = std::min(m_x_min, node->value);
		}
	}

	m_x_value = m_x_min;
	m_y_value = m_y_min;
}

Blendspace2D::~Blendspace2D()
{
	for (const auto& row : m_rows)
	{
		for (const auto& node : row.nodes)
		{
			if (node)
				delete node;
		}
	}
}

void Blendspace2D::set_x_value(float value)
{
	value = std::max(m_x_min, value);
	value = std::min(m_x_max, value);
	m_x_value = value;
}

float Blendspace2D::max_x()
{
	return m_x_max;
}

float Blendspace2D::min_x()
{
	return m_x_min;
}

float Blendspace2D::value_x()
{
	return m_x_value;
}

void Blendspace2D::set_y_value(float value)
{
	value = std::max(m_y_min, value);
	value = std::min(m_y_max, value);
	m_y_value = value;
}

float Blendspace2D::max_y()
{
	return m_y_max;
}

float Blendspace2D::min_y()
{
	return m_y_min;
}

float Blendspace2D::value_y()
{
	return m_y_value;
}

Pose* Blendspace2D::evaluate(float dt)
{
	for (uint32_t i = 0; i < m_rows.size(); i++)
	{
		if (m_y_value == m_rows[i].value)
			return blended_pose_from_row(m_rows[i], m_blend_1.get(), dt);
		else if (m_y_value < m_rows[i].value)
		{
			const Row& low = m_rows[i - 1];
			const Row& high = m_rows[i];

			float blend_factor = (m_y_value - low.value) / (high.value - low.value);

			Pose* low_pose = blended_pose_from_row(low, m_blend_1.get(), dt);
			Pose* high_pose = blended_pose_from_row(high, m_blend_2.get(), dt);

			return m_blend_3->blend(low_pose, high_pose, blend_factor);
		}
	}

	return nullptr;
}

Pose* Blendspace2D::blended_pose_from_row(const Row& row, AnimBlend* blend, float dt)
{
	if (row.nodes.size() == 1)
		return row.nodes[0]->sampler->sample(dt);

	for (uint32_t j = 0; j < row.nodes.size(); j++)
	{
		if (m_x_value == row.nodes[j]->value)
			return row.nodes[j]->sampler->sample(dt);
		else if (m_x_value < row.nodes[j]->value)
		{
			Node* low = row.nodes[j - 1];
			Node* high = row.nodes[j];

			return blended_pose_from_nodes(low, high, blend, dt);
		}
	}

	return nullptr;
}

Pose* Blendspace2D::blended_pose_from_nodes(Node* low, Node* high, AnimBlend* blend, float dt)
{
	float blend_factor = (m_x_value - low->value) / (high->value - low->value);

	Pose* low_pose = low->sampler->sample(dt);
	Pose* high_pose = high->sampler->sample(dt);

	return blend->blend(low_pose, high_pose, blend_factor);
}