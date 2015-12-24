#version 150


uniform sampler2D uVelocity;
uniform sampler2D uObstacles;
uniform float uHalfInverseCellSize;

out float FragColor;

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

    // Find neighboring velocities:
    vec2 vN = texelFetchOffset(uVelocity, T, 0, ivec2(0, 1)).xy;
    vec2 vS = texelFetchOffset(uVelocity, T, 0, ivec2(0, -1)).xy;
    vec2 vE = texelFetchOffset(uVelocity, T, 0, ivec2(1, 0)).xy;
    vec2 vW = texelFetchOffset(uVelocity, T, 0, ivec2(-1, 0)).xy;
    
    // Find neighboring obstacles:
    vec3 oN = texelFetchOffset(uObstacles, T, 0, ivec2(0, 1)).xyz;
    vec3 oS = texelFetchOffset(uObstacles, T, 0, ivec2(0, -1)).xyz;
    vec3 oE = texelFetchOffset(uObstacles, T, 0, ivec2(1, 0)).xyz;
    vec3 oW = texelFetchOffset(uObstacles, T, 0, ivec2(-1, 0)).xyz;
    
    // Use obstacle velocities for solid cells:
    if (oN.x > 0) vN = oN.yz;
    if (oS.x > 0) vS = oS.yz;
    if (oE.x > 0) vE = oE.yz;
    if (oW.x > 0) vW = oW.yz;
    
    FragColor = uHalfInverseCellSize * (vE.x - vW.x + vN.y - vS.y);
    
}