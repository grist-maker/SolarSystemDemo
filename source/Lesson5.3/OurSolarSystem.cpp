#include "pch.h"
#include "OurSolarSystem.h"
#include "FirstPersonCamera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Model.h"
#include "ProxyModel.h"
#include "PointLightMaterial.h"
#include <GameTime.cpp>

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	OurSolarSystem::OurSolarSystem(Game & game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera),
		OrbitMaterial(*mGame)
	{
	}

	OurSolarSystem::~OurSolarSystem()
	{
	}

	bool OurSolarSystem::AnimationEnabled() const
	{
		return IsAnimationEnabled;
	}

	void OurSolarSystem::SetAnimationEnabled(bool enabled)
	{
		IsAnimationEnabled = enabled;
	}

	void OurSolarSystem::ToggleAnimation()
	{
		IsAnimationEnabled = !IsAnimationEnabled;
	}

	void OurSolarSystem::SpeedUp()
	{
		if (Earth.OrbitalPeriod + 0.0001f < 0.005f)
		{
			OrbitalSpeed += 0.0001f;
			Earth.OrbitalPeriod += 0.0001f;
			Earth.RotationalPeriod += DirectX::XM_PI/26;
		}
	}

	void OurSolarSystem::SlowDown()
	{
		if (Earth.OrbitalPeriod - 0.0001f >= 0.0001f)
		{
			OrbitalSpeed -= 0.0001f;
			Earth.OrbitalPeriod -= 0.0001f;
			Earth.RotationalPeriod -= DirectX::XM_PI / 26;
		}
	}

	void OurSolarSystem::InitializeOrbitLines()
	{
		OrbitMaterial.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		OrbitMaterial.Initialize();
		OrbitMaterial.SetSurfaceColor(OrbitColor);

		ID3D11Device* direct3DDevice = GetGame()->Direct3DDevice();

		//Each orbit line will have 10,000 line segments to simulate a smooth circle. There must be one orbit line per planet (including Pluto the faker), so we need 90,000 segments in total. We size it accordingly.
		int size = sizeof(VertexPosition) * 10000 * sizeof(Bodies) / sizeof(Bodies[0]);
		std::unique_ptr<VertexPosition> vertexData(new VertexPosition[10000 * sizeof(Bodies) / sizeof(Bodies[0])]);
		VertexPosition* vertices = vertexData.get();

		//The outer loop cycles through each body
		for (int i = 0; i < sizeof(Bodies) / sizeof(Bodies[0]); i++)
		{
			//While the inner loop creates the segments for that bodies orbit
			for (int j = 0; j < 10000; j++)
			{
				DirectX::XMFLOAT4 Offset{ 0, 0, 0, 1 };
				Offset.x += Bodies[i].OrbitalDistance * cos(j * DirectX::XM_2PI / 10000);
				Offset.z += Bodies[i].OrbitalDistance * sin(j * DirectX::XM_2PI / 10000);

				vertices[j + i *10000] = VertexPosition(Offset);
			}
		}
		//We form the lines and ensure everything was created properly.
		D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = size;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData{ 0 };
		vertexSubResourceData.pSysMem = vertices;

		ThrowIfFailed(direct3DDevice->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, OrbitVertexBuffer.put()), "ID3D11Device::CreateBuffer() failed");
	}
	void OurSolarSystem::Initialize()
	{
		auto direct3DDevice = mGame->Direct3DDevice();
		const auto PlanetModel = mGame->Content().Load<Model>(L"Models\\Sphere.obj.bin"s);
		Mesh* PlanetMesh = PlanetModel->Meshes().at(0).get();

		auto sunTexture = mGame->Content().Load<Texture2D>(L"Textures\\SunMap.dds"s);
		auto PlanetSpecular = mGame->Content().Load<Texture2D>(L"Textures\\NoReflection.dds"s);
		
		VertexPositionTextureNormal::CreateVertexBuffer(direct3DDevice, *PlanetMesh, not_null<ID3D11Buffer**>(PlanetVertexBuffer.put()));
		PlanetMesh->CreateIndexBuffer(*direct3DDevice, not_null<ID3D11Buffer**>(PlanetIndexBuffer.put()));
		PlanetIndexCount = narrow<uint32_t>(PlanetMesh->Indices().size());

		//We initialize the orbit lines for each planet for easier reading
		InitializeOrbitLines();

		//As well as a nice space backdrop for the skybox
		SpaceBackdrop = make_unique<Skybox>(*mGame, mCamera, L"Textures\\SpaceMap.dds"s, 500.0f);
		SpaceBackdrop->Initialize();

		//Specifies earth's specular map
		Earth.SpecularTextureName = L"Textures\\EarthSpecularMap.dds";

		//Creates the Sun
		CreateSun(sunTexture, PlanetSpecular);

		//Creates all the planets
		CreateBody(Mercury);
		CreateBody(Venus);
		CreateBody(Earth);
		CreateBody(Moon);
		CreateBody(Mars);
		CreateBody(Jupiter);
		CreateBody(Saturn);
		CreateBody(Uranus);
		CreateBody(Neptune);
		CreateBody(Pluto);

		auto updateMaterialFunc = [this]() { UpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		auto firstPersonCamera = mCamera->As<FirstPersonCamera>();
		if (firstPersonCamera != nullptr)
		{
			firstPersonCamera->AddPositionUpdatedCallback([this]()
			{
				//Ensures all materials know where the camera is positioned
				Mercury.Material->UpdateCameraPosition(mCamera->Position());
				Venus.Material->UpdateCameraPosition(mCamera->Position());
				Earth.Material->UpdateCameraPosition(mCamera->Position());
				Moon.Material->UpdateCameraPosition(mCamera->Position());
				Mars.Material->UpdateCameraPosition(mCamera->Position());
				Jupiter.Material->UpdateCameraPosition(mCamera->Position());
				Saturn.Material->UpdateCameraPosition(mCamera->Position());
				Uranus.Material->UpdateCameraPosition(mCamera->Position());
				Neptune.Material->UpdateCameraPosition(mCamera->Position());
				Pluto.Material->UpdateCameraPosition(mCamera->Position());
			});
		}
	}


	void OurSolarSystem::CreateSun(std::shared_ptr<Library::Texture2D> ColorMap, std::shared_ptr<Library::Texture2D>LightMap)
	{
		SunPointLight = make_unique<PointLight>();
		SunModel = make_unique<ProxyModel>(*mGame, mCamera, "Models\\Sphere.obj.bin"s, SunScale);

		SunPointLight->SetPosition(0.0f, 0.0f, 0.0f);

		//Specifies the sun model itself, as well as ensuring it and the point light are initialized
		SunMaterial = make_shared<PointLightMaterial>(*mGame, ColorMap, LightMap);
		SunMaterial->Initialize();
		SunModel->Initialize();
		SunModel->SetPosition(0.0f, 0.0, 0.0f);

		SunMaterial->SetLightPosition(SunPointLight->Position());
		SunMaterial->SetAmbientColor(XMFLOAT4(1, 1, 1, 0));
		SunMaterial->SetLightRadius(12000);
	}

	void OurSolarSystem::CreateBody(CelestialBody& Body)
	{
		auto textureColor = mGame->Content().Load<Texture2D>(Body.ColorTextureName);
		auto textureSpecular = mGame->Content().Load<Texture2D>(Body.SpecularTextureName);

		Body.Material = make_shared<PointLightMaterial>(*mGame, textureColor, textureSpecular);
		Body.Material->Initialize();
		Body.Location = XMMatrixIdentity();
		XMStoreFloat4x4(&Body.WorldMatrix, XMMatrixScaling(Body.Scale, Body.Scale, Body.Scale) * Body.Location);

		Body.Material->SetLightPosition(SunPointLight->Position());
		Body.Material->SetLightRadius(12000);

		Body.Material->UpdateCameraPosition(mCamera->Position());
	}

	void OurSolarSystem::Update(const GameTime& gameTime)
	{
		if (AnimationEnabled())
		{
			Orbit(gameTime, Mercury, Mercury);
			Orbit(gameTime, Venus, Venus);
			Orbit(gameTime, Earth, Earth);
			Orbit(gameTime, Moon, Earth);
			Orbit(gameTime, Mars, Mars);
			Orbit(gameTime, Jupiter, Jupiter);
			Orbit(gameTime, Saturn, Saturn);
			Orbit(gameTime, Uranus, Uranus);
			Orbit(gameTime, Neptune, Neptune);
			Orbit(gameTime, Pluto, Pluto);

			SunCurrentRotation += Earth.RotationalPeriod/1000;
			XMStoreFloat4x4(&SunWorldMatrix, XMMatrixRotationY(SunCurrentRotation) * XMMatrixScaling(SunScale, SunScale, SunScale));
			SunModel->Update(gameTime);
		}
		UpdateMaterial = true;
	}

	//Lets a body orbit around a central point: either the origin or the satellite (if it differs from the planet)
	void OurSolarSystem::Orbit(const GameTime& gameTime, CelestialBody& Body, const CelestialBody& SatelliteTarget)
	{
		//If the body is not Earth, we 
		if (Body.Name != Earth.Name)
		{
			Body.CurrentRotation += gameTime.ElapsedGameTimeSeconds().count() * Body.RotationalPeriod*Earth.RotationalPeriod;
		}
		else
		{
			Body.CurrentRotation += gameTime.ElapsedGameTimeSeconds().count() * Body.RotationalPeriod;
		}

		XMStoreFloat4x4(&Body.WorldMatrix, XMMatrixScaling(Body.Scale, Body.Scale, Body.Scale) * XMMatrixRotationY(Body.CurrentRotation) * XMMatrixRotationZ(Body.AxialTilt) * Body.Location);
		DirectX::XMFLOAT3 Offset{ 0, 0, 0 };

		//If the Satellite and Body are different bodies, we need to set a translation corresponding to the Body being orbited around
		if (SatelliteTarget.Name != Body.Name)
		{
			MatrixHelper::GetTranslation(SatelliteTarget.Location, Offset);
			Offset.x += Body.OrbitalDistance * (cos(Body.CurrentOrbitDegrees + SatelliteTarget.CurrentOrbitDegrees));
			Offset.z += Body.OrbitalDistance * (sin(Body.CurrentOrbitDegrees + SatelliteTarget.CurrentOrbitDegrees));
		}
		else
		{
			Offset.x -= Body.OrbitalDistance * cos(Body.CurrentOrbitDegrees);
			Offset.z -= Body.OrbitalDistance * sin(Body.CurrentOrbitDegrees);
		}

		//If not Earth, we need to offset this based on Earth's orbital period.
		if (Body.Name != Earth.Name)
		{
			Body.CurrentOrbitDegrees -= (Body.OrbitalPeriod * Earth.OrbitalPeriod);
		}
		else
		{
			Body.CurrentOrbitDegrees -= Body.OrbitalPeriod;
		}
		MatrixHelper::SetTranslation(Body.Location, Offset);
	}

	//Updates a single celestial body and its transforms
	void OurSolarSystem::UpdateBody(CelestialBody& Body)
	{
		const XMMATRIX PlanetWorldMatrix = XMLoadFloat4x4(&Body.WorldMatrix);
		const XMMATRIX Planetwvp = XMMatrixTranspose(PlanetWorldMatrix * mCamera->ViewProjectionMatrix());
		Body.Material->UpdateTransforms(Planetwvp, XMMatrixTranspose(PlanetWorldMatrix));
	}

	void OurSolarSystem::Draw(const GameTime& gameTime)
	{
		//Check if it's necessary to redraw the drawable components
		if (UpdateMaterial)
		{
			//Drawing the skybox
			SpaceBackdrop->Draw(gameTime);

			const XMMATRIX worldMatrix = XMLoadFloat4x4(&OrbitWorldMatrix);
			const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
			OrbitMaterial.UpdateTransform(wvp);
			
			//Drawing celestial bodies
			UpdateBody(Mercury);
			UpdateBody(Venus);
			UpdateBody(Earth);
			UpdateBody(Moon);
			UpdateBody(Mars);
			UpdateBody(Jupiter);
			UpdateBody(Saturn);
			UpdateBody(Uranus);
			UpdateBody(Neptune);
			UpdateBody(Pluto);
			//Drawing the sun
			const XMMATRIX sunworldMatrix = XMLoadFloat4x4(&SunWorldMatrix);
			const XMMATRIX sunwvp = XMMatrixTranspose(sunworldMatrix * mCamera->ViewProjectionMatrix());
			SunMaterial->UpdateTransforms(sunwvp, XMMatrixTranspose(sunworldMatrix));
			//We no longer need to update the materials
			UpdateMaterial = false;
		}
		//Here we draw each of the orbit lines
		OrbitMaterial.Draw(not_null<ID3D11Buffer*>(OrbitVertexBuffer.get()), 10000 * sizeof(Bodies) / sizeof(Bodies[0]), 0);
		//Drawing all drawable component materials
		DrawMaterials();
	}

	void OurSolarSystem::DrawMaterials()
	{
		Mercury.Material->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
		Venus.Material->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
		Moon.Material->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
		Earth.Material->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
		Mars.Material->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
		Jupiter.Material->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
		Saturn.Material->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
		Uranus.Material->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
		Neptune.Material->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
		Pluto.Material->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
		SunMaterial->DrawIndexed(not_null<ID3D11Buffer*>(PlanetVertexBuffer.get()), not_null<ID3D11Buffer*>(PlanetIndexBuffer.get()), PlanetIndexCount);
	}
}