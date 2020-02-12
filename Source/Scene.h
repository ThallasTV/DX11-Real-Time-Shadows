
//
// Scene.h
//

// DirectX interfaces and scene variables
#pragma once
#include <Windows.h>
#include <CGDClock.h>
#include <Camera.h>
#include <LookAtCamera.h>
#include <Triangle.h>
#include <Model.h>
#include <Box.h>
#include <Grid.h>
#include <CBufferStructures.h>
#include <FirstPersonCamera.h>
#include <Terrain.h>
#include <Quad.h>
#include <DoomHUD.h>
#include "GPUParticles.h"
#include "ShadowVolume.h"


class Scene{

	int health = 100;
	int weapon = 0;
	int	ammo = 100;
	int armor = 100;



	HINSTANCE								hInst = NULL;
	HWND									wndHandle = NULL;

	// Strong reference to associated Direct3D device and rendering context.
	System									*system = nullptr;

	D3D11_VIEWPORT							viewport;
	CGDClock								*mainClock;
	LookAtCamera							*shadowMapCamera;
	FirstPersonCamera						*mainCamera;

	ID3D11RasterizerState *rsStateCullFront;
	ID3D11RasterizerState *rsStateCullBack;
	ID3D11RasterizerState *rsStateCullNone;
	
	ID3D11SamplerState *shadowSampler;

	CBufferScene* cBufferSceneCPU = nullptr;
	ID3D11Buffer *cBufferSceneGPU = nullptr;
	CBufferLight* cBufferLightCPU = nullptr;
	ID3D11Buffer *cBufferLightGPU = nullptr;
	CBufferShadow* cBufferShadowCPU = nullptr;
	ID3D11Buffer* cBufferShadowGPU = nullptr;

	ID3D11Texture2D*depthStencilBuffer = nullptr;
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ID3D11ShaderResourceView* shadowMapSRV;
	D3D11_VIEWPORT mViewport;
	ID3D11DepthStencilView*depthStencilView = nullptr;
	ID3D11DepthStencilView* mDepthMapDSV = nullptr;
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	


	// Add objects to the scene
	Triangle								*triangle = nullptr; //pointer to a Triangle the actual triangle is created in initialiseSceneResources
	Box										*box = nullptr; //pointer to a Box the actual Box is created in initialiseSceneResources
	Model									*chromeSphere = nullptr; //pointer to a Box the actual Box is created in initialiseSceneResources
	DoomHUD									*HUD = nullptr;
	Model									*bridge = nullptr;
	Quad									*quad = nullptr;

	
	Effect *reflectionMapEffect;
	Effect *shadowVolumeEffect;
	Effect *shadowGrassEffect;
	Effect *shadowReflectionMapEffect;
	Effect *shadowTreeEffect;

	Effect *grassEffect;
	Effect *treeEffect;
	Effect *basicColourEffect;
	Effect *basicTextureEffect;
	Effect *basicLightingEffect;
	Effect *perPixelLightingEffect;
	Effect *reflectionMappingEffect;
	// Tutorial - Shadow Volumes
	ShadowVolume							*shadowVolume = nullptr;
	ID3D11DepthStencilState   *renderShadowDepthStencilState = nullptr;


	static const int						numTreeTypes = 3;
	static const int						numTrees = 30;
	XMMATRIX treeArray[numTrees];
	int treeTypeArray[numTrees];
	Model									*trees[3];
	Grid									*grass = nullptr;
	Terrain									*ground = nullptr;
	float									grassLength = 0.01f;
	int										numGrassPasses = 20;


	// Private constructor
	Scene(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc);
	// Return TRUE if the window is in a minimised state, FALSE otherwise
	BOOL isMinimised();

public:
	// Public methods
	// Method to create the main Scene
	static Scene* CreateScene(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc);
	


	void DrawObjects(ID3D11DeviceContext *context);
	void randomerinoObjecterino(ID3D11DeviceContext *context);

	// Methods to handle initialisation, update and rendering of the scene
	HRESULT rebuildViewport();
	HRESULT initialiseSceneResources();
	HRESULT updateScene(ID3D11DeviceContext *context, Camera *camera);
	HRESULT renderScene();
	HRESULT initShadowMap();
	void DrawGrass(ID3D11DeviceContext *context);
	HRESULT shadowMappingViewport();

	// Clock handling methods
	void startClock();
	void stopClock();
	void resetClock();
	void reportTimingData();

	// Event handling methods
	// Process mouse move with the left button held down
	void handleMouseLDrag(const POINT &disp);
	// Process mouse wheel movement
	void handleMouseWheel(const short zDelta);
	// Process key down event.  keyCode indicates the key pressed while extKeyFlags indicates the extended key status at the time of the key down event (see http://msdn.microsoft.com/en-gb/library/windows/desktop/ms646280%28v=vs.85%29.aspx).
	void handleKeyDown(const WPARAM keyCode, const LPARAM extKeyFlags);
	// Process key up event.  keyCode indicates the key released while extKeyFlags indicates the extended key status at the time of the key up event (see http://msdn.microsoft.com/en-us/library/windows/desktop/ms646281%28v=vs.85%29.aspx).
	void handleKeyUp(const WPARAM keyCode, const LPARAM extKeyFlags);
	
	// Helper function to call updateScene followed by renderScene
	HRESULT updateAndRenderScene();

	// Destructor
	~Scene();
	// Decouple the encapsulated HWND and call DestoryWindow on the HWND
	void destoryWindow();
	// Resize swap chain buffers and update pipeline viewport configurations in response to a window resize event
	HRESULT resizeResources();
};
