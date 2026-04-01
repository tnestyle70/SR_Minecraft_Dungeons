// PostProcess.fx
// DX9 HLSL Post-Process Effect
// 화면 노이즈 / 왜곡 / 보라 공허 이펙트

// ──────────────────────────────────────────
// Textures
// ──────────────────────────────────────────
texture SceneMap;
texture NoiseMap;

sampler2D SceneTex = sampler_state
{
    Texture = <SceneMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

sampler2D NoiseTex = sampler_state
{
    Texture = <NoiseMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;
    AddressU = WRAP;
    AddressV = WRAP;
};

// ──────────────────────────────────────────
// Parameters
// ──────────────────────────────────────────
float  fTime;          // 누적 시간 (초)
float  fNoiseAmt;      // 그레인 노이즈 강도   (0~1)
float  fDistortAmt;    // UV 왜곡 강도         (0~1)
float  fShakeX;        // 화면 흔들림 X 오프셋 (UV 공간)
float  fShakeY;        // 화면 흔들림 Y 오프셋 (UV 공간)
float4 vVoidTint;      // xyz = 보라 컬러, w = 블렌드 강도

// ── Noise grain (TechNoise) ──
float  gNoiseAmt;      // grain intensity (0~1)
float  gNoisePhase;    // per-frame phase offset

// ── Glass break (TechGlassBreak) ──
float  gGlassAmt;      // glass distortion strength (0~1)
float  gGridSize;      // cell count per axis (8~16)

// ── Wave distortion (TechWave) ──
float  gWaveAmpX;      // X amplitude (UV fraction)
float  gWaveAmpY;      // Y amplitude
float  gWaveFreqX;     // X frequency
float  gWaveFreqY;     // Y frequency
float  gWaveSpeedX;    // X scroll speed
float  gWaveSpeedY;    // Y scroll speed

// ──────────────────────────────────────────
// Vertex Shader  (pass-through, NDC 좌표)
// ──────────────────────────────────────────
struct VS_IN
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

VS_OUT VS_Screen(VS_IN i)
{
    VS_OUT o;
    o.pos = i.pos;   // 이미 클립 공간 NDC
    o.uv  = i.uv;
    return o;
}

// ──────────────────────────────────────────
// Pixel Shader
// ──────────────────────────────────────────
float4 PS_PostProcess(float2 uv : TEXCOORD0) : COLOR0
{
    // 1. 화면 흔들림 (쉐이킹)
    uv.x += fShakeX;
    uv.y += fShakeY;

    // 2. UV 왜곡 (꼬리 공격 / 브레스 왜곡)
    if (fDistortAmt > 0.001f)
    {
        float2 noiseUV = uv * 4.0f + float2(fTime * 0.4f, -fTime * 0.25f);
        float2 distort = (tex2D(NoiseTex, noiseUV).xy * 2.0f - 1.0f);
        uv += distort * fDistortAmt * 0.04f;
    }

    // UV 클램프 (화면 밖 샘플링 방지)
    uv = saturate(uv);

    // 3. 씬 샘플링
    float4 scene = tex2D(SceneTex, uv);

    // 4. 그레인 노이즈 오버레이 (피격 충격)
    if (fNoiseAmt > 0.001f)
    {
        float2 grainUV = uv * 8.0f + float2(fTime * 23.7f, fTime * 17.3f);
        float  grain   = tex2D(NoiseTex, grainUV).r * 2.0f - 1.0f;
        scene.rgb += grain * fNoiseAmt * 0.25f;
    }

    // 5. 보라 공허 이펙트 + 비네팅 (브레스 상태)
    if (vVoidTint.w > 0.001f)
    {
        float2 c        = uv - 0.5f;
        float  vignette = saturate(1.0f - dot(c, c) * 2.8f);
        scene.rgb = lerp(scene.rgb,
                         scene.rgb * vignette + vVoidTint.xyz * 0.4f,
                         vVoidTint.w);
    }

    return saturate(scene);
}

// ──────────────────────────────────────────
// Technique
// ──────────────────────────────────────────
technique PostProcess
{
    pass P0
    {
        VertexShader    = compile vs_2_0 VS_Screen();
        PixelShader     = compile ps_2_0 PS_PostProcess();

        ZEnable         = FALSE;
        ZWriteEnable    = FALSE;
        AlphaBlendEnable= FALSE;
        CullMode        = NONE;
    }
}

// ──────────────────────────────────────────
// TechNoise — Film grain overlay
// ──────────────────────────────────────────
float4 PS_Noise(float2 uv : TEXCOORD0) : COLOR0
{
    float3 col = tex2D(SceneTex, uv).rgb;

    // Per-frame varying UV offset via gNoisePhase
    float2 noiseUV = uv * 8.0f + float2(
        sin(gNoisePhase * 1.7f) * 0.01f + fTime * 23.7f,
        cos(gNoisePhase * 2.3f) * 0.01f + fTime * 17.3f
    );
    float grain = tex2D(NoiseTex, noiseUV).r;
    grain = (grain - 0.5f) * 2.0f * gNoiseAmt;
    col += grain * 0.25f;

    return float4(saturate(col), 1.0f);
}

technique TechNoise
{
    pass P0
    {
        VertexShader     = compile vs_2_0 VS_Screen();
        PixelShader      = compile ps_2_0 PS_Noise();
        ZEnable          = FALSE;
        ZWriteEnable     = FALSE;
        AlphaBlendEnable = FALSE;
        CullMode         = NONE;
    }
}

// ──────────────────────────────────────────
// TechGlassBreak — Shattered-glass UV distortion
// ──────────────────────────────────────────
float4 PS_GlassBreak(float2 uv : TEXCOORD0) : COLOR0
{
    // Snap UV to grid cell center
    float2 cell     = floor(uv * gGridSize) / gGridSize;
    // Per-cell random offset from noise texture
    float2 noiseOff = tex2D(NoiseTex, cell * 3.7f + fTime * 0.05f).rg;
    noiseOff        = (noiseOff - 0.5f) * 2.0f * gGlassAmt;
    float2 distUV   = uv + noiseOff * 0.04f;
    distUV          = saturate(distUV);

    return tex2D(SceneTex, distUV);
}

technique TechGlassBreak
{
    pass P0
    {
        VertexShader     = compile vs_2_0 VS_Screen();
        PixelShader      = compile ps_2_0 PS_GlassBreak();
        ZEnable          = FALSE;
        ZWriteEnable     = FALSE;
        AlphaBlendEnable = FALSE;
        CullMode         = NONE;
    }
}

// ──────────────────────────────────────────
// TechWave — Sinusoidal wave distortion
// ──────────────────────────────────────────
float4 PS_Wave(float2 uv : TEXCOORD0) : COLOR0
{
    float2 wuv = uv;
    wuv.x += sin(uv.y * gWaveFreqY + fTime * gWaveSpeedX) * gWaveAmpX;
    wuv.y += cos(uv.x * gWaveFreqX + fTime * gWaveSpeedY) * gWaveAmpY;
    wuv = saturate(wuv);

    return tex2D(SceneTex, wuv);
}

technique TechWave
{
    pass P0
    {
        VertexShader     = compile vs_2_0 VS_Screen();
        PixelShader      = compile ps_2_0 PS_Wave();
        ZEnable          = FALSE;
        ZWriteEnable     = FALSE;
        AlphaBlendEnable = FALSE;
        CullMode         = NONE;
    }
}

// ──────────────────────────────────────────
// VoidFlame — 빌보드 구체 셰이더
// Fresnel + fBm 3옥타브 + Screen Space 굴절 + HDR (ps_3_0)
// ──────────────────────────────────────────
float4x4 matWVP;           // 빌보드 WorldViewProj
float4x4 matWV;            // 빌보드 WorldView (eyeVec Fresnel용)
float    fFlameAmt;        // 0~1 활성화 강도
float    fRefractionIndex; // 배경 굴절 강도 (0.025 권장)
float    iScreenW;         // 화면 너비 (픽셀)
float    iScreenH;         // 화면 높이 (픽셀)

struct VF_IN  { float4 pos : POSITION; float2 uv : TEXCOORD0; };
struct VF_OUT
{
    float4 pos    : POSITION;
    float2 uv     : TEXCOORD0;
    float3 eyeVec : TEXCOORD1;   // 뷰 공간 픽셀 위치 (Fresnel용)
};

VF_OUT VS_Billboard(VF_IN i)
{
    VF_OUT o;
    o.pos    = mul(i.pos, matWVP);
    o.uv     = i.uv;
    // 뷰 공간 위치 계산 (matWV = World * View, Proj 제외)
    float4 viewPos = mul(i.pos, matWV);
    o.eyeVec = normalize(viewPos.xyz);
    return o;
}

// ── fBm 3 옥타브 ──────────────────────────
float fBm3(float2 uv, float t)
{
    float n0 = tex2D(NoiseTex, uv * 1.0f + t * float2( 0.50f, -0.30f)).r;
    float n1 = tex2D(NoiseTex, uv * 2.0f + t * float2(-0.80f,  0.50f)).r;
    float n2 = tex2D(NoiseTex, uv * 4.0f + t * float2( 1.20f, -0.90f)).r;
    return n0 * 0.500f + n1 * 0.250f + n2 * 0.125f;
}

// ── PS 입력 구조체 (VPOS 단독 선언 에러 방지) ──
struct PS_IN
{
    float2 uv     : TEXCOORD0;
    float3 eyeVec : TEXCOORD1;
    float2 vpos   : VPOS;        // 화면 픽셀 좌표 (ps_3_0 전용)
};

float4 PS_VoidFlame(PS_IN i) : COLOR0
{
    // ── 1. 방사형 거리 + 구체 법선 ──────────────
    float2 c    = i.uv * 2.0f - 1.0f;
    float  dist = length(c);
    clip(0.99f - dist);

    float3 N = normalize(float3(c.x, -c.y, sqrt(max(0.001f, 1.0f - dot(c, c)))));
    float3 V = normalize(i.eyeVec);

    // ── 2. Fresnel (외곽→1, 중심→0) ─────────────
    float fresnel = pow(1.0f - saturate(dot(N, V)), 4.5f);

    // ── 3. fBm 3옥타브 이글거림 ─────────────────
    float fbmVal = fBm3(i.uv, fTime);

    // ── 4. Screen Space 굴절 ─────────────────────
    float2 screenUV = i.vpos.xy / float2(iScreenW, iScreenH);
    float2 deltaUV  = (fbmVal - 0.4375f) * fRefractionIndex * fresnel;
    float4 bgColor  = tex2D(SceneTex, screenUV + deltaUV);

    // ── 5. 색상 합성 ─────────────────────────────
    float coreFade = saturate(dist / 0.5f);

    // [A] 내부 채우기 — Fresnel 무관, 원 전체를 채우는 base fill
    float outerMask = saturate(1.0f - (dist - 0.90f) / 0.09f); // 극외곽만 페이드
    float baseFill  = coreFade * outerMask;                      // 원 전체 불투명도

    // [B] 외곽 Fresnel 림 — HDR 보라 발광
    float rimAlpha  = fresnel * coreFade
                    * saturate(1.0f - (dist - 0.75f) / 0.24f);

    // 색상: 내부=공허 블랙퍼플, 외곽=HDR 보라
    float3 coreColor = float3(0.03f, 0.0f, 0.07f);
    float3 rimColor  = float3(5.5f, 0.0f, 10.0f)
                     * (1.0f + fbmVal * 1.5f)
                     * fresnel;

    // 내부→외곽으로 coreColor → rimColor 전환
    float3 col = lerp(coreColor, rimColor, saturate(fresnel * 2.5f));
    col = lerp(bgColor.rgb, col, baseFill);

    // 알파: 내부 fill(0.85) + 외곽 림 합산
    float alpha = saturate(baseFill * 0.85f + rimAlpha) * fFlameAmt;

    return float4(col * alpha, alpha);
}

technique VoidFlame
{
    pass P0
    {
        VertexShader     = compile vs_3_0 VS_Billboard();
        PixelShader      = compile ps_3_0 PS_VoidFlame();
        ZEnable          = TRUE;
        ZWriteEnable     = FALSE;
        AlphaBlendEnable = TRUE;
        SrcBlend         = ONE;
        DestBlend        = ONE;
        CullMode         = NONE;
    }
}
