#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D framebufferTexture;
uniform sampler2D bloomTexture;
uniform sampler2D depthTexture;

uniform bool enableBloom;
uniform float bloomAmount;
uniform bool enableFog;
uniform vec4 fogColor;
uniform float near;
uniform float far;
uniform float mincutoff;
uniform float maxcuttoff;
uniform float exposure;
uniform bool enableGammaCorrection;
uniform bool enableExposure;
uniform bool enableDepthVisualization;
uniform vec3 colorGrade;
uniform float saturation;
uniform float contrast;
uniform float realtime;
uniform vec2 chromatic;
uniform ivec2 resolution;

// 0.0 = original look
// 1.0 = radical cyber/painterly look
float postProcessAmount = 2;


void calculateFogEffect(in float depthAmount, out vec4 fogAmount)
{
    if (depthAmount < mincutoff || depthAmount > maxcuttoff)
    {
        fogAmount = vec4(0, 0, 0, 0);
    }
    else
    {
        float fromBaseLow = mincutoff;
        float fromBaseHigh = maxcuttoff;

        float toBaseLow = 0;
        float toBaseHigh = 1;

        float newValue =
            ((depthAmount - fromBaseLow) *
            ((toBaseHigh - toBaseLow) /
            (fromBaseHigh - fromBaseLow))) +
            toBaseLow;

        fogAmount =
            mix(
                vec4(0, 0, 0, 0),
                fogColor,
                newValue
            );
    }
}


float lum(vec3 c)
{
    return dot(
        c,
        vec3(0.299, 0.587, 0.114)
    );
}


