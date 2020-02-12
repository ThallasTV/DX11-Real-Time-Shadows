
//
// Scene.cpp
//

#include <stdafx.h>
#include <string.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <System.h>
#include <DirectXTK\DDSTextureLoader.h>
#include <DirectXTK\WICTextureLoader.h>
#include <CGDClock.h>
#include <Scene.h>

#include <Effect.h>
#include <VertexStructures.h>
#include <Texture.h>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;


// Methods to handle initialisation, update and rendering of the scene
HRESULT Scene::rebuildViewport(){
	// Binds the render target view and depth/stencil view to the pipeline.
	// Sets up viewport for the main window (wndHandle) 
	// Called at initialisation or in response to window resize
	ID3D11DeviceContext *context = system->getDeviceContext();
	if (!context)
		return E_FAIL;
	// Bind the render target view and depth/stencil view to the pipeline.
	ID3D11RenderTargetView* renderTargetView = system->getBackBufferRTV();
	context->OMSetRenderTargets(1, &renderTargetView, system->getDepthStencil());
	// Setup viewport for the main window (wndHandle)
	RECT clientRect;
	GetClientRect(wndHandle, &clientRect);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(clientRect.right - clientRect.left);
	viewport.Height = static_cast<FLOAT>(clientRect.bottom - clientRect.top);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//Set Viewport
	context->RSSetViewports(1, &viewport);
	return S_OK;
}

HRESULT Scene::shadowMappingViewport()
{
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Width = static_cast<float>(8192);
	mViewport.Height = static_cast<float>(8192);
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	return S_OK;
}

