#version 410 core

in vec3 worldCoord;
in vec3 normal;
in vec2 texCoord;
in vec4 shadowCoord;

out vec4 out_Color;

uniform sampler2D diffTex;
uniform sampler2D bumpTex;
uniform sampler2D shadowTex;

uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
uniform vec3 lightPos = vec3(10.0, 20.0, 10.0);
uniform vec3 cameraPos = vec3(0.0, 0.0, 5.0);
uniform float shininess = 32.0;
uniform vec3 color = vec3(0.5, 0.5, 0.5);

uniform int bumpTexEnabled = 1;
uniform int diffTexEnabled = 1;

const float TEX_DELTA = 0.0001;
const float STRENGTH = 0.1;
const float SHADOW_BIAS = 0.001;
const vec2 poissonDisk[64] = vec2[](
                                      vec2(-0.613392, 0.617481),
                                      vec2(0.170019, -0.040254),
                                      vec2(-0.299417, 0.791925),
                                      vec2(0.645680, 0.493210),
                                      vec2(-0.651784, 0.717887),
                                      vec2(0.421003, 0.027070),
                                      vec2(-0.817194, -0.271096),
                                      vec2(-0.705374, -0.668203),
                                      vec2(0.977050, -0.108615),
                                      vec2(0.063326, 0.142369),
                                      vec2(0.203528, 0.214331),
                                      vec2(-0.667531, 0.326090),
                                      vec2(-0.098422, -0.295755),
                                      vec2(-0.885922, 0.215369),
                                      vec2(0.566637, 0.605213),
                                      vec2(0.039766, -0.396100),
                                      vec2(0.751946, 0.453352),
                                      vec2(0.078707, -0.715323),
                                      vec2(-0.075838, -0.529344),
                                      vec2(0.724479, -0.580798),
                                      vec2(0.222999, -0.215125),
                                      vec2(-0.467574, -0.405438),
                                      vec2(-0.248268, -0.814753),
                                      vec2(0.354411, -0.887570),
                                      vec2(0.175817, 0.382366),
                                      vec2(0.487472, -0.063082),
                                      vec2(-0.084078, 0.898312),
                                      vec2(0.488876, -0.783441),
                                      vec2(0.470016, 0.217933),
                                      vec2(-0.696890, -0.549791),
                                      vec2(-0.149693, 0.605762),
                                      vec2(0.034211, 0.979980),
                                      vec2(0.503098, -0.308878),
                                      vec2(-0.016205, -0.872921),
                                      vec2(0.385784, -0.393902),
                                      vec2(-0.146886, -0.859249),
                                      vec2(0.643361, 0.164098),
                                      vec2(0.634388, -0.049471),
                                      vec2(-0.688894, 0.007843),
                                      vec2(0.464034, -0.188818),
                                      vec2(-0.440840, 0.137486),
                                      vec2(0.364483, 0.511704),
                                      vec2(0.034028, 0.325968),
                                      vec2(0.099094, -0.308023),
                                      vec2(0.693960, -0.366253),
                                      vec2(0.678884, -0.204688),
                                      vec2(0.001801, 0.780328),
                                      vec2(0.145177, -0.898984),
                                      vec2(0.062655, -0.611866),
                                      vec2(0.315226, -0.604297),
                                      vec2(-0.780145, 0.486251),
                                      vec2(-0.371868, 0.882138),
                                      vec2(0.200476, 0.494430),
                                      vec2(-0.494552, -0.711051),
                                      vec2(0.612476, 0.705252),
                                      vec2(-0.578845, -0.768792),
                                      vec2(-0.772454, -0.090976),
                                      vec2(0.504440, 0.372295),
                                      vec2(0.155736, 0.065157),
                                      vec2(0.391522, 0.849605),
                                      vec2(-0.620106, -0.328104),
                                      vec2(0.789239, -0.419965),
                                      vec2(-0.545396, 0.538133),
                                      vec2(-0.178564, -0.596057)
                                    );


vec3 getBumpNormal(vec2 texCoord) {
    float hL = texture(bumpTex, texCoord + vec2(-TEX_DELTA, 0.0)).r;
    float hR = texture(bumpTex, texCoord + vec2(TEX_DELTA, 0.0)).r;
    float hD = texture(bumpTex, texCoord + vec2(0.0, -TEX_DELTA)).r;
    float hU = texture(bumpTex, texCoord + vec2(0.0, TEX_DELTA)).r;

    vec3 tangent = vec3(1.0, 0.0, (hR - hL) / (2.0 * TEX_DELTA));
    vec3 bitangent = vec3(0.0, 1.0, (hU - hD) / (2.0 * TEX_DELTA));
    vec3 perturbedNormal = normalize(cross(tangent, bitangent));

    return perturbedNormal;
}

float random(int i){
    float dot_product = dot(vec4(gl_FragCoord.xyz, i), vec4(12.9898, 78.233, 45.164, 94.673));
    return fract(sin(dot_product)*43758.5453);
}

float checkShadow(sampler2D shadowTex, vec3 sCoord, float shadowMapSize, float bias) {

    float visibility = 1.0;
    
    for (int i = 0; i <16; ++i) {
        int index = int(random(i)*64)%64;
        vec2 offset = poissonDisk[index] / 200;
        float shadowMapDepth = texture(shadowTex, sCoord.xy + offset).z;
        if (sCoord.z-bias > shadowMapDepth) {
            visibility -= 0.0625;
        }
    }

    return visibility;
}



void main(void) {

    vec3 N = normalize(normal);
    
    if (bumpTexEnabled != 0) {
        vec3 bumpNormal = getBumpNormal(texCoord);
        N = normalize(N + STRENGTH * bumpNormal);
    }

    vec3 L = normalize(lightPos - worldCoord);
    vec3 V = normalize(cameraPos - worldCoord);
    vec3 R = reflect(-L, N);

    vec3 diffuseColor = color;
    if (diffTexEnabled != 0) {
        diffuseColor = texture(diffTex, texCoord).rgb;
    }
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = lightColor * diffuseColor * NdotL;

    float RdotV = max(dot(R, V), 0.0);
    float specularFactor = pow(RdotV, shininess);
    vec3 specular = lightColor * specularFactor;

    vec3 sCoord = shadowCoord.xyz / shadowCoord.w;
    float visibility = checkShadow(shadowTex, sCoord, 1024.0, 0.002);

    vec3 finalColor = (diffuse + specular) * visibility;
    
    out_Color = vec4(pow(finalColor,vec3(1/2.2)), 1.0);
//    out_Color = vec4(texture(shadowTex,sCoord.xy).rgb, 1.0);
//    out_Color = vec4(sCoord.rgb, 1.0);
}
