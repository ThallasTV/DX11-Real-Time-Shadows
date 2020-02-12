
#pragma once

#include <d3d11_2.h>
#include <DirectXMath.h>
#include <string>
#include "VertexStructures.h"
#include "BaseModel.h"
class Effect;
class Camera;

class ShadowVolume : public BaseModel {

private:
	ID3D11Device			*device;	
	// Shadow  pipeline configuration
	Effect					*renderEffect = nullptr;
	ID3D11RasterizerState	*RSstateWire = nullptr; //Render wireframe volume for debugging 

	DWORD					numVerticesSdw,numFacesObj, dwNumVerticesObj,*edgesSdw = nullptr;
	uint32_t					*indicesObj = nullptr ;
	ID3D11InputLayout		*ShadowVertexLayout;
	ID3D11Buffer			*shadowBuffer;
	ID3D11Buffer			*stagingBuffer = nullptr;
	BasicVertexStruct		*verticesSdw = nullptr;
	BYTE					*verticesObj= nullptr;
	bool					objDoubleSided = false;

	void load(ID3D11Device *_device, ID3D11DeviceContext  *context,Effect *_effect, XMVECTOR _pointLightVec, ID3D11Buffer *vertexBuffer, DWORD _vBSizeBytes, ID3D11Buffer  *_pIndices, DWORD _dwNumFace);
	void initialiseShadowBuffers(ID3D11Device *device, ID3D11DeviceContext  *context, ID3D11Buffer *vertexBuffer, ID3D11Buffer  *_pIndices);
	HRESULT init(ID3D11Device *device) { return S_OK; };
	void addEdge(DWORD* pEdges, DWORD& dwNumEdges, DWORD v0, DWORD v1);

	DWORD stride;

public:
	ShadowVolume(ID3D11Device *_device, ID3D11DeviceContext  *context,
		Effect* _renderEffect,
		XMVECTOR _pointLightVec, ID3D11Buffer *vertexBuffer, DWORD _vBSizeBytes, ID3D11Buffer  *_pIndices, DWORD _dwNumFace,
		Material					*_materials[] = nullptr,
		int							_numMaterials = 0,
		ID3D11ShaderResourceView	**_textures = nullptr,
		int							_numTextures = 0)
		: BaseModel(_device, _renderEffect, _materials, _numMaterials, _textures, _numTextures)
	{
		load(_device,   context,_renderEffect,_pointLightVec,  vertexBuffer, _vBSizeBytes, _pIndices,_dwNumFace );
	}
	~ShadowVolume();

	void render(ID3D11DeviceContext *context);
	void renderWire(ID3D11DeviceContext *context);
	HRESULT buildShadowVolume(ID3D11DeviceContext  *context,  DirectX::XMVECTOR vLight);
	
};

