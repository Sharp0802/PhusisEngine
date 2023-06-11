#ifndef PHUSIS_MESH_HXX
#define PHUSIS_MESH_HXX

#include "buffer.hxx"
#include "fw.hxx"
#include <utility>

namespace Phusis
{
	struct Mesh
	{
		const std::vector<Buffer> Vertices;
		const std::vector<Buffer> Indices;

		explicit Mesh(
				std::vector<Buffer> vertices,
				std::vector<Buffer> indices) noexcept
				: Vertices(std::move(vertices)),
				  Indices(std::move(indices))
		{
		}
	};
}

#endif //PHUSIS_MESH_HXX
