#include "blendspace_1d.h"

Blendspace1D::Blendspace1D(Skeleton* _skeleton, std::vector<Node*> _nodes) : m_nodes(_nodes)
{
	m_blend = std::make_unique<AnimBlend>(_skeleton);

	if (m_nodes.size() > 0)
	{
		m_min = m_nodes[0]->value;
		m_max = m_nodes[std::max(0.0f, float(m_nodes.size() - 1.0f))]->value;
		m_value = m_min;
	}
}

Blendspace1D::~Blendspace1D()
{
	for (auto& node : m_nodes)
	{
		if (node)
			delete node;
	}
}

void Blendspace1D::set_value(float value)
{
	value = std::max(m_min, value);
	value = std::min(m_max, value);
	m_value = value;
}

float Blendspace1D::max()
{
	return m_max;
}

float Blendspace1D::min()
{
	return m_min;
}

float Blendspace1D::value()
{
	return m_value;
}

Pose* Blendspace1D::evaluate(float dt)
{
	for (uint32_t i = 0; i < m_nodes.size(); i++)
	{
		if (m_value == m_nodes[i]->value)
			return m_nodes[i]->sampler->sample(dt);
		else if (m_value < m_nodes[i]->value)
		{
			Node* low = m_nodes[i - 1];
			Node* high = m_nodes[i];

			float blend_factor = (m_value - low->value) / (high->value - low->value);

			Pose* low_pose = low->sampler->sample(dt);
			Pose* high_pose = high->sampler->sample(dt);

			return m_blend->blend(low_pose, high_pose, blend_factor);
		}
	}

	return nullptr;
}