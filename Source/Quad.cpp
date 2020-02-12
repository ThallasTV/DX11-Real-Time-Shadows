
#include <stdafx.h>
#include <Quad.h>
#include <Material.h>
#include <VertexStructures.h>
#include <iostream>
#include <exception>


using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;



HRESULT Quad::init(ID3D11Device *device) {

	try
	{
		if (numMaterials == 0)
		{
			numMaterials = 1;
			materials[0] = new Material();
		}
		Material *material= materials[0];

		// Setup Quad vertex buffer

		if (!device || !inputLayout)
			throw exception("Invalid parameters for triangle model instantiation");

		BasicVertexStruct vertices[] = {

			{ XMFLOAT3(-1.0f, 1.0f, 0.0f), material->getColour()->diffuse },
			{ XMFLOAT3(1.0f, 1.0f, 0.0f), material->getColour()->diffuse },
			{ XMFLOAT3(-1.0f, -1.0f, 0.0f), material->getColour()->diffuse },
			{ XMFLOAT3(1.0f, -1.0f, 0.0f), material->getColour()->diffuse },

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



		//// Setup index buffer
		//// Model indices for a single quad with no adjacent control points
		//DWORD quadIndices[] = {

		//	0, 1, 2, 3
		//};
		//D3D11_BUFFER_DESC indexDesc = {};
		//D3D11_SUBRESOURCE_DATA indexData = {};

		//indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		//indexDesc.ByteWidth = sizeof(DWORD) * 4; // 1 quad, 4 control per quad, no adjacency control points (only used first 4)
		//indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		//indexData.pSysMem = quadIndices;

		//device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);



	}
	catch (exception& e)
	{
		cout << "Quad object could not be instantiated due to:\n";
		cout << e.what() << endl;

		if (vertexBuffer)
			vertexBuffer->Release();

		if (inputLayout)
			inputLayout->Release();

		vertexBuffer = nullptr;
		inputLayout = nullptr;
	}
}


Quad::~Quad() {

	if (vertexBuffer)
		vertexBuffer->Release();

	if (inputLayout)
		inputLayout->Release();


}


void Quad::render(ID3D11DeviceContext *context) {


	// Validate object before rendering (see notes in constructor)
	if (!context || !vertexBuffer || !inputLayout)
		return;

	if (effect)
		// Sets shaders, states
		effect->bindPipeline(context);
	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	if (numTextures>0 && sampler) {

		context->PSSetShaderResources(0, numTextures, textures);
		context->PSSetSamplers(0, 1, &sampler);
	}

	// Set vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = { vertexBuffer };
	UINT vertexStrides[] = { sizeof(BasicVertexStruct) };
	UINT vertexOffsets[] = { 0 };

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);

	// Apply the cBuffer.
	context->VSSetConstantBuffers(0, 1, &cBufferModelGPU);
	context->PSSetConstantBuffers(0, 1, &cBufferModelGPU);

	// Set primitive topology for IA
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// Bind texture resource views and texture sampler objects to the PS stage of the pipeline

	// Draw triangle object
	// Note: Draw vertices in the buffer one after the other.  Not the most efficient approach (see duplication in the above vertex data)
	// This is shown here for demonstration purposes
	context->Draw(4, 0);
}
