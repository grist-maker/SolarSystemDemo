#include "pch.h"
#include "RenderingGame.h"
#include "GameException.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "FpsComponent.h"
#include "OurSolarSystem.h"
#include "Grid.h"
#include "FirstPersonCamera.h"
#include "SamplerStates.h"
#include "RasterizerStates.h"
#include "VectorHelper.h"
#include "ImGuiComponent.h"
#include "imgui_impl_dx11.h"
#include "UtilityWin32.h"
#include <limits>

using namespace std;
using namespace DirectX;
using namespace Library;

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Rendering
{
	RenderingGame::RenderingGame(std::function<void* ()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback) :
		Game(getWindowCallback, getRenderTargetSizeCallback)
	{
	}

	void RenderingGame::Initialize()
	{
		SamplerStates::Initialize(Direct3DDevice());
		RasterizerStates::Initialize(Direct3DDevice());

		mKeyboard = make_shared<KeyboardComponent>(*this);
		mComponents.push_back(mKeyboard);
		mServices.AddService(KeyboardComponent::TypeIdClass(), mKeyboard.get());

		mMouse = make_shared<MouseComponent>(*this, MouseModes::Absolute);
		mComponents.push_back(mMouse);
		mServices.AddService(MouseComponent::TypeIdClass(), mMouse.get());

		mGamePad = make_shared<GamePadComponent>(*this);
		mComponents.push_back(mGamePad);
		mServices.AddService(GamePadComponent::TypeIdClass(), mGamePad.get());

		auto camera = make_shared<FirstPersonCamera>(*this);
		mComponents.push_back(camera);
		mServices.AddService(Camera::TypeIdClass(), camera.get());

		//Creating the solar system model as a single component
		mSolarSystem = make_shared<OurSolarSystem>(*this, camera);
		mComponents.push_back(mSolarSystem);

		//Making a "Guide" for controls to be visible onscreen
		auto imGui = make_shared<ImGuiComponent>(*this);
		mComponents.push_back(imGui);
		mServices.AddService(ImGuiComponent::TypeIdClass(), imGui.get());
		auto imGuiWndProcHandler = make_shared<UtilityWin32::WndProcHandler>(ImGui_ImplWin32_WndProcHandler);
		UtilityWin32::AddWndProcHandler(imGuiWndProcHandler);

		//Definition of different areas of control list
		auto helpTextImGuiRenderBlock = make_shared<ImGuiComponent::RenderBlock>([this]()
			{
				ImGui::Begin("Controls");
				ImGui::SetNextWindowPos(ImVec2(10, 10));

				stringstream fpsLabel;
				fpsLabel << setprecision(3) << "Frame Rate: " << mFpsComponent->FrameRate() << "    Total Elapsed Time: " << mGameTime.TotalGameTimeSeconds().count();
				ImGui::Text(fpsLabel.str().c_str());

				ImGui::Text("Camera (WASD + Left-Click-Mouse-Look)");
				ImGui::Text("Rotate Directional Light (Arrow Keys)");

				stringstream animationEnabledLabel;
				animationEnabledLabel << "Toggle Animation (Space): " << (mSolarSystem->AnimationEnabled() ? "Enabled" : "Disabled");
				ImGui::Text(animationEnabledLabel.str().c_str());

				stringstream SpeedChangeLabel;
				SpeedChangeLabel << "Speed Up (G) and Slow Down (H): " << mSolarSystem->OrbitalSpeed;
				ImGui::Text(SpeedChangeLabel.str().c_str());
				ImGui::End();
			});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		//Counts elapsed time and frame rate for display to user on control menu
		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		Game::Initialize();

		camera->SetPosition(0.0f, 20.0f, 80.0f);
	}

	void RenderingGame::Update(const GameTime& gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape) || mGamePad->WasButtonPressedThisFrame(GamePadButtons::Back))
		{
			Exit();
		}

		//Allow the user to hold the mouse to look around in the simulation, adjusting camera view
		if (mMouse->WasButtonPressedThisFrame(MouseButtons::Left))
		{
			mMouse->SetMode(MouseModes::Relative);
		}

		if (mMouse->WasButtonReleasedThisFrame(MouseButtons::Left))
		{
			mMouse->SetMode(MouseModes::Absolute);
		}

		//Toggle animation on and off
		if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
		{
			mSolarSystem->ToggleAnimation();
		}

		//Speeds up the rotation and orbit speeds of bodies in the solar system.
		if (mKeyboard->WasKeyPressedThisFrame(Keys::G))
		{
			mSolarSystem->SpeedUp();
		}
		//Slows down the rotation and orbit speeds of bodies in the solar system.
		if (mKeyboard->WasKeyPressedThisFrame(Keys::H))
		{
			mSolarSystem->SlowDown();
		}

		Game::Update(gameTime);
	}

	void RenderingGame::Draw(const GameTime& gameTime)
	{
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView.get(), BackgroundColor.f);
		mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		//This single call draws all components attached to the game.
		Game::Draw(gameTime);

		HRESULT hr = mSwapChain->Present(1, 0);

		// If the device was removed either by a disconnection or a driver upgrade, we must recreate all device resources.
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			HandleDeviceLost();
		}
		else
		{
			ThrowIfFailed(hr, "IDXGISwapChain::Present() failed.");
		}
	}

	void RenderingGame::Shutdown()
	{
		mFpsComponent = nullptr;
		mSolarSystem = nullptr;
		RasterizerStates::Shutdown();
		SamplerStates::Shutdown();
		Game::Shutdown();
	}

	void RenderingGame::Exit()
	{
		PostQuitMessage(0);
	}
}