void main(){
    vec2 uv = TexCoords;
    vec2 texel = 1.0 / vec2(resolution);


    // ========================================================
    // DEPTH
    // ========================================================

    float depth = texture(depthTexture, uv).r;
    float z = depth * 2.0 - 1.0;

    float depthAmount =
        ((2.0 * near * far) /
        (far + near - z * (far - near))) /
        far;


    if (enableDepthVisualization){
        FragColor = vec4(vec3(depthAmount), 1.0);
        return;
    }


    // ========================================================
    // FOG
    // ========================================================

    vec4 fogEffect = vec4(0, 0, 0, 0);
    if (enableFog){
        calculateFogEffect(depthAmount, fogEffect);
    }


    // ========================================================
    // BASE FRAMEBUFFER
    // ========================================================
    vec3 framebufferColor = texture(framebufferTexture, uv).rgb;

    // ========================================================
    // OLD / ORIGINAL LOOK
    // ========================================================

    vec3 oldColor;


    if (enableBloom){
        oldColor =
            fogEffect.rgb +
            framebufferColor +
            (
                bloomAmount *
                texture(
                    bloomTexture,
                    TexCoords
                ).rgb
            );
    }
    else{
        oldColor = fogEffect.rgb + framebufferColor;
    }


    // Original chromatic aberration
    vec2 offset = vec2(chromatic.x, chromatic.y);

    oldColor =
        vec3(
            texture(
                framebufferTexture,
                uv + offset
            ).r,
            texture(
                framebufferTexture,
                uv
            ).g,

            texture(
                framebufferTexture,
                uv - offset
            ).b
        );


    // Original exposure
    if (enableExposure){
        oldColor = vec3(1.0) - exp(-oldColor * exposure);
    }


    // Original gamma
    if (enableGammaCorrection){
        oldColor = pow(oldColor, vec3(1.0 / 2.2));
    }


    // Original saturation
    float oldLuminance =
        dot(
            oldColor,
            vec3(
                0.299,
                0.587,
                0.114
            )
        );

    oldColor =
        mix(
            oldColor,
            vec3(oldLuminance),
            saturation
        );


    // Original contrast
    oldColor =
        (oldColor - 0.5) *
        contrast +
        0.5;


    // Original color grade
    oldColor *= colorGrade;


    // ========================================================
    // RADICAL / CYBER-PAINTERLY LOOK
    // ========================================================

    vec3 paint = framebufferColor;


    // --------------------------------------------------------
    // Bloom
    // --------------------------------------------------------

    if (enableBloom){
        paint += bloomAmount * texture(bloomTexture, uv).rgb;
    }

    paint += fogEffect.rgb;


    // --------------------------------------------------------
    // Flow field
    // --------------------------------------------------------

    float l = lum(paint);
    float lx = lum(texture(framebufferTexture, uv + vec2(texel.x * 3.0, 0.0)).rgb);
    float ly = lum(texture(framebufferTexture, uv + vec2(0.0, texel.y * 3.0)).rgb);


    vec2 flow =
        normalize(
            vec2(
                l - lx,
                l - ly
            ) +
            vec2(0.0001)
        );


    // --------------------------------------------------------
    // Painterly sampling
    // --------------------------------------------------------

    vec3 paintSample = vec3(0.0);

    paintSample +=
        texture(
            framebufferTexture,
            uv - flow * texel * 7.0
        ).rgb * 0.10;

    paintSample +=
        texture(
            framebufferTexture,
            uv - flow * texel * 4.0
        ).rgb * 0.15;

    paintSample +=
        texture(
            framebufferTexture,
            uv - flow * texel * 2.0
        ).rgb * 0.20;

    paintSample +=
        paint * 0.30;

    paintSample +=
        texture(
            framebufferTexture,
            uv + flow * texel * 2.0
        ).rgb * 0.15;

    paintSample +=
        texture(
            framebufferTexture,
            uv + flow * texel * 4.0
        ).rgb * 0.10;


    paint =
        paintSample;


    // --------------------------------------------------------
    // Color quantization
    // --------------------------------------------------------

    float levels = 4.0;

    paint =
        floor(
            paint * levels
        ) / levels;


    // --------------------------------------------------------
    // Cyber color mapping
    // --------------------------------------------------------

    float brightness =
        lum(paint);

    vec3 shadowColor =
        vec3(
            0.04,
            0.01,
            0.12
        );

    vec3 midColor =
        vec3(
            0.05,
            0.20,
            0.35
        );

    vec3 highlightColor =
        vec3(
            0.40,
            0.90,
            1.00
        );

    vec3 cyberColor;

    if (brightness < 0.35)
    {
        cyberColor =
            mix(
                shadowColor,
                midColor,
                brightness / 0.35
            );
    }
    else
    {
        cyberColor =
            mix(
                midColor,
                highlightColor,
                (brightness - 0.35) / 0.65
            );
    }

    paint =
        mix(
            paint,
            paint * cyberColor * 3.0,
            0.45
        );


    // --------------------------------------------------------
    // Sobel edge detection
    // --------------------------------------------------------

    float tl =
        lum(texture(
            framebufferTexture,
            uv + texel * vec2(-1.0, 1.0)
        ).rgb);

    float tc =
        lum(texture(
            framebufferTexture,
            uv + texel * vec2(0.0, 1.0)
        ).rgb);

    float tr =
        lum(texture(
            framebufferTexture,
            uv + texel * vec2(1.0, 1.0)
        ).rgb);

    float ml =
        lum(texture(
            framebufferTexture,
            uv + texel * vec2(-1.0, 0.0)
        ).rgb);

    float mr =
        lum(texture(
            framebufferTexture,
            uv + texel * vec2(1.0, 0.0)
        ).rgb);

    float bl =
        lum(texture(
            framebufferTexture,
            uv + texel * vec2(-1.0, -1.0)
        ).rgb);

    float bc =
        lum(texture(
            framebufferTexture,
            uv + texel * vec2(0.0, -1.0)
        ).rgb);

    float br =
        lum(texture(
            framebufferTexture,
            uv + texel * vec2(1.0, -1.0)
        ).rgb);


    float gx =
        -tl - 2.0 * ml - bl +
        tr + 2.0 * mr + br;

    float gy =
        -tl - 2.0 * tc - tr +
        bl + 2.0 * bc + br;

    float edge =
        sqrt(
            gx * gx +
            gy * gy
        );


    edge =
        smoothstep(
            0.025,
            0.14,
            edge
        );


    // --------------------------------------------------------
    // Animated neon outline
    // --------------------------------------------------------

    vec3 neon =
        vec3(
            0.5 +
            0.5 * sin(realtime * 0.7),

            0.5 +
            0.5 * sin(realtime * 0.7 + 2.0),

            0.5 +
            0.5 * sin(realtime * 0.7 + 4.0)
        );


    paint +=
        neon *
        edge *
        1.5;


    // --------------------------------------------------------
    // RGB displacement
    // --------------------------------------------------------

    vec2 displacement =
        flow *
        texel *
        3.0;

    float red =
        texture(
            framebufferTexture,
            uv + displacement * 2.0
        ).r;

    float green =
        texture(
            framebufferTexture,
            uv
        ).g;

    float blue =
        texture(
            framebufferTexture,
            uv - displacement * 2.0
        ).b;

    vec3 displaced =
        vec3(
            red,
            green,
            blue
        );


    paint =
        mix(
            paint,
            displaced,
            0.20
        );


    // --------------------------------------------------------
    // Moving energy
    // --------------------------------------------------------

    float scan =
        sin(
            uv.y * 35.0 -
            realtime * 2.0
        );

    scan =
        smoothstep(
            0.75,
            1.0,
            scan
        );

    paint +=
        neon *
        scan *
        0.035;


    // --------------------------------------------------------
    // Highlight energy
    // --------------------------------------------------------

    float high =
        smoothstep(
            0.55,
            0.9,
            lum(paint)
        );

    paint +=
        neon *
        high *
        high *
        0.25;


    // ========================================================
    // RADICAL COLOR PIPELINE
    // ========================================================

    if (enableExposure)
    {
        paint =
            vec3(1.0) -
            exp(
                -paint *
                exposure
            );
    }

    if (enableGammaCorrection)
    {
        paint =
            pow(
                max(paint, vec3(0.0)),
                vec3(1.0 / 2.2)
            );
    }


    float paintLuminance =
        dot(
            paint,
            vec3(
                0.299,
                0.587,
                0.114
            )
        );

    paint =
        mix(
            paint,
            vec3(paintLuminance),
            saturation
        );


    paint =
        (paint - 0.5) *
        contrast +
        0.5;


    paint *= colorGrade;


    // ========================================================
    // THE ACTUAL TRANSITION
    // ========================================================
    //
    // 0.0 = original
    // 0.5 = half original / half radical
    // 1.0 = radical
    //

    float amount = clamp(postProcessAmount, 0.0, 1.0);
    vec3 finalColor = mix(oldColor, paint, amount);
    FragColor = vec4(finalColor, 1.0);
}