
// Basic colour geometry shader
struct GeometryInputPackets // output of vertex shader – input to geometry shader
{
	float4				colour		: COLOR;
	float4				posH		: SV_POSITION;
};

struct GeometryOutputPacket // output of geometry shader – input to pixel shader
{
	float4 colour : COLOR;
	float4 posH : SV_POSITION;
};

[maxvertexcount(3)]
void main(triangle GeometryInputPackets  inputVertex[3],inout TriangleStream <GeometryOutputPacket> tStream)
{
	GeometryOutputPacket		outputVertex;
	outputVertex.colour = inputVertex[0].colour;
	outputVertex.posH = inputVertex[0].posH;
	tStream.Append(outputVertex);
	outputVertex.colour = inputVertex[1].colour;
	outputVertex.posH = inputVertex[1].posH;
	tStream.Append(outputVertex);
	outputVertex.colour = inputVertex[2].colour;
	outputVertex.posH = inputVertex[2].posH;
	tStream.Append(outputVertex);

}
