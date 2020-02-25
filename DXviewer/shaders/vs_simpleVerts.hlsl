

cbuffer MVP_t : register(b0)
{
    matrix modeling;
    matrix view;
    matrix projection;
};
struct VSIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tang : TANGENT;
};
struct VSOut
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 world_pos : POSITIONT;
    float4 eye_pos : EYEPOS;
};
VSOut main(VSIn input)
{
    VSOut output;
    output.world_pos = mul(modeling, float4(input.pos, 1.0f));
    output.pos = mul(view, output.world_pos);
    output.pos = mul(projection, output.pos);
    output.eye_pos.xyz = view[3].xyz;
    output.eye_pos.w = 1.0f;
    output.normal = mul(float4(input.normal.xyz, 0.0f), modeling).xyz;
    output.uv = input.uv.xy;
    return output;
}