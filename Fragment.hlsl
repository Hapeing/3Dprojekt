Texture2D tex : register(t0);
SamplerState sampleType;

struct GS_OUT
{
	float4 pos : SV_POSITION;
	float2 UV : TEXCOORD0;
	float3 norm : NORMAL;
	float4 wPos : POSITION;

};

float4 PS_main(GS_OUT input) : SV_Target
{
	float4 result = tex.Sample(sampleType, input.UV).rgba;
	result = result * dot(float4(input.norm, 0), normalize(float4(0, 0, -2, 1) - input.wPos));
	//result = float4(input.norm,1);

	return float4(result);
};