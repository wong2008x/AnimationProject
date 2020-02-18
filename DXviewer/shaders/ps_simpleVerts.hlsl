Texture2D tx_diffuse : register(t0);
Texture2D tx_emissive : register(t1);
Texture2D tx_specular : register(t2);
SamplerState samLinear : register(s0);

cbuffer lightbuffer : register(b0)
{
    float4 dLightDir;
    float4 dLightClr;
}

struct VSOut
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 world_pos : POSITIONT;
    float4 eye_pos : EYEPOS;
};
//struct PS_OUTPUT
//{
//    float4 color : SV_TARGET;
//};
static const float4 ambient_light = { 0.25f, 0.25f, 0.25f, 0.0f };

float4 main(VSOut input) : SV_TARGET
{
    //PS_OUTPUT output;
    float4 specClr = {1,1,1,64 };
    input.normal = normalize(input.normal);
    
   //loat3 light_dir = dLightPos.xyz - input.world_pos.xyz;
    float light_intensity = saturate(dot(input.normal, -dLightDir.xyz));
    float3 toCam = normalize(input.eye_pos.xyz - input.world_pos.xyz);
    
    float3 reflection = reflect(dLightDir.xyz, input.normal);
    float specDot = saturate(dot(reflection, toCam));
    specDot = pow(specDot, specClr.w);
    
    
    float4 mat_diffuse = tx_diffuse.Sample(samLinear, input.uv); // *float4(surface_diffuse, 0.0f) * surface_diffuse_factor;
    float4 mat_specular = tx_specular.Sample(samLinear, input.uv); // *float4(surface_specular, 0.0f) * surface_specular_factor;
    float4 mat_emissive = tx_emissive.Sample(samLinear, input.uv); // *float4(surface_emissive, 0.0f) * surface_emissive_factor;
    float4 emissive = mat_emissive;
    float4 ambient = mat_diffuse * ambient_light;
    float4 specular = mat_specular * specDot * specClr;
    float4 diffuse = mat_diffuse * light_intensity * dLightClr;
    // hacky conservation of energy
    diffuse.xyz -= specular.xyz;
    diffuse.xyz = saturate(diffuse.xyz);
    float4 color = saturate(ambient + specular + diffuse + emissive);
        
    //output.color = color;
    return color;
}