// Main resource setup for the application.  These are setup around a given Direct3D device.
HRESULT Scene::initialiseSceneResources() {
	ID3D11DeviceContext *context = system->getDeviceContext();
	ID3D11Device *device = system->getDevice();
	if (!device)
		return E_FAIL;
	// Set up viewport for the main window (wndHandle) 
	rebuildViewport();
	shadowMappingViewport();
	initShadowMap();
	
	// Setup main effects (pipeline shaders, states etc)

	// The Effect class is a helper class similar to the depricated DX9 Effect. 
	// It stores pipeline shaders, pipeline states  etc and binds them to setup the pipeline to render with a particular Effect.
	// The constructor requires that at least shaders are provided along a description of the vertex structure.
	basicColourEffect = new Effect(device, "Shaders\\cso\\basic_colour_vs.cso", "Shaders\\cso\\basic_colour_ps.cso","Shaders\\cso\\basic_colour_gs.cso", basicVertexDesc, ARRAYSIZE(basicVertexDesc));
	basicTextureEffect = new Effect(device, "Shaders\\cso\\basic_texture_vs.cso", "Shaders\\cso\\basic_texture_ps.cso", basicVertexDesc, ARRAYSIZE(basicVertexDesc));
	basicLightingEffect = new Effect(device, "Shaders\\cso\\basic_lighting_vs.cso", "Shaders\\cso\\basic_colour_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	perPixelLightingEffect = new Effect(device, "Shaders\\cso\\per_pixel_lighting_vs.cso", "Shaders\\cso\\per_pixel_lighting_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	//shadowVolumeEffect = new Effect(device, "Shaders\\cso\\basic_shadow_vs.cso", "Shaders\\cso\\basic_shadow_ps.cso", basicVertexDesc, ARRAYSIZE(basicVertexDesc));

	//shadowEffect = new Effect(device, "Shaders\\cso\\basic_shadow_vs.cso", "Shaders\\cso\\basic_shadow_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	reflectionMapEffect = new Effect(device, "Shaders\\cso\\per_pixel_lighting_vs.cso", "Shaders\\cso\\reflection_map_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	// Create a generic  materials and arrays that can be use to create objects below
	Material *whiteMat = new Material();
	whiteMat->setSpecular(XMCOLOR(0.0, 0.0, 0.0,0.0));
	whiteMat->setDiffuse(XMCOLOR(1.0, 1.0, 1.0, 1.0));
	Material *whiteMatArray[] = { whiteMat };

	Material *whiteGloss = new Material();
	whiteGloss->setSpecular(XMCOLOR(1.0, 1.0, 1.0, 1.0));
	whiteGloss->setDiffuse(XMCOLOR(1.0, 1.0, 1.0, 1.0));
	Material *whiteGlossArray[] = { whiteGloss };
	
	Material *chrome = new Material();
	chrome->setSpecular(XMCOLOR(1.0, 1.0, 1.0, 1.0));
	chrome->setDiffuse(XMCOLOR(0.7, 0.7, 0.7, 1.0));
	Material *chromeArray[] = { chrome };
	
	Material *gold = new Material();
	gold->setSpecular(XMCOLOR(0.9, 0.7, 0.4, 1.0));
	gold->setDiffuse(XMCOLOR(0.9, 0.5, 0.1, 1.0));
	Material *goldArray[] = { gold };


	// Setup Objects - the object below are derived from the Base model class
	// The constructors for all objects derived from BaseModel require at least a valid pointer to the main DirectX device
	// And a valid effect with a vertex shader input structure that matches the object vertex structure.
	// In addition to the Texture array pointer mentioned above an additional optional parameters of the BaseModel class are a pointer to an array of Materials along with an int that contains the number of Materials
	// The baseModel class now manages a CBuffer containing model/world matrix properties. It has methods to update the cbuffers if the model/world changes 
	// The render methods of the objects attatch the world/model Cbuffers to the pipeline at slot b0 for vertex and pixel shaders

	//Create grassy ground terrain
	// Setup grass textures
	// The Texture class is a helper class to load textures
	Texture* grassAlpha = new Texture(device, L"Resources\\Textures\\grassAlpha.tif");
	Texture* grassTexture = new Texture(device, L"Resources\\Textures\\grass.png");
	Texture* heightMap = new Texture(device, L"Resources\\Textures\\heightmap2.bmp");
	Texture* normalMap = new Texture(device, L"Resources\\Textures\\normalmap.bmp");
	// Even if we only need 1 texture/shader resource view for an effect we still need to create an array.
	ID3D11ShaderResourceView *grassTextureArray[] = { grassTexture->getShaderResourceView(), grassAlpha->getShaderResourceView(), shadowMapSRV };

	// Setup specialised grass effect (pipeline shaders, states etc)
	grassEffect = new Effect(device, "Shaders\\cso\\grass_vs.cso", "Shaders\\cso\\grass_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));

	// Set up custome effect states for grass
	ID3D11BlendState *grassBlendState = grassEffect->getBlendState();
	D3D11_BLEND_DESC grassBlendDesc;
	grassBlendState->GetDesc(&grassBlendDesc);
	grassBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	grassBlendDesc.AlphaToCoverageEnable = TRUE; // Use pixel coverage info from rasteriser 
	grassBlendState->Release(); device->CreateBlendState(&grassBlendDesc, &grassBlendState);
	grassEffect->setBlendState(grassBlendState);
	
	//Create a new terrain using the grass effect
	ground = new Terrain(context, heightMap->getTexture(), normalMap->getTexture(), device, grassEffect, whiteMatArray, 1, grassTextureArray, 3);
	ground->setWorldMatrix(ground->getWorldMatrix()*XMMatrixTranslation(-20, 0, -25)*XMMatrixScaling(4, 0.5, 4)*XMMatrixRotationY(XMConvertToRadians(45)));
	ground->update(context);

	// Create Bridge 
	// Load bridge textures
	Texture* brickTexture = new Texture(device, L"Resources\\Textures\\brick_DIFFUSE.jpg");
	// The BaseModel class supports multitexturing and the constructor takes a pointer to an array of shader resource views of textures. 
	// Even if we only need 1 texture/shader resource view for an effect we still need to create an array.
	ID3D11ShaderResourceView *brickTextureArray[] = { brickTexture->getShaderResourceView() };
	
	//Load bridge Model - use standard per pixel lighting effect
	// The Model class is derived from the BaseModel class with an additional load method that loads 3d data from a file
	// The load method makes use of the ASSIMP (open ASSet IMPort) Library for loading 3d data http://assimp.sourceforge.net/.
	bridge = new  Model(device, wstring(L"Resources\\Models\\bridge.3ds"), perPixelLightingEffect, whiteMatArray, 1, brickTextureArray, 1);
	
	// Bridge will be placed on the ground at position X=0,Z=0 
	// we can use ground->CalculateYValueWorld(X,Z) to find the ground height at position X=0,Z=0
	float bridgeHeight = ground->CalculateYValueWorld(0, 0);
	// Scale and position the bridge
	bridge->setWorldMatrix(XMMatrixScaling(0.01, 0.01, 0.01)*XMMatrixTranslation(0, bridgeHeight, 0));
	bridge->update(context);

	// Add Trees
	Texture* treeTexture = new Texture(device, context, L"Resources\\Textures\\tree.tif");
	ID3D11ShaderResourceView *treeTextureArray[] = { treeTexture->getShaderResourceView(), grassTexture->getShaderResourceView() };

	treeEffect = new Effect(device, "Shaders\\cso\\tree_vs.cso", "Shaders\\cso\\tree_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	// Set up custome effect states for trees
	// just get a copy of the grassBlendState setup above
	treeEffect->setBlendState(grassEffect->getBlendState()); 

	// Create 3 tree materials and models with subtly different diffuse colours
	Material treeMat;
	treeMat.setSpecular(XMCOLOR(0, 0, 0, 1));
	Material *matArray[] = { &treeMat };
	treeMat.setDiffuse(XMCOLOR(1, 0.5, 0.2, 1));

	trees[0] = new  Model(device, wstring(L"Resources\\Models\\tree.3ds"), treeEffect, matArray, 1, treeTextureArray, 2);
	treeMat.setDiffuse(XMCOLOR(0.7, 1, 0.7, 1));
	trees[1] = new  Model(device, wstring(L"Resources\\Models\\tree.3ds"), treeEffect, matArray, 1, treeTextureArray, 2);
	treeMat.setDiffuse(XMCOLOR(1, 0.5, 0.0, 1));
	trees[2] = new  Model(device, wstring(L"Resources\\Models\\tree.3ds"), treeEffect, matArray, 1, treeTextureArray, 2);

	// Randomly place the trees
	for (int i = 0; i < numTrees; i++)
	{
		// set X and Z randomly
		treeTypeArray[i] = (int)((((float)rand() / RAND_MAX))*(float)numTreeTypes);
		float x = (((float)rand() / RAND_MAX)) - 0.5;
		float z = (((float)rand() / RAND_MAX)) - 0.5;
		float scale = (((float)rand() / RAND_MAX)*0.5 + 0.4);

		x *= 50;
		z *= 50;
		float y;
		//find the height of each tree
		y = ground->CalculateYValueWorld(x, z);
		treeArray[i] = XMMatrixRotationY((((float)rand() / RAND_MAX) - 0.5))*XMMatrixScaling(scale, scale, scale)*XMMatrixTranslation(x, y, z);
	}


	// Setup SkyBox effect and custom pipeline states
	 Effect *skyBoxEffect = new Effect(device, "Shaders\\cso\\sky_box_vs.cso", "Shaders\\cso\\sky_box_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	// We can customise states if required
	ID3D11DepthStencilState *skyBoxDSState = skyBoxEffect->getDepthStencilState();
	D3D11_DEPTH_STENCIL_DESC skyBoxDSDesc;
	skyBoxDSState->GetDesc(&skyBoxDSDesc);
	skyBoxDSDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	skyBoxDSState->Release(); device->CreateDepthStencilState(&skyBoxDSDesc, &skyBoxDSState);
	skyBoxEffect->setDepthStencilState(skyBoxDSState);

	ID3D11RasterizerState *rsStateSky=skyBoxEffect->getRasterizerState();;
	D3D11_RASTERIZER_DESC		RSdesc; 
	rsStateSky->GetDesc(&RSdesc);
	RSdesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&RSdesc, &rsStateSky);
	skyBoxEffect->setRasterizerState(rsStateSky);

	Texture* cubeDayTexture = new Texture(device, L"Resources\\Textures\\grassenvmap1024.dds");
	ID3D11ShaderResourceView *skyBoxTextureArray[] = { cubeDayTexture->getShaderResourceView()};
	// Create a skybox
	// The box class is derived from the BaseModel class 
	box = new Box(device, skyBoxEffect, NULL, 0, skyBoxTextureArray,1);
	// Scale the box x1000
	box->setWorldMatrix(box->getWorldMatrix()*XMMatrixScaling(1000, 1000, 1000));
	box->update(context);

	reflectionMappingEffect = new Effect(device, "Shaders\\cso\\per_pixel_lighting_vs.cso", "Shaders\\cso\\reflection_map_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));

	chromeSphere = new  Model(device, wstring(L"Resources\\Models\\sphere.3ds"), reflectionMappingEffect, whiteGlossArray, 1, skyBoxTextureArray, 1);
	//chromeSphere = new  Model(device, wstring(L"Resources\\Models\\knight.3ds"), reflectionMappingEffect, goldArray, 1, skyBoxTextureArray, 1);

	float sphereHeight = ground->CalculateYValueWorld(10, 0);
	chromeSphere->setWorldMatrix(XMMatrixScaling(1, 1, 1)*XMMatrixTranslation(10, sphereHeight+1, 0)); 
	chromeSphere->update(context);



	Effect *DoomHUDEffect = new Effect(device, "Shaders\\cso\\DoomHUD_vs.cso", "Shaders\\cso\\DoomHUD_ps.cso", basicVertexDesc, ARRAYSIZE(basicVertexDesc));
	ID3D11DepthStencilState *DoomHUDDSState = DoomHUDEffect->getDepthStencilState();
	D3D11_DEPTH_STENCIL_DESC DoomHUDDSDesc;
	DoomHUDDSState->GetDesc(&DoomHUDDSDesc);
	DoomHUDDSDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	DoomHUDDSState->Release(); device->CreateDepthStencilState(&DoomHUDDSDesc, &DoomHUDDSState);
	DoomHUDEffect->setDepthStencilState(DoomHUDDSState);
	
	Texture* DoomHUDTexture = new Texture(device, L"Resources\\Textures\\DoomHUD\\DoomHUD.png");
	Texture* DoomFacesTexture = new Texture(device, L"Resources\\Textures\\DoomHUD\\Faces.png");
	Texture* DoomKeysTexture = new Texture(device, L"Resources\\Textures\\DoomHUD\\Keys.png");
	Texture* DoomWeaponsTexture = new Texture(device, L"Resources\\Textures\\DoomHUD\\Weapons.png");
	Texture* DoomNumbersTexture = new Texture(device, L"Resources\\Textures\\DoomHUD\\Numbers.png");

	// The BaseModel class supports multitexturing and the constructor takes a pointer to an array of shader resource views of textures. 
	// Even if we only need 1 texture/shader resource view for an effect we still need to create an array.
	ID3D11ShaderResourceView *DoomHUDTextureArray[] = { DoomHUDTexture->getShaderResourceView(),DoomFacesTexture->getShaderResourceView(),DoomKeysTexture->getShaderResourceView(),DoomWeaponsTexture->getShaderResourceView(),DoomNumbersTexture->getShaderResourceView() };
	//Create a new instance of a triangle using the basicColourEffect
	HUD = new DoomHUD(device, DoomHUDEffect, NULL, 0, DoomHUDTextureArray, 5);

	//Tutorial  - Shadow Volumes
	Effect *shadowVolumeEffect = new Effect(device, "Shaders\\cso\\basic_shadow_vs.cso", "Shaders\\cso\\basic_shadow_ps.cso", basicVertexDesc, ARRAYSIZE(basicVertexDesc));
	renderShadowDepthStencilState =  perPixelLightingEffect->getDepthStencilState();

	//Depth stencil for shadow render pass
	D3D11_DEPTH_STENCIL_DESC dsDescRenderpass = {};
	renderShadowDepthStencilState->GetDesc(&dsDescRenderpass);

	dsDescRenderpass.DepthEnable = TRUE;
	dsDescRenderpass.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDescRenderpass.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDescRenderpass.StencilEnable = TRUE;
	dsDescRenderpass.StencilReadMask = 0xFF;
	dsDescRenderpass.StencilWriteMask = 0xFF;

	// In other words, we are only using the values to mask pixels.
	dsDescRenderpass.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDescRenderpass.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDescRenderpass.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	// The stencil test passes if the passed parameter is not equal to value in the buffer.
	dsDescRenderpass.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	// The stencil test passes if the passed parameter is not equal to value in the buffer.
	dsDescRenderpass.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDescRenderpass.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDescRenderpass.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDescRenderpass.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

	// Create renderDepthStencil
	device->CreateDepthStencilState(&dsDescRenderpass, &renderShadowDepthStencilState);

	// Get light vec in sphere model space
	XMVECTOR det, pointLightVec = { -125.0, 100.0, 70.0, 1.0 };
	pointLightVec = XMVector3TransformCoord(pointLightVec, XMMatrixInverse(&det, chromeSphere->getWorldMatrix()));
	shadowVolume = new ShadowVolume(device,context, shadowVolumeEffect, pointLightVec, chromeSphere->getVB(), chromeSphere->getVBSizeBytes(), chromeSphere->getIndices(), chromeSphere->getNumFaces(), whiteMatArray, 1, nullptr, 0);
	shadowVolume->setWorldMatrix(chromeSphere->getWorldMatrix());
	shadowVolume->update(context);


	// Setup a camera
	// The LookAtCamera is derived from the base Camera class. The constructor for the Camera class requires a valid pointer to the main DirectX device
	// and and 3 vectors to define the initial position, up vector and target for the camera.
	// The camera class  manages a Cbuffer containing view/projection matrix properties. It has methods to update the cbuffers if the camera moves changes  
	// The camera constructor and update methods also attaches the camera CBuffer to the pipeline at slot b1 for vertex and pixel shaders
	//mainCamera =  new LookAtCamera(device, XMVectorSet(0.0, 0.0, -10.0, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), XMVectorZero());
	mainCamera = new FirstPersonCamera(device, XMVectorSet(-9.0, 2.0, 17.0, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), XMVectorSet(0.8f, 0.0f, -1.0f, 1.0f));
	mainCamera->setFlying(true);

	XMMATRIX T = XMMATRIX(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);


	shadowMapCamera = new LookAtCamera(device, XMVectorSet(-125.0, 100.0, 70.0, 1.0), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), XMVectorZero());
	cBufferShadowCPU = (CBufferShadow*)_aligned_malloc(sizeof(CBufferShadow), 16);
	cBufferShadowCPU->shadowTransformMatrix = shadowMapCamera->getViewMatrix() * shadowMapCamera->getProjMatrix() * T;

	D3D11_BUFFER_DESC cbufferDesc;
	D3D11_SUBRESOURCE_DATA cbufferInitData;
	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&cbufferInitData, sizeof(D3D11_SUBRESOURCE_DATA));

	cbufferDesc.ByteWidth = sizeof(CBufferShadow);
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferInitData.pSysMem = cBufferShadowCPU;// Initialise GPU CBuffer with data from CPU CBuffer

	HRESULT hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData,
		&cBufferShadowGPU);

	context->VSSetConstantBuffers(5, 1, &cBufferShadowGPU);

	static const int numLights = 2;
	// Add a CBuffer to store light properties - you might consider creating a Light Class to manage this CBuffer
	// Allocate 16 byte aligned block of memory for "main memory" copy of cBufferLight
	cBufferLightCPU = (CBufferLight *)_aligned_malloc(sizeof(CBufferLight)*numLights, 16);

	// Fill out cBufferLightCPU
	cBufferLightCPU[0].lightVec = XMFLOAT4(-125.0, 100.0, 70.0,1.0);
	cBufferLightCPU[0].lightAmbient = XMFLOAT4(0.1, 0.1, 0.1, 1.0);
	cBufferLightCPU[0].lightDiffuse = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	cBufferLightCPU[0].lightSpecular = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	cBufferLightCPU[0].lightAttenuation = XMFLOAT4(1, 0.00, 0.0000, 1000.0);//const = 0.0; linear = 0.0; quad = 0.05; cutOff = 200.0;


	cBufferLightCPU[1].lightVec = XMFLOAT4(0.0, 0.0, 0.0, 1.0);
	cBufferLightCPU[1].lightAmbient = XMFLOAT4(0.0, 0.0, 0.0, 1.0);
	cBufferLightCPU[1].lightDiffuse = XMFLOAT4(0.0, 0.0, 0.0, 1.0);
	cBufferLightCPU[1].lightSpecular = XMFLOAT4(0.0, 0.0, 0.0, 1.0);
	cBufferLightCPU[1].lightAttenuation = XMFLOAT4(0.0, 0.0, 0.05, 10.0);//const = 0.0; linear = 0.0; quad = 0.05; cutOff = 10.0;

	// Create GPU resource memory copy of cBufferLight
	// fill out description (Note if we want to update the CBuffer we need  D3D11_CPU_ACCESS_WRITE)
	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&cbufferInitData, sizeof(D3D11_SUBRESOURCE_DATA));

	cbufferDesc.ByteWidth = sizeof(CBufferLight)*2;
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferInitData.pSysMem = cBufferLightCPU;// Initialise GPU CBuffer with data from CPU CBuffer

	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData,
		&cBufferLightGPU);

	//context->VSSetConstantBuffers(5, 1, &cBufferLightGPU);
	// We dont strictly need to call map here as the GPU CBuffer was initialised from the CPU CBuffer at creation.
	// However if changes are made to the CPU CBuffer during update the we need to copy the data to the GPU CBuffer 
	// using the mapCbuffer helper function provided the in Util.h   

	mapCbuffer(context, cBufferLightCPU, cBufferLightGPU, sizeof(CBufferLight)*2);
	context->PSSetConstantBuffers(2, 1, &cBufferLightGPU);// Attach CBufferLightGPU to register b2 for the Pixel shader.
	context->VSSetConstantBuffers(2, 1, &cBufferLightGPU);// Attach CBufferLightGPU to register b2 for the Pixel shader.
	context->GSSetConstantBuffers(2, 1, &cBufferLightGPU);// Attach CBufferLightGPU to register b2 for the Pixel shader.

	// Add a Cbuffer to stor world/scene properties
	// Allocate 16 byte aligned block of memory for "main memory" copy of cBufferLight
	cBufferSceneCPU = (CBufferScene *)_aligned_malloc(sizeof(CBufferScene), 16);

	// Fill out cBufferSceneCPU
	cBufferSceneCPU->windDir = XMFLOAT4(1, 0, 0, 1);
	cBufferSceneCPU->Time = 0.0;
	cBufferSceneCPU->grassHeight = 0.0;
	cBufferSceneCPU->deltaTime = 0.0;
	cBufferSceneCPU->USE_SHADOW_MAP = true;
	cBufferSceneCPU->REFLECTION_PASS = false;
	
	cbufferInitData.pSysMem = cBufferSceneCPU;// Initialise GPU CBuffer with data from CPU CBuffer
	cbufferDesc.ByteWidth = sizeof(CBufferScene);

	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferSceneGPU);

	mapCbuffer(context, cBufferSceneCPU, cBufferSceneGPU, sizeof(CBufferScene));
	context->VSSetConstantBuffers(3, 1, &cBufferSceneGPU);// Attach CBufferSceneGPU to register b3 for the vertex shader. 
	context->PSSetConstantBuffers(3, 1, &cBufferSceneGPU);// Attach CBufferSceneGPU to register b3 for the Pixel shader
	context->GSSetConstantBuffers(3, 1, &cBufferSceneGPU);// Attach CBufferSceneGPU to register b3 for the Geometry shader

	ID3D11ShaderResourceView* shadowMapTextureArray[] = { shadowMapSRV };

	quad = new Quad(device, basicTextureEffect, NULL, 0, shadowMapTextureArray, 1);
	quad->setWorldMatrix(XMMatrixScaling(0.25, 0.25, 0.25) *XMMatrixTranslation(0.75, 0.75, 0.0));
	quad->update(context);



	return S_OK;
}

