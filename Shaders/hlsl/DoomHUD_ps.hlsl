
//  HUD Part pixel shader

cbuffer HUDCBuffer : register(b4) {
	int Value;
	int NumIcons;
};

// input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct fragmentInputPacket {
	float2				texCoord	: TEXCOORD;
	float4				posH			: SV_POSITION;
};

struct fragmentOutputPacket {
	float4				fragmentColour : SV_TARGET;
};

Texture2D inputTex : register(t0);
Texture2D inputTex2 : register(t1);
SamplerState linearSampler : register(s0);

fragmentOutputPacket main(fragmentInputPacket inputFragment) {

	fragmentOutputPacket outputFragment;

	float3 transparent = float3(1.0, 0.0, 1.0);//transparent colour is pink
	float2 texCoord = inputFragment.texCoord;

	//Divide textCoord.x by number of icons in the texture	
	texCoord.x /= (double)NumIcons;
	//Select the appropriate icon given by value
	texCoord.x += Value * 1.0 / (double)NumIcons;
		
	float3 colour=inputTex.Sample(linearSampler, texCoord);
		
	//Clip if sampled colour is transparent
	if (all(colour== transparent))
		clip(-1);

	outputFragment.fragmentColour = float4(colour*0.95+0.05,1.0);//bump up brightness
	return outputFragment;
}
