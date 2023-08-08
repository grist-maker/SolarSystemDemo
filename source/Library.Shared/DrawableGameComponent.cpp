#include "pch.h"
#include "DrawableGameComponent.h"

using namespace std;

namespace Library
{
	RTTI_DEFINITIONS(DrawableGameComponent)

	DrawableGameComponent::DrawableGameComponent(Game& game, shared_ptr<Camera> camera) :
		GameComponent(game), mCamera(camera)
	{
	}

	bool DrawableGameComponent::Visible() const
	{
		return mVisible;
	}

	void DrawableGameComponent::SetVisible(bool visible)
	{
		mVisible = visible;
	}

	shared_ptr<Camera> DrawableGameComponent::GetCamera()
	{
		return mCamera;
	}

	void DrawableGameComponent::SetCamera(const shared_ptr<Camera>& camera)
	{
		mCamera = camera;
	}

	void DrawableGameComponent::Draw(const GameTime&)
	{
	}
}