// Update scene state (perform animations etc)
HRESULT Scene::updateScene(ID3D11DeviceContext *context, Camera *camera) {

	// mainClock is a helper class to manage game time data
	mainClock->tick();
	double dT = mainClock->gameTimeDelta();
	double gT = (mainClock->gameTimeElapsed());
	

	
	HUD->setAnim((int)gT % 3);
	HUD->setHealth(100-((int)gT % 50)*2);
	HUD->setAmmo(100 - ((int)gT % 100));
	HUD->setArmor(100 - ((int)gT % 25)*4);
	
	
	// update camera
	if (!mainCamera->getFlying())//if not flying stick to ground
	{
		XMVECTOR camPos = camera->getPos();
		float camHeight = 1 + ground->CalculateYValueWorld(camPos.vector4_f32[0], camPos.vector4_f32[2]);
		mainCamera->setHeight(camHeight);
	}
	//camera->setPos(camPos);
	camera->update(context);

	//chromeSphere->setWorldMatrix(XMMatrixScaling(sin(gT*4)*0.5+ 1, sin(gT*4)*0.5 + 1, sin(gT*4)*0.5 + 1)*XMMatrixTranslation(10, 5+sin(gT*2)*0.05, 0)*XMMatrixRotationY(gT));
	chromeSphere->setWorldMatrix(XMMatrixScaling(0.5, 0.5, 0.5)*XMMatrixTranslation(5, 2, 0)*XMMatrixRotationY(gT));
	chromeSphere->update(context);

	shadowVolume->setWorldMatrix(chromeSphere->getWorldMatrix());
	shadowVolume->update(context);

	// Get light vec in flag model space
	XMVECTOR det, pointLightVec = { -125.0, 100.0, 70.0, 1.0 };
	pointLightVec = XMVector3TransformCoord(pointLightVec, XMMatrixInverse(&det,chromeSphere->getWorldMatrix()));
	shadowVolume->buildShadowVolume(context,  pointLightVec);

	// Update the scene time as it is needed for animated effects
	cBufferSceneCPU->windDir = XMFLOAT4(0.0, 0.0, cos((FLOAT)mainClock->gameTimeElapsed() * 3), 1.0);
	cBufferSceneCPU->Time = (float)gT;
	cBufferSceneCPU->deltaTime = (float)dT*3;
	mapCbuffer(context, cBufferSceneCPU, cBufferSceneGPU, sizeof(CBufferScene));

	return S_OK;
}



