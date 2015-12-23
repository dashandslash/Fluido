#version 150


uniform sampler2D uVelocity;
uniform sampler2D uTemperature;
uniform sampler2D uDensity;

uniform float uAmbientTemperature;
uniform float uTimeStep;
uniform float uSigma;
uniform float uKappa;


out vec2 FragColor;


void main()
{
    ivec2 TC = ivec2(gl_FragCoord.xy);
    vec3 temperature = (texelFetch(uTemperature, TC, 0).rgb);
    float T = length(temperature);
    vec2 V = texelFetch(uVelocity, TC, 0).xy;

    FragColor = V;
    
    if (T > uAmbientTemperature)
    {
        vec3 density = (texelFetch(uDensity, TC, 0).rgb);
        float D = length(density);
        FragColor += (uTimeStep * (T - uAmbientTemperature) * uSigma - D * uKappa ) * vec2(-0.40, 0.0);
    }
}
