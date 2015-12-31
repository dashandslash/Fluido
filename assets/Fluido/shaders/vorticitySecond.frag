#version 410

uniform sampler2D uVorticity;
uniform float uDeltaT;
uniform float uConfinementScale;
uniform float uHalfInverseCellSize;

in vec2 vTexCoord;
out vec4 			FragColor;
								  
void main(){

    ivec2 T = ivec2(gl_FragCoord.st);
    
    // Find neighboring obstacles:
    float voN = texelFetchOffset(uVorticity, T, 0, ivec2(0, 1)).x;
    float voS = texelFetchOffset(uVorticity, T, 0, ivec2(0, -1)).x;
    float voE = texelFetchOffset(uVorticity, T, 0, ivec2(1, 0)).x;
    float voW = texelFetchOffset(uVorticity, T, 0, ivec2(-1, 0)).x;

	float voC = texture(uVorticity, vTexCoord).x;

	vec2 force = uHalfInverseCellSize * vec2(abs(voN) - abs(voS), abs(voE) - abs(voW));

	// safe normalize
	const float EPSILON = 2.4414e-4; // 2^-12

	float magSqr = max(EPSILON, dot(force, force));
	force = force * inversesqrt(magSqr);

	force *= uConfinementScale * voC * vec2(1., -1.);
	force *= uDeltaT;

	FragColor = vec4(force, 0.0, 1.0);
}