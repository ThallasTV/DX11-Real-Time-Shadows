
#include <stdafx.h>
#include <DoomHUD.h>
#include <VertexStructures.h>
#include <iostream>
#include <exception>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

double HUDWidth = 1.0;
double HUDHeight = 0.2;
double HUDX = 0.0;
double HUDY = -0.8;
int HUDTextWidth = 216;
int HUDTextHeight = 32;

HRESULT DoomHUD::init(ID3D11Device *device) {

	try
	{
		scale = XMMatrixIdentity();
		health = 100;
		ammo = 100;
		keys[0] = 0;
		keys[1] = 0;
		keys[2] = 0;
		weapon = 0;
		armor = 100;

		// Setup triangle vertex buffer
		BasicVertexStruct vertices[] = {
			{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f) },
		};
	
		// Setup vertex buffer
		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;
		ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));
		vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDesc.ByteWidth = sizeof(BasicVertexStruct) * 4;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexData.pSysMem = vertices;
		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);
		if (!SUCCEEDED(hr))
			throw exception("Vertex buffer cannot be created");

	}
	catch (exception& e)
	{
		cout << "Quad object could not be instantiated due to:\n";
		cout << e.what() << endl;
		if (vertexBuffer)
			vertexBuffer->Release();
		vertexBuffer = nullptr;
	}

	// If textures are used a sampler is required for the pixel shader to sample the texture
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = 64.0f;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	device->CreateSamplerState(&samplerDesc, &sampler);
	initCBufferHUD(device);
}

DoomHUD::~DoomHUD() {
	if (vertexBuffer)
		vertexBuffer->Release();
}

void DoomHUD::initCBufferHUD(ID3D11Device *device) {

	// Allocate 16 byte aligned block of memory for "main memory" copy of cBufferBasic
	cBufferHUDCPU = (CBufferHUD*)_aligned_malloc(sizeof(CBufferHUD), 16);
	// Fill out cBufferModelCPU
	cBufferHUDCPU->value = 0;
	cBufferHUDCPU->numIcons = 1;
	// Create GPU resource memory copy of cBufferBasic
	// fill out description (Note if we want to update the CBuffer we need  D3D11_CPU_ACCESS_WRITE)
	D3D11_BUFFER_DESC cbufferDesc;
	D3D11_SUBRESOURCE_DATA cbufferInitData;
	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&cbufferInitData, sizeof(D3D11_SUBRESOURCE_DATA));
	cbufferDesc.ByteWidth = sizeof(CBufferHUD);
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferInitData.pSysMem = cBufferHUDCPU;// Initialise GPU CBuffer with data from CPU CBuffer
	HRESULT hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData,
		&cBufferHUDGPU);
}
void DoomHUD::updateHUD(ID3D11DeviceContext *context) {
	mapCbuffer(context, cBufferHUDCPU, cBufferHUDGPU, sizeof(CBufferHUD));
	context->PSSetConstantBuffers(4, 1, &cBufferHUDGPU);
}

void DoomHUD::render(ID3D11DeviceContext *context) {

	// Validate object before rendering 
	if (!context || !vertexBuffer || !inputLayout)
		return;

	if (effect)
		// Sets shaders, states
		effect->bindPipeline(context);

	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	// Set vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = { vertexBuffer };
	UINT vertexStrides[] = { sizeof(BasicVertexStruct) };
	UINT vertexOffsets[] = { 0 };

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Bind samplers to the PS stage of the pipeline
	if (sampler) 
		context->PSSetSamplers(0, 1, &sampler);

	//Render each part of the HUD separately

	//Background
	drawPartPixels(context, BACKGRND, 0, 1, HUDTextWidth,HUDTextHeight, 0, 0);

	//Face 
	drawPartPixels(context, FACE, floor((100 - health) / 20)*NUM_ANIMS + anim, NUM_FACES, 30, 32, 100, 0);

	//To do -  render keys, weapon, health(numbers), ammo(numbers), armor(numbers)

	//Keys
	//Key1
	if (keys[0] > 0)
		drawPartPixels(context, KEY, 0, NUM_KEYS, 11, 10, 131, 1);
	//Key2
	if (keys[1] > 0)
		drawPartPixels(context, KEY, 1, NUM_KEYS, 11, 10, 143, 1);
	//Key3
	if (keys[2] > 0)
		drawPartPixels(context, KEY, 2, NUM_KEYS, 11, 10, 155, 1);
	
	//Weapons
	drawPartPixels(context, WEAPON, weapon, NUM_WEAPONS, 38, 20, 129, 12);
	
	// Ammo
	int digit = ammo / 100;
	drawPartPixels(context, NUMBER, digit, 10, 14, 16, 4, 4);
	digit = (ammo - digit * 100) / 10;
	drawPartPixels(context, NUMBER, digit, 10, 14, 16, 4 + 14, 4);
	digit = (ammo - digit * 10);
	drawPartPixels(context, NUMBER, digit, 10, 14, 16, 4 + 14 + 14 , 4);
	
	// Health
	digit = health / 100;
	drawPartPixels(context, NUMBER, digit, 10, 14, 16, 55, 4);
	digit = (health - digit * 100)/10;
	drawPartPixels(context, NUMBER, digit, 10, 14, 16, 55 + 14, 4);
	digit = (health - digit * 10);
	drawPartPixels(context, NUMBER, digit, 10, 14, 16, 55 + 14 + 14, 4);
	
	// Armor
	digit = armor / 100;
	drawPartPixels(context, NUMBER, digit, 10, 14, 16, 172, 4);
	digit = (armor - digit * 100) / 10;
	drawPartPixels(context, NUMBER, digit, 10, 14, 16, 172 + 14, 4);
	digit = (armor - digit * 10);
	drawPartPixels(context, NUMBER, digit, 10, 14, 16, 172 + 14 + 14, 4);
}

void DoomHUD::drawPartPixels(ID3D11DeviceContext *context, int part, int value,int numIcons, int Width, int Height, int X, int Y){
	if (numTextures > part)
		context->PSSetShaderResources(0, 1, &textures[part]);
	setScalePixels(Width, Height);
	setPosPixels(X, Y);
	update(context);
	cBufferHUDCPU->value = value;
	cBufferHUDCPU->numIcons = numIcons;
	updateHUD(context);
	context->Draw(4, 0);
}

void DoomHUD::setScalePixels(int Width, int Height){
	setScaleRelative((double)Width/ (double)HUDTextWidth * HUDWidth, (double)Height/ (double)HUDTextHeight* HUDHeight);
}
void DoomHUD::setScaleRelative(double Width, double Height){
	scale = XMMatrixTranslation(1.0, -1.0, 0.0)*XMMatrixScaling(Width, Height, 1.0)*XMMatrixTranslation(-1.0, 1.0 + (2.0 * HUDY), 0.0);
	setWorldMatrix(scale);
}
void DoomHUD::setPosPixels(int X, int Y){
	setPosRelative((double)X / (double)HUDTextWidth, (double)Y / (double)HUDTextHeight);
}
void DoomHUD::setPosRelative(double X, double Y){
	setWorldMatrix(scale*XMMatrixTranslation(X*2.0*HUDWidth, -Y*2.0* HUDHeight, 0.0));
}
