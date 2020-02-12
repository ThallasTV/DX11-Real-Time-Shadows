
//
// Reflection Mapping Pixel Shader
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


//-----------------------------------------------------------------
// Globals
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// Globals
//-----------------------------------------------------------------
struct Light {
	float4				Vec; // w=1: Vec represents position, w=0: Vec  represents direction.
	float4				Ambient;
	float4				Diffuse;
	float4				Specular;
	float4				Attenuation;// x=constant,y=linear,z=quadratic,w=cutOff
};
cbuffer modelCBuffer : register(b0) {

	float4x4			worldMatrix;
	float4x4			worldITMatrix; // Correctly transform normals to world space
};
cbuffer cameraCbuffer : register(b1) {
	float4x4			viewMatrix;
	float4x4			projMatrix;
	float4				eyePos;
}
cbuffer lightCBuffer : register(b2) {
	Light light[2];
};

//
// Textures
//

// Assumes texture bound to texture t0 and sampler bound to sampler s0
TextureCube envMap : register(t0);
Texture2D diffMap : register(t1); 
Texture2D specMap : register(t2);

SamplerState linearSampler : register(s0);


//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------

// Input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct FragmentInputPacket {

	// Vertex in world coords
	float3				posW			: POSITION;
	// Normal in world coords
	float3				normalW			: NORMAL;
	float3				tangentW		: TANGENT;
	float4				matDiffuse		: DIFFUSE; // a represents alpha.
	float4				matSpecular		: SPECULAR; // a represents specular power. 
	float2				texCoord		: TEXCOORD;
	float4				posH			: SV_POSITION;
};


struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};


//-----------------------------------------------------------------
// Pixel Shader - Lighting 
//-----------------------------------------------------------------

FragmentOutputPacket main(FragmentInputPacket v) {

	FragmentOutputPacket outputFragment;
	bool useTexture = false;
	float FresnelBias = 0.1;//0.3;
	float FresnelExp = 0.5;//4;
	bool useSpecMap = false;

	float4 baseColour = v.matDiffuse;
	
	if (useTexture)
		baseColour = baseColour * diffMap.Sample(linearSampler, v.texCoord);


	float3 N = normalize(v.normalW);
	float3 Nn = normalize(N);


	float3 colour = 0;
	for (int i = 0; i < 2; i++)
	{


		float att = 1.0;

		// Calculate the lambertian term (essentially the brightness of the surface point based on the dot product of the normal vector with the vector pointing from v to the light source's location)
		float3 lightDir = -light[i].Vec.xyz; // Directional light
		if (light[i].Vec.w == 1.0)// Positional light
		{

			lightDir = light[i].Vec.xyz - v.posW;
			float dist = length(lightDir);
			float cutOff = light[i].Attenuation.w;
			if (dist > cutOff)
				continue;
			else
			{
				float con = light[i].Attenuation.x; float lin = light[i].Attenuation.y; float quad = light[i].Attenuation.z;
				att = 1.0 / (con + lin * dist + quad * dist*dist);
			}

		}

		//Initialise returned colour to ambient component
		colour += baseColour.xyz* light[i].Ambient*v.matDiffuse*att;;

		lightDir = normalize(lightDir);
		// Add diffuse light if relevant (otherwise we end up just returning the ambient light colour)
		// Add Code Here (Add diffuse light calculation)
		colour +=  (max(dot(lightDir, N), 0.0f) *baseColour.xyz * light[i].Diffuse);

		 //Calc specular light
		float specPower = max(v.matSpecular.a*1000.0, 1.0f);
		float3 eyeDir = normalize(eyePos - v.posW);
		float3 R = reflect(-lightDir, Nn);
		float specFactor = pow(max(dot(R, eyeDir), 0.0f), specPower)*v.matSpecular.a;
		colour += att * specFactor * v.matSpecular.xyz * light[i].Specular.xyz;

	
	}


	// Calc specular light

		// Add Code Here (Specular Factor calculation)	
//	float specFactor = v.matSpecular.a;


	float3 eyeDir = normalize(eyePos - v.posW);
	float3 ER = reflect(-eyeDir, N);

	float specFactor = v.matSpecular.a;
	
	if (useSpecMap > 0.0)
		specFactor *= specMap.Sample(linearSampler, v.texCoord).r;



	float3 specColour =  envMap.Sample(linearSampler, ER).rgb *v.matSpecular.rgb*specFactor;
	//float3 specColour =  envMap.Sample(linearSampler, ER).rgb*v.matSpecular.rgb;

	//colour += att * specFactor * specColour * light[i].Specular.xyz;
	//colour = specColour;


	//float3 specColour = specFactor * envMap.Sample(linearSampler, ER).rgb* v.matSpecular.rgb;

	// Calculate Fresnel term
	float facing = 1 - max(dot(N, eyeDir), 0);
	float fres = specFactor*(FresnelBias + (1.0 - FresnelBias)*pow(abs(facing), abs(FresnelExp)));

	colour = (colour*(1 - fres)) + (fres * specColour);
	//colour += specColour;
	outputFragment.fragmentColour = float4(colour, baseColour.a);

	return outputFragment;
}
