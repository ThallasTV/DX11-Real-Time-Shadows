#pragma once
#include <Effect.h>
#include <Material.h>
#include <BaseModel.h>
#define NUM_KEYS 6
#define NUM_ANIMS 3
#define NUM_FACES 16
#define NUM_WEAPONS 8

enum {BACKGRND,FACE,KEY,WEAPON,NUMBER };//HUD part being rendered

__declspec(align(16)) struct CBufferHUD {
	int value;
	int numIcons;
};

class DoomHUD :public  BaseModel
{
	XMMATRIX scale;
	int anim;
	int health;
	bool keys[3];
	int weapon;
	int ammo;
	int armor;
	CBufferHUD				*cBufferHUDCPU = nullptr;
	ID3D11Buffer			*cBufferHUDGPU = nullptr;

public:
	DoomHUD(ID3D11Device *device, Effect *_effect, Material *_materials[] = nullptr, int _numMaterials = 0, ID3D11ShaderResourceView **textures = nullptr, int numTextures = 0) : BaseModel(device, _effect, _materials, _numMaterials, textures, numTextures) { init(device); }
	DoomHUD(ID3D11Device *device, ID3D11InputLayout *_inputLayout, Material *_materials[], int _numMaterials = 0, ID3D11ShaderResourceView **textures = nullptr, int numTextures = 0) : BaseModel(device, _inputLayout, _materials, _numMaterials, textures, numTextures) { init(device); }
	~DoomHUD();
	HRESULT init(ID3D11Device *device);
	void initCBufferHUD(ID3D11Device *device);
	void updateHUD(ID3D11DeviceContext *context);
	void render(ID3D11DeviceContext *context);
	void setHealth(int _health) { health = _health; };
	void setAnim(int _anim) { anim = _anim; };
	void setKey(int key, int _found) { keys[key] = _found; };
	void setWeapon(int _weapon) { weapon = _weapon; };
	void setAmmo(int _ammo) { ammo = _ammo; };
	void setArmor(int _armor) { armor = _armor; };
	void drawPartPixels(ID3D11DeviceContext *context, int part, int value,int numIcons, int Width, int Height, int X, int Y);
	void setScaleRelative(double Width, double Height);
	void setScalePixels(int Width, int Height);
	void setPosRelative(double X, double Y);
	void setPosPixels(int X, int Y);
};