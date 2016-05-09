texture2D baseTexture : register(t0);

SamplerState filter : register(s0) = NULL;

float4 main(float2 baseUV : TEXCOORD0, float4 worldPos : WORLD_POS, float3 baseNorm : NORM) : SV_TARGET
{
	float4 PlocalLight = float4(0, 1, 5, 1);
	float4 PlightDir = normalize(PlocalLight - worldPos);
	float PlightRatio = saturate(dot(PlightDir, baseNorm));
	float4 PlightColor = float4(1, 1, 1, 1);
	float4 temp = baseTexture.Sample(filter, baseUV);

	return (temp * PlightRatio * PlightColor);
}