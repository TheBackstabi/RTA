texture2D baseTexture : register(t0);

SamplerState filter : register(s0) = NULL;

float4 main(float2 baseUV : TEXCOORD0) : SV_TARGET
{
	float4 temp = baseTexture.Sample(filter, baseUV);

	return temp;
}