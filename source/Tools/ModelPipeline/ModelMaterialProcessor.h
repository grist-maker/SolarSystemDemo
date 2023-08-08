#pragma once

#include <memory>
#include <cstdint>
#include "ModelMaterial.h"

struct aiMaterial;

namespace Library
{
	class Model;
	class ModelMaterial;
}

namespace ModelPipeline
{
    class ModelMaterialProcessor final
    {
    public:
		ModelMaterialProcessor() = delete;
		
		//static std::shared_ptr<Library::ModelMaterial> LoadModelMaterial(Library::Model& model, aiMaterial& material);

	private:
        static const std::map<Library::TextureType, std::uint32_t> sTextureTypeMappings;
    };
}