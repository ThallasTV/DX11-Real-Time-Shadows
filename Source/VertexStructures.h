#pragma once
#include <d3d11_2.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

struct SkinnedVertexStruct {
	DirectX::XMFLOAT3					pos;
	DirectX::XMFLOAT3					normal;
	DirectX::XMFLOAT3					tangent;
	DirectX::PackedVector::XMCOLOR		matDiffuse;
	DirectX::PackedVector::XMCOLOR		matSpecular;
	DirectX::XMFLOAT2					texCoord;

	DirectX::XMFLOAT4					boneWeights;
	DirectX::XMINT4						boneIndices;

	DirectX::XMFLOAT4					boneWeights2;
	DirectX::XMINT4						boneIndices2;
	DirectX::XMFLOAT4					boneWeights3;
	DirectX::XMINT4						boneIndices3;
};

// Vertex input descriptor based on skinnedVertexStruct
static const D3D11_INPUT_ELEMENT_DESC skinVertexDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "DIFFUSE", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "SPECULAR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "INDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, 68, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "WEIGHTSX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 84, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "INDICESX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, 100, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "WEIGHTSY", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 116, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "INDICESY", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, 132, D3D11_INPUT_PER_VERTEX_DATA, 0 }


};
struct BasicClothVertex {

	DirectX::XMFLOAT3			pos;
	DirectX::XMFLOAT3			oldPos;
	DirectX::XMFLOAT3			normal;
	DirectX::PackedVector::XMCOLOR			matDiffuse;
	DirectX::PackedVector::XMCOLOR			matSpecular;
	DirectX::XMFLOAT2				texCoord;
	UINT						isFixed;
	//static HRESULT createInputLayout(ID3D11Device *device, DXBlob shaderBlob, UINT iaSlotIndex, ID3D11InputLayout **layout);

};

// Vertex description based on DXBasicClothVertex struct
static const D3D11_INPUT_ELEMENT_DESC basicClothVertexDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "POSITION_OLD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "DIFFUSE", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "SPECULAR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "FIXED", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

static const D3D11_SO_DECLARATION_ENTRY basicClothSODesc[] = {
	{ 0, "POSITION", 0, 0, 3, 0 },
	{ 0, "POSITION_OLD", 0, 0, 3, 0 },
	{ 0, "NORMAL", 0, 0, 3, 0 },
	{ 0, "DIFFUSE", 0, 0, 1, 0 },
	{ 0, "SPECULAR", 0, 0, 1, 0 },
	{ 0, "TEXCOORD", 0, 0, 2, 0 },
	{ 0, "FIXED", 0, 0, 1, 0 }
};

struct FlareVertexStruct {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 posL;
	DirectX::PackedVector::XMCOLOR		colour;
};



// Vertex input descriptor based on Flare VertexStruct
static const D3D11_INPUT_ELEMENT_DESC flareVertexDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "LPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};


struct BasicVertexStruct {
	DirectX::XMFLOAT3					pos;
	DirectX::PackedVector::XMCOLOR		colour;
};

// Add code here (Create an input element description)
// Vertex input descriptor based on BasicVertexStruct

static const D3D11_INPUT_ELEMENT_DESC basicVertexDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};


struct ExtendedVertexStruct {
	DirectX::XMFLOAT3					pos;
	DirectX::XMFLOAT3					normal;
	DirectX::XMFLOAT3					tangent;
	DirectX::PackedVector::XMCOLOR		matDiffuse;
	DirectX::PackedVector::XMCOLOR		matSpecular;
	DirectX::XMFLOAT2					texCoord;
};

// Vertex input descriptor based on ExtendedVertexStruct
static const D3D11_INPUT_ELEMENT_DESC extVertexDesc[] = {
	//{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "DIFFUSE", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "SPECULAR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }


	//{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "DIFFUSE", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "SPECULAR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "DIFFUSE", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "SPECULAR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

struct ParticleVertexStruct {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 posL;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT3 data;//;[age,?,?]
};

// Vertex input descriptor based on ParticleVertexStruct
static const D3D11_INPUT_ELEMENT_DESC particleVertexDesc[] = {
	// Add Code Here (setup particle vertex description)
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "LPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "DATA", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
static const D3D11_SO_DECLARATION_ENTRY particleSODesc[] = {
	// semantic name, semantic index, start component, component count, output slot
	{ 0, "POSITION", 0, 0, 3, 0 },   // output all components of position
	{ 0, "LPOS", 0, 0, 3, 0 },     // output the first 3 of the normal
	{ 0, "VELOCITY", 0, 0, 3, 0 },     // output the first 3 of the normal
	{ 0, "DATA", 0, 0, 3, 0 },     // output the first 2 texture coordinates
};
