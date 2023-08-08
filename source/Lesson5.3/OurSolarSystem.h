#pragma once

#include <gsl\gsl>
#include <winrt\Windows.Foundation.h>
#include <d3d11.h>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include <PointLight.h>
#include "Mesh.h"
#include "Skybox.h"
#include "Texture2D.h"
#include "BasicMaterial.h"

namespace Library
{
	class ProxyModel;
}

namespace Rendering
{
	class PointLightMaterial;

	class OurSolarSystem final : public Library::DrawableGameComponent
	{
	public:
		OurSolarSystem(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		OurSolarSystem(const OurSolarSystem&) = delete;
		OurSolarSystem(OurSolarSystem&&) = default;
		OurSolarSystem& operator=(const OurSolarSystem&) = default;		
		OurSolarSystem& operator=(OurSolarSystem&&) = default;
		~OurSolarSystem();

		/// <summary>
		/// The CelestialBody struct stores information relevant to bodies within the Solar System that rotate and orbit with their own speeds, scales, and textures at different axial tilts, with unique names.
		/// </summary>
		struct CelestialBody
		{
			std::string Name;
			std::shared_ptr<PointLightMaterial> Material;
			std::shared_ptr<Library::Texture2D> Texture;
			std::wstring ColorTextureName = L"Textures\\EarthColorMap.dds";
			std::wstring SpecularTextureName = L"Textures\\NoReflection.dds";
			DirectX::XMMATRIX Location = DirectX::XMMatrixIdentity();
			DirectX::XMFLOAT4X4 WorldMatrix{ Library::MatrixHelper::Identity };
			float OrbitalPeriod = 0.0025f;
			float CurrentOrbitDegrees = 0;
			float CurrentRotation = 0;
			float OrbitalDistance = 40;
			float RotationalPeriod{ DirectX::XM_PI };
			float AxialTilt = 23.5f / 90.0f;
			float Scale = .4f;
		};

		/// <summary>
		/// Creates a new Celestial Body, creating and initializing details like material, texture, location, and light settings.
		/// <param name="Body">The body being adjusted, passed by reference.</param>
		/// </summary>
		void CreateBody(CelestialBody& Body);
		/// <summary>
		/// Controls orbiting behavior of Celestial Bodies. This includes translating, rotating, and scaling them appropriately to rotate around their own axis and orbit as a satellite to a provided body
		/// at their axial tilt. This Satellite is considered the origin if not euqal to the body argument provided. In this Solar system, the Sun is at the origin point, so this is sufficient.
		/// </summary>
		/// <param name="gameTime">The elapsed GameTime used to specify how much the body should be moved. Movement is performed as a product of time.</param>
		/// <param name="Body">The Body that is currently orbiting.</param>
		/// <param name="SatelliteTarget">The Body being orbited about. If this is the same as the body, the origin is assumed the orbit point.</param>
		void Orbit(const Library::GameTime& gameTime, CelestialBody& Body, const CelestialBody& SatelliteTarget);
		
		/// <summary>
		/// Updates a single CelestialBody object, particularly the transforms stored by its material. This is done every time the material must be updated.
		/// <param name="Body">The body being adjusted, passed by reference.</param>
		/// </summary>
		void UpdateBody(CelestialBody& Body);

		/// <summary>
		/// This creates the Sun object for the solar system, allowing the user to pass in a color and specular map to display its model onscreen.
		/// </summary>
		/// <param name="ColorMap">The shared pointer to the Color Map texture for the Sun.</param>
		/// <param name="LightMap">The shared pointer to the Specular Map texture for the Sun.</param>
		void CreateSun(std::shared_ptr<Library::Texture2D> ColorMap, std::shared_ptr<Library::Texture2D>LightMap);

		/// <summary>
		/// Slows down the rotation and orbit of all bodies in the system.
		/// </summary>
		void SlowDown();
		/// <summary>
		/// Speeds up the rotation and orbit of all bodies in the system.
		/// </summary>
		void SpeedUp();

		/// <summary>
		/// Returns a boolean denoting whether or not animation is currently enabled in the simulation
		/// </summary>
		/// <returns></returns>
		bool AnimationEnabled() const;
		/// <summary>
		/// Toggles the animation, switching whatever state it's currently in.
		/// </summary>
		void ToggleAnimation();

		/// <summary>
		/// Initializes all necessary Solar System components to allow them to be used later in the program.
		/// </summary>
		virtual void Initialize() override;
		/// <summary>
		/// Updates all updateable components within the OurSolarSystem object. This update is performed with regard to the GameTime argument provided.
		/// <param name="gameTime">GameTime(based on an in-game clock) elapsed this frame.</param>
		/// </summary>
		virtual void Update(const Library::GameTime& gameTime) override;
		/// <summary>
		/// Draws all drawable components within the OurSolarSystem object. This draw is performed with regard to the GameTime argument provided.
		/// <param name="gameTime">GameTime(based on an in-game clock) elapsed this frame.</param>
		/// </summary>
		virtual void Draw(const Library::GameTime& gameTime) override;
		/// <summary>
		/// Draws all celestial body materials within the solar system, used as a helper function to keep code organized and readable.
		/// </summary>
		void DrawMaterials();

		/// <summary>
		/// This creates all the orbit lines through a series of many line segments.They are stored off for drawing later in the program.
		/// </summary>
		void InitializeOrbitLines();

		///This is the orbital speed variable currently being used for the Earth's orbital period. It is public to allow it to be displayed to the user onscreen when adjusting rotation and movement rates of the
		/// solar system bodies.
		float OrbitalSpeed = 0.0025f;
	private:
		/// <summary>
		/// Sets the animation to enabled or disabled, generally bound to a key-press to let the user toggle it.
		/// </summary>
		/// <param name="enabled">Whether or not the animation should be enabled.</param>
		void SetAnimationEnabled(bool enabled);

		//These variables specify the planet model data. This can be reused for all bodies, so they are stored generically for reuse, independent of the exact body being defined.
		winrt::com_ptr<ID3D11Buffer> PlanetVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> PlanetIndexBuffer;
		std::uint32_t PlanetIndexCount{ 0 };

		/// <summary>
		/// These are all the Celestial bodies. The Earth is used as the standard for all other bodies
		/// </summary>
		CelestialBody Earth;

		CelestialBody Moon{std::string("Moon"), nullptr, nullptr,  L"Textures\\MoonMap.dds", L"Textures\\NoReflection.dds",
		DirectX::XMMatrixIdentity(), Library::MatrixHelper::Identity, 365.0f / 27.3f, 0, 0,
		Earth.OrbitalDistance * 0.08f, 1, 6.7f / 90.0f, Earth.Scale/4 };

		CelestialBody Mercury{ std::string("Mercury"), nullptr, nullptr,  L"Textures\\MercuryMap.dds", L"Textures\\NoReflection.dds",
		DirectX::XMMatrixIdentity(), Library::MatrixHelper::Identity, 1.0f/0.241f, 0, 0,
		Earth.OrbitalDistance* 0.387f, 1/ 58.646f, 0.01f/90.0f, Earth.Scale *.382f};

		CelestialBody Venus{ std::string("Venus"), nullptr, nullptr,  L"Textures\\VenusMap.dds", L"Textures\\NoReflection.dds",
		DirectX::XMMatrixIdentity(), Library::MatrixHelper::Identity, 1.0f/ 0.615f, 0, 0,
		Earth.OrbitalDistance * 0.723f, 1/ 243.01f, 177.4f / 90.0f, Earth.Scale * .949f };

		CelestialBody Mars{ std::string("Mars"), nullptr, nullptr,  L"Textures\\MarsMap.dds", L"Textures\\NoReflection.dds",
		DirectX::XMMatrixIdentity(), Library::MatrixHelper::Identity, 1.0f/ 1.88f, 0, 0,
		Earth.OrbitalDistance * 1.523f, 1 / 1.0257f, 25.2f / 90.0f, Earth.Scale * .532f };

		CelestialBody Jupiter{ std::string("Jupiter"), nullptr, nullptr,  L"Textures\\JupiterMap.dds", L"Textures\\NoReflection.dds",
		DirectX::XMMatrixIdentity(), Library::MatrixHelper::Identity, 1.0f/11.86f, 0, 0,
		Earth.OrbitalDistance * 5.205f, 1 / 0.4097f, 3.1f / 90.0f, Earth.Scale * 11.19f };

		CelestialBody Saturn{ std::string("Saturn"), nullptr, nullptr,  L"Textures\\SaturnMap.dds", L"Textures\\NoReflection.dds",
		DirectX::XMMatrixIdentity(), Library::MatrixHelper::Identity, 1.0f / 29.42f, 0, 0,
		Earth.OrbitalDistance *9.582f, 1 / 0.4264f, 26.7f / 90.0f, Earth.Scale *9.26f };

		CelestialBody Uranus{ std::string("Uranus"), nullptr, nullptr,  L"Textures\\UranusMap.dds", L"Textures\\NoReflection.dds",
		DirectX::XMMatrixIdentity(), Library::MatrixHelper::Identity, 1.0f / 83.75f, 0, 0,
		Earth.OrbitalDistance * 19.2f, 1 / 0.7167f, 97.8f / 90.0f, Earth.Scale * 4.01f };

		CelestialBody Neptune{ std::string("Neptune"), nullptr, nullptr,  L"Textures\\NeptuneMap.dds", L"Textures\\NoReflection.dds",
		DirectX::XMMatrixIdentity(), Library::MatrixHelper::Identity, 1.0f / 163.72f, 0, 0,
		Earth.OrbitalDistance * 30.05f, 1 / 0.67125f, 28.3f / 90.0f, Earth.Scale * 3.88f };

		CelestialBody Pluto{ std::string("Pluto"), nullptr, nullptr,  L"Textures\\PlutoMap.dds", L"Textures\\NoReflection.dds",
		DirectX::XMMatrixIdentity(), Library::MatrixHelper::Identity, 1.0f / 247.93f, 0, 0,
		Earth.OrbitalDistance * 39.48f, 1 / 6.3874f, 122.5f / 90.0f, Earth.Scale * 0.18f};

		/// <summary>
		/// This is the point light created to simulate the Sun's light in the solar system.
		/// </summary>
		std::shared_ptr<Library::PointLight> SunPointLight;
		//The sun model itself
		std::unique_ptr<Library::ProxyModel> SunModel;
		//The material for the Sun model
		std::shared_ptr<PointLightMaterial> SunMaterial;
		//The sun's world matrix
		DirectX::XMFLOAT4X4 SunWorldMatrix{ Library::MatrixHelper::Identity };
		//The sun's current rotation
		float SunCurrentRotation{ 0.0f };
		//The scale of the sun
		float SunScale = 1;

		//This is a pointer to the skybox of space
		std::unique_ptr<Library::Skybox> SpaceBackdrop;

		//These booleans check if the material needs to be updated and if animation is ongoing respectively
		bool UpdateMaterial{ true };
		bool IsAnimationEnabled = true;

		//An array of all Celestial bodies that require orbit lines, cycled through when generating lines
		CelestialBody Bodies[9] = { Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune, Pluto };

		//These variables are used to render orbital lines
		Library::BasicMaterial OrbitMaterial;
		winrt::com_ptr<ID3D11Buffer> OrbitVertexBuffer;
		DirectX::XMFLOAT4 OrbitColor{ 0.961f, 0.871f, 0.702f, 1.0f };
		DirectX::XMFLOAT4X4 OrbitWorldMatrix{ Library::MatrixHelper::Identity };
	};
}