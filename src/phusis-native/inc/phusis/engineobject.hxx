#ifndef PHUSIS_ENGINEOBJECT_HXX
#define PHUSIS_ENGINEOBJECT_HXX

#include "mesh.hxx"
#include "fw.hxx"
#include <utility>

struct EngineObjectData
{
	bool enabled = true;

	const glm::mat4 Rotation;
	const glm::vec4 Color;
	const Mesh Mesh;

	explicit EngineObjectData(glm::mat4 rot, glm::vec4 color, struct Mesh mesh)
			: Rotation(rot), Color(color), Mesh(std::move(mesh))
	{
	}
};

#endif //PHUSIS_ENGINEOBJECT_HXX
