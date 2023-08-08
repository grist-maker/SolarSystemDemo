#pragma once

#include "Model.h"
#include <memory>

struct aiNode;

namespace Library
{
	struct ModelData;
	class SceneNode;
}

namespace ModelPipeline
{
    struct ModelProcessor final
    {
		ModelProcessor() = delete;

		//static Library::Model LoadModel(const std::string& filename, bool flipUVs = false);		
    };
}
