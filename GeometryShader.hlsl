cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 UV : TEXCOORD0;
	float3 norm : NORMAL;
	float4 wPos : POSITION;
};

struct GS_IN
{
	float4 pos : SV_POSITION;
	float2 UV : TEXCOORD0;
};

[maxvertexcount(6)]
void main(
	triangle GS_IN input[3], 
	inout TriangleStream< GSOutput > output)
{
	GSOutput element[3];
	float4 duplicate[3];
	float4 camPos = float4(0, 0, -2, 1);


	for (uint i = 0; i < 3; i++)
	{
		element[i].UV = input[i].UV;

		duplicate[i].x = input[i].pos.x + 1.0f;
		duplicate[i].y = input[i].pos.y;
		duplicate[i].z = input[i].pos.z;// -1.0f;
		duplicate[i].w = input[i].pos.w;
		element[i].pos = mul(input[i].pos, world);

		
	}

	for (int i = 0; i < 3; i++)
	{
		element[i].norm = normalize(cross(element[2].pos - element[1].pos, element[0].pos - element[1].pos));
		element[i].wPos = element[i].pos;
	}
	for (int i = 0; i < 3; i++)
	{

		element[i].pos = mul(element[i].pos, view);
		element[i].pos = mul(element[i].pos, proj);

		output.Append(element[i]);
	}
	output.RestartStrip();

	//second quad

	GSOutput element2[3];

	for (uint i = 0; i < 3; i++)
	{

		element2[i].UV = input[i].UV;

		element2[i].pos = mul(duplicate[i], world);
	}

	for (int i = 0; i < 3; i++)
	{
		element2[i].norm = normalize(cross(element2[2].pos - element2[1].pos, element2[0].pos - element2[1].pos));
		element2[i].wPos = element2[i].pos;
	}

	for (int i = 0; i < 3; i++)
	{
		element2[i].pos = mul(element2[i].pos, view);
		element2[i].pos = mul(element2[i].pos, proj);
		output.Append(element2[i]);
	}
}