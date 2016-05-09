struct V_IN
{
	float3 posL : POSITION;
	float3 uvs : TEXCOORD;
	float3 normals : NORMAL;
};
struct V_OUT
{
	float2 uvs : TEXCOORD0;
	float3 normals : TEXCOORD1;
	float4 posH : SV_POSITION;
};
cbuffer OBJECT : register(b0)
{
	float4x4 worldMatrix;
}
cbuffer SCENE : register(b1)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}
V_OUT main(V_IN input)
{
	V_OUT output = (V_OUT)0;
	float4 localH = float4(input.posL, 1);
	localH = mul(worldMatrix, localH);
	localH = mul(viewMatrix, localH);

	localH = mul(projectionMatrix, localH);

	output.uvs = input.uvs.xy;
	output.normals = input.normals;
	output.posH = localH;

	return output; // send projected vertex to the rasterizer stage
}
