#pragma pack_matrix(row_major)
cbuffer MVP_t : register(b0)
{
    matrix modeling;
    matrix view;
    matrix projection;
};

cbuffer cb: register(b1)
{
    matrix m[28];
};

struct VSIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 weights : BLENDWEIGHTS;
    int4 indices : BLENDINDICES;
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
    
    float4 skinned_pos = float4(0, 0, 0, 0);
    float4 skinned_norm = float4(0, 0, 0, 0);

     [unroll]
    for (int i = 0; i < 4; i++)
    {
        skinned_pos += mul(float4(input.pos.xyz, 1.0f), m[input.indices[i]]) * input.weights[i];
        skinned_norm += mul(float4(input.normal.xyz, 0.0f), m[input.indices[i]]) * input.weights[i];
    }
    
    output.world_pos = mul(modeling, skinned_pos);
    output.pos = mul(view, output.world_pos);
    output.pos = mul(projection, output.pos);
    output.eye_pos.xyz = view[3].xyz;
    output.eye_pos.w = 1.0f;
    output.normal = mul(modeling, skinned_norm).xyz;
    output.uv = input.uv;
    return output;
}