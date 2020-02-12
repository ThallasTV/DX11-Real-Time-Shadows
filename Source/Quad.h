#pragma once
#include <BaseModel.h>



class Quad   : public BaseModel
{
protected:

public:
	Quad(ID3D11Device *device, Effect *_effect, Material *_materials[] = nullptr, int _numMaterials = 0, ID3D11ShaderResourceView **textures = nullptr, int numTextures = 0) : BaseModel(device, _effect, _materials, _numMaterials, textures, numTextures) { init(device); }

	//Quad(ID3D11Device *device,  ID3D11InputLayout	*_inputLayout);
	~Quad();
	HRESULT  init(ID3D11Device *device);
	void render(ID3D11DeviceContext *context);
};