#pragma once
#include <Vector>
#include "VertexData.h"
#include "Material.h"

// ModelData構造体
struct ModelData final {
	std::vector<VertexData> vertices;
	MaterialData material;
};

