#include "pch.h"
#include "TextureCube.h"

using namespace winrt;

namespace Library
{
	RTTI_DEFINITIONS(TextureCube)

	TextureCube::TextureCube(const com_ptr<ID3D11ShaderResourceView>& shaderResourceView) :
		Texture(shaderResourceView)
	{
	}
}