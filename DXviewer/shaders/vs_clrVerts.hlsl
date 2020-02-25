
cbuffer MVP_t : register(b0)
{
    matrix modeling;
    matrix view;
    matrix projection;
};

struct VSIn
{
    float3 pos : POSITION;
    float4 color : COLOR;
};
struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};
VSOut main(VSIn input)
{
    VSOut output;
    output.pos = mul(float4(input.pos, 1), modeling);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.color = input.color;
	return output;
}