#version 150

uniform sampler2D uPressureTex;
uniform sampler2D uDivergenceTex;
uniform sampler2D uObstacles;

uniform float uAlpha;
uniform float uInverseBeta;

in vec2 vTexCoord;
out vec4 FragColor;

void main()
{   //window space uv coordinates
    ivec2 T = ivec2(gl_FragCoord.xy);
    
    // Find neighboring pressure:
    vec4 pN = texelFetchOffset(uPressureTex, T, 0, ivec2(0, 1));
    vec4 pS = texelFetchOffset(uPressureTex, T, 0, ivec2(0, -1));
    vec4 pE = texelFetchOffset(uPressureTex, T, 0, ivec2(1, 0));
    vec4 pW = texelFetchOffset(uPressureTex, T, 0, ivec2(-1, 0));
    vec4 pC = texelFetch(uPressureTex, T, 0);
    
    // Find neighboring obstacles:
    vec3 oN = texelFetchOffset(uObstacles, T, 0, ivec2(0, 1)).xyz;
    vec3 oS = texelFetchOffset(uObstacles, T, 0, ivec2(0, -1)).xyz;
    vec3 oE = texelFetchOffset(uObstacles, T, 0, ivec2(1, 0)).xyz;
    vec3 oW = texelFetchOffset(uObstacles, T, 0, ivec2(-1, 0)).xyz;
    
    // Use center pressure for solid cells:
    if (oN.x > 0) pN = pC;
    if (oS.x > 0) pS = pC;
    if (oE.x > 0) pE = pC;
    if (oW.x > 0) pW = pC;
    
    vec4 bC = texelFetch(uDivergenceTex, T, 0);
    FragColor = (pW + pE + pS + pN + uAlpha * bC) * uInverseBeta;
}