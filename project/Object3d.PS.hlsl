#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
};


struct DirectionalLight
{
    float4 color; //ライトの色
    float3 direction; //ライトの向き
    float intensity; //輝度
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixcelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixcelShaderOutput main(VertexShaderOutput input)
{
    PixcelShaderOutput output;
    float4 transformedUV = mul(float4(input.texcoord,0.0f,1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    //float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    //output.color = gMaterial.color * textureColor;
    
    if (gMaterial.enableLighting != 0)//Lightingする場合
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL*0.5f+0.5f,2.0f);
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;

    }
    else //Lightingしない場合
    {
        output.color = gMaterial.color * textureColor;
    }
    
    
    
    return output;
};