void Scene::DrawGrass(ID3D11DeviceContext *context)
{
	// Draw the Grass
	if (ground) {
		ground->render(context);
		//// Render grass layers from base to tip
		for (int i = 0; i < numGrassPasses; i++)
		{
			cBufferSceneCPU->grassHeight = (grassLength / numGrassPasses)*i;
			mapCbuffer(context, cBufferSceneCPU, cBufferSceneGPU, sizeof(CBufferScene));
			context->PSSetSamplers(1, 1, &shadowSampler);
			ground->render(context);
		}
	}
}



void Scene::DrawObjects(ID3D11DeviceContext *context)
{
	// Render SkyBox
	if (box) {
		box->render(context);
	}
	if (ground)
		DrawGrass(context);

	// Render Head Up Display
	if (HUD)
		HUD->render(context);

	// Render trees
	if (true)
		for (int i = 0; i < numTrees; i++)
		{
			int type = treeTypeArray[i];
			trees[type]->setWorldMatrix(treeArray[i]);
			trees[type]->update(context);
			trees[type]->render(context);
		}
	
	if (quad) {
		quad->render(context);
	}
}

void Scene::randomerinoObjecterino(ID3D11DeviceContext * context)
{
	if (ground)
		DrawGrass(context);

	if (chromeSphere) 
		chromeSphere->render(context);
	
	if (true) {
		for (int i = 0; i < numTrees; i++)
		{
			int type = treeTypeArray[i];
			trees[type]->setWorldMatrix(treeArray[i]);
			trees[type]->update(context);
			trees[type]->render(context);
		}
	}
}

