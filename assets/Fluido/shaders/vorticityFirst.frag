
#version 410

uniform sampler2D 	uVelocity;
uniform sampler2D 	uObstacle;


out vec4 			FragColor;
								  
void v2TexNeighbors(sampler2D tex, vec2 st, out vec2 left, out vec2 right, out vec2 bottom, out vec2 top) {
	left   = textureOffset(tex, st, ivec2(-1, 0)).xy;
	right  = textureOffset(tex, st, ivec2(1, 0)).xy;
	bottom = textureOffset(tex, st, ivec2(0, -1)).xy;
	top    = textureOffset(tex, st, ivec2(0, 1)).xy;
}

void fRoundTexNeighbors(sampler2D tex, vec2 st, out float left, out float right, out float bottom, out float top) {
	left   = ceil(textureOffset(tex, st, ivec2(-1, 0)).x - 0.5); // round not available
	right  = ceil(textureOffset(tex, st, ivec2(1, 0)).x - 0.5);
	bottom = ceil(textureOffset(tex, st, ivec2(0, -1)).x - 0.5);
	top    = ceil(textureOffset(tex, st, ivec2(0, 1)).x - 0.5);
}

void main(){
    
    ivec2 T = ivec2(gl_FragCoord.st);
    
    // Find neighboring velocities:
    vec2 vN = texelFetchOffset(uVelocity, T, 0, ivec2(0, 1)).xy;
    vec2 vS = texelFetchOffset(uVelocity, T, 0, ivec2(0, -1)).xy;
    vec2 vE = texelFetchOffset(uVelocity, T, 0, ivec2(1, 0)).xy;
    vec2 vW = texelFetchOffset(uVelocity, T, 0, ivec2(-1, 0)).xy;
    
    // Find neighboring obstacles:
    vec3 oN = texelFetchOffset(uObstacle, T, 0, ivec2(0, 1)).xyz;
    vec3 oS = texelFetchOffset(uObstacle, T, 0, ivec2(0, -1)).xyz;
    vec3 oE = texelFetchOffset(uObstacle, T, 0, ivec2(1, 0)).xyz;
    vec3 oW = texelFetchOffset(uObstacle, T, 0, ivec2(-1, 0)).xyz;

    vec2 vC = texture(uVelocity, T).xy;

    // Use obstacle velocities for solid cells:
    if (oN.x > 0) vN = oN.yz;
    if (oS.x > 0) vS = oS.yz;
    if (oE.x > 0) vE = oE.yz;
    if (oW.x > 0) vW = oW.yz;


	float vorticity = 0.5 * ((vE.y - vW.y) - (vN.x - vS.x));
	FragColor = vec4(vorticity, 0.0, 0.0, 1.0);

}