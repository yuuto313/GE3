#pragma once
#include <Vector>
#include "VertexData.h"
#include "MaterialLibrary.h"

// ModelData構造体
struct ModelData final {
	std::vector<VertexData> vertices;
	MaterialData material;
};