// Render scene
HRESULT Scene::renderScene() {

	ID3D11DeviceContext *context = system->getDeviceContext();
	ID3D11Device *device = system->getDevice();

	// Validate window and D3D context
	if (isMinimised() || !context)
		return E_FAIL;
	
	// Clear the screen
	static const FLOAT clearColor[4] = { 1.0f,0.0f, 0.0f, 0.0f };

	context->RSSetViewports(1, &mViewport);

	ID3D11RenderTargetView *renderTarget[1];

	renderTarget[0] = NULL;

	context->OMSetRenderTargets(1, renderTarget, depthStencilView);

	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	shadowMapCamera->update(context);
	
	ground->setEffect(shadowGrassEffect);
	chromeSphere->setEffect(shadowReflectionMapEffect);
	for (int i = 0; i < numTrees; i++)
	{
		int type = treeTypeArray[i];
		trees[type]->setEffect(shadowTreeEffect);
		
	}

	randomerinoObjecterino(context);

	mainCamera->update(context);

	ground->setEffect(grassEffect);
	chromeSphere->setEffect(reflectionMappingEffect);

	// Render Scene objects

	/*ground->setEffect(grassEffect);
	chromeSphere->setEffect(reflectionMappingEffect);*/
	for (int i = 0; i < numTrees; i++)
	{
		int type = treeTypeArray[i];
		trees[type]->setEffect(treeEffect);

	}

	context->RSSetViewports(1, &viewport);

	mainCamera->update(context);

	renderTarget[0] = system->getBackBufferRTV();

	context->OMSetRenderTargets(1, renderTarget, system->getDepthStencil());

	context->ClearRenderTargetView(system->getBackBufferRTV(), clearColor);
	context->ClearDepthStencilView(system->getDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	DrawObjects(context);

	if (chromeSphere)
		chromeSphere->render(context);

	if (shadowVolume)
		shadowVolume->render(context);

	//use only ambient light turn off difuse and specular
	cBufferLightCPU[0].lightDiffuse = XMFLOAT4(0.0, 0.0, 0.0, 0.0);
	cBufferLightCPU[0].lightSpecular = XMFLOAT4(0.0, 0.0, 0.0, 0.0);
	mapCbuffer(context, cBufferLightCPU, cBufferLightGPU, sizeof(CBufferLight) * 2);

	//// Render grass
	auto groundDSState = ground->getEffect()->getDepthStencilState();
	ground->getEffect()->setDepthStencilState(renderShadowDepthStencilState);

	if (ground)
		DrawGrass(context);

	ground->getEffect()->setDepthStencilState(groundDSState);

	// Set full lighting back
	cBufferLightCPU[0].lightDiffuse = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	cBufferLightCPU[0].lightSpecular = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	mapCbuffer(context, cBufferLightCPU, cBufferLightGPU, sizeof(CBufferLight) * 2);

	//shadowVolume->renderWire(context);

	// Present current frame to the screen
	HRESULT hr = system->presentBackBuffer();

	return S_OK;
}

HRESULT Scene::initShadowMap()
{
	ID3D11DeviceContext *context = system->getDeviceContext();
	ID3D11Device *device = system->getDevice();
	if (!device)
		return E_FAIL;

	shadowTreeEffect = new Effect(device, "Shaders\\cso\\tree_vs.cso", "Shaders\\cso\\tree_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	shadowGrassEffect = new Effect(device, "Shaders\\cso\\grass_vs.cso", "Shaders\\cso\\grass_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	shadowReflectionMapEffect = new Effect(device, "Shaders\\cso\\reflection_map_vs.cso", "Shaders\\cso\\reflection_map_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));

	ID3D11BlendState *grassBlendState = shadowGrassEffect->getBlendState();
	D3D11_BLEND_DESC grassBlendDesc;
	grassBlendState->GetDesc(&grassBlendDesc);
	grassBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	grassBlendDesc.AlphaToCoverageEnable = TRUE; // Use pixel coverage info from rasteriser 
	grassBlendState->Release(); device->CreateBlendState(&grassBlendDesc, &grassBlendState);
	shadowGrassEffect->setBlendState(grassBlendState);
	shadowTreeEffect->setBlendState(grassBlendState);

	ID3D11RasterizerState* shadowRSState = shadowGrassEffect->getRasterizerState();
	D3D11_RASTERIZER_DESC RDesc;
	shadowRSState->GetDesc(&RDesc);
	RDesc.DepthBias = 100.0f;
	RDesc.DepthBiasClamp = 0.0f;
	RDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&RDesc, &shadowRSState);
	

	shadowGrassEffect->setRasterizerState(shadowRSState);
	shadowTreeEffect->setRasterizerState(shadowRSState);
	shadowReflectionMapEffect->setRasterizerState(shadowRSState);

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	// Return 0 for points outside the light frustum   
	// to put them in shadow. AddressU = BORDER;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 0.0f;// { 0.0f, 0.0f, 0.0f, 0.0f };
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateSamplerState(&samplerDesc, &shadowSampler);
	context->PSSetSamplers(1, 1, &shadowSampler);

	depthStencilDesc;
	depthStencilDesc.Width = mViewport.Width; //width of the depth stencil is the new custom viewport width
	depthStencilDesc.Height = mViewport.Height; //width of the depth stencil is the new custom viewport height
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ID3D11Texture2D *depthStencilBuffer = nullptr;

	HRESULT hr = device->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBuffer);

	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	if (SUCCEEDED(hr))
		hr = device->CreateDepthStencilView(depthStencilBuffer, &descDSV, &depthStencilView);

	hr = device->CreateShaderResourceView(depthStencilBuffer, &shaderResourceViewDesc, &shadowMapSRV);

	if (depthStencilBuffer)
		depthStencilBuffer->Release();

	

	return S_OK;
}

//
// Event handling methods
//
// Process mouse move with the left button held down
void Scene::handleMouseLDrag(const POINT &disp) {
	//LookAtCamera

	//mainCamera->rotateElevation((float)-disp.y * 0.01f);
	//mainCamera->rotateOnYAxis((float)-disp.x * 0.01f);

	//FirstPersonCamera
	mainCamera->elevate((float)-disp.y * 0.005f);
	mainCamera->turn((float)disp.x * 0.005f);
}

// Process mouse wheel movement
void Scene::handleMouseWheel(const short zDelta) {
	//LookAtCamera

	//if (zDelta<0)
	//	mainCamera->zoomCamera(1.2f);
	//else if (zDelta>0)
	//	mainCamera->zoomCamera(0.9f);
	//cout << "zoom" << endl;
	//FirstPersonCamera
	mainCamera->move(zDelta*0.01);
}

// Process key down event.  keyCode indicates the key pressed while extKeyFlags indicates the extended key status at the time of the key down event (see http://msdn.microsoft.com/en-gb/library/windows/desktop/ms646280%28v=vs.85%29.aspx).
void Scene::handleKeyDown(const WPARAM keyCode, const LPARAM extKeyFlags) {
	// Add key down handler here...

	// change weapon - numbers 1-9 on keyboard
	if (keyCode >= 0x31 && keyCode <= 0x38)
		HUD->setWeapon((keyCode-0x30)-1);
	
	// enable keys
	if (keyCode == VK_F1)
		HUD->setKey(0,1);
	if (keyCode == VK_F2)
		HUD->setKey(1, 1);
	if (keyCode == VK_F3)
		HUD->setKey(2, 1);

	// disable keys
	if (keyCode == VK_F4)
	{
		HUD->setKey(0, 0);
		HUD->setKey(1, 0);
		HUD->setKey(2, 0);
	}

	if (keyCode == VK_HOME)
		mainCamera->elevate(0.05f);
	
	if (keyCode == VK_END)
		mainCamera->elevate(-0.05f);
	
	if (keyCode == VK_LEFT)
		mainCamera->turn(-0.05f);

	if (keyCode == VK_RIGHT)
		mainCamera->turn(0.05f);
	
	if (keyCode == VK_SPACE)
	{
		bool isFlying = mainCamera->toggleFlying();
		if (isFlying)
			cout << "Flying mode is on" << endl;
		else
			cout << "Flying mode is off" << endl;
	}
	if (keyCode == VK_UP)
		mainCamera->move(0.5);
	
	if (keyCode == VK_DOWN)
		mainCamera->move(-0.5);
	



}
// Process key up event.  keyCode indicates the key released while extKeyFlags indicates the extended key status at the time of the key up event (see http://msdn.microsoft.com/en-us/library/windows/desktop/ms646281%28v=vs.85%29.aspx).
void Scene::handleKeyUp(const WPARAM keyCode, const LPARAM extKeyFlags) {
	// Add key up handler here...
}


// Clock handling methods
void Scene::startClock() {
	mainClock->start();
}

void Scene::stopClock() {
	mainClock->stop();
}
void Scene::resetClock() {
	mainClock->reset();
}

void Scene::reportTimingData() {

	cout << "Actual time elapsed = " << mainClock->actualTimeElapsed() << endl;
	cout << "Game time elapsed = " << mainClock->gameTimeElapsed() << endl << endl;
	mainClock->reportTimingData();
}

// Private constructor
Scene::Scene(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc) {
	try
	{
		// 1. Register window class for main DirectX window
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_CROSS);
		wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = wndClassName;
		wcex.hIconSm = NULL;
		if (!RegisterClassEx(&wcex))
			throw exception("Cannot register window class for Scene HWND");
		// 2. Store instance handle in our global variable
		hInst = hInstance;
		// 3. Setup window rect and resize according to set styles
		RECT		windowRect;
		windowRect.left = 0;
		windowRect.right = _width;
		windowRect.top = 0;
		windowRect.bottom = _height;
		DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;
		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);
		// 4. Create and validate the main window handle
		wndHandle = CreateWindowEx(dwExStyle, wndClassName, wndTitle, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 200, 100, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, hInst, this);
		if (!wndHandle)
			throw exception("Cannot create main window handle");
		ShowWindow(wndHandle, nCmdShow);
		UpdateWindow(wndHandle);
		SetFocus(wndHandle);
		// 5. Create DirectX host environment (associated with main application wnd)
		system = System::CreateDirectXSystem(wndHandle);
		if (!system)
			throw exception("Cannot create Direct3D device and context model");
		// 6. Setup application-specific objects
		HRESULT hr = initialiseSceneResources();
		if (!SUCCEEDED(hr))
			throw exception("Cannot initalise scene resources");
		// 7. Create main clock / FPS timer (do this last with deferred start of 3 seconds so min FPS / SPF are not skewed by start-up events firing and taking CPU cycles).
		mainClock = CGDClock::CreateClock(string("mainClock"), 3.0f);
		if (!mainClock)
			throw exception("Cannot create main clock / timer");
	}
	catch (exception &e)
	{
		cout << e.what() << endl;
		// Re-throw exception
		throw;
	}
}

