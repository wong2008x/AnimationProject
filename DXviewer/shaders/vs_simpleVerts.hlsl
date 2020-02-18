#include "mvp.hlsli"
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
    output.world_pos = mul(float4(input.pos.xyz, 1.0f), modeling);
    output.pos = mul(output.world_pos, view);
    output.pos = mul(output.pos, projection);
    output.eye_pos.xyz = view[3].xyz;
    output.eye_pos.w = 1.0f;
    output.normal = mul(float4(input.normal.xyz, 0.0f), modeling).xyz;
    output.uv = input.uv.xy;
    return output;
}