// Helper function to call updateScene followed by renderScene
HRESULT Scene::updateAndRenderScene() {
	ID3D11DeviceContext *context = system->getDeviceContext();
	HRESULT hr = updateScene(context, (Camera*)mainCamera);
	if (SUCCEEDED(hr))
		hr = renderScene();
	return hr;
}

// Return TRUE if the window is in a minimised state, FALSE otherwise
BOOL Scene::isMinimised() {

	WINDOWPLACEMENT				wp;

	ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);

	return (GetWindowPlacement(wndHandle, &wp) != 0 && wp.showCmd == SW_SHOWMINIMIZED);
}

//
// Public interface implementation
//
// Method to create the main Scene instance
Scene* Scene::CreateScene(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc) {
	static bool _scene_created = false;
	Scene *system = nullptr;
	if (!_scene_created) {
		system = new Scene(_width, _height, wndClassName, wndTitle, nCmdShow, hInstance, WndProc);
		if (system)
			_scene_created = true;
	}
	return system;
}

// Destructor
Scene::~Scene() {
	//Clean Up
	if (wndHandle)
		DestroyWindow(wndHandle);
}

// Call DestoryWindow on the HWND
void Scene::destoryWindow() {
	if (wndHandle != NULL) {
		HWND hWnd = wndHandle;
		wndHandle = NULL;
		DestroyWindow(hWnd);
	}
}

//
// Private interface implementation
//
// Resize swap chain buffers and update pipeline viewport configurations in response to a window resize event
HRESULT Scene::resizeResources() {
	if (system) {
		// Only process resize if the System *system exists (on initial resize window creation this will not be the case so this branch is ignored)
		HRESULT hr = system->resizeSwapChainBuffers(wndHandle);
		rebuildViewport();
		RECT clientRect;
		GetClientRect(wndHandle, &clientRect);
		if (!isMinimised())
			renderScene();
	}
	return S_OK;
}

