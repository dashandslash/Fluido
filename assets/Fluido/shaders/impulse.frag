#version 150

uniform vec2 uPoint;
uniform float uRadius;
uniform vec4 uColor;

out vec4 FragColor;

void main()
{
    float d = distance(uPoint, gl_FragCoord.xy);
    FragColor = vec4(uColor);
    if (d < uRadius) {
        float a = max((uRadius - d) / uRadius, 0.0);
        a = min(a, 1.0);
        FragColor += vec4(uColor);
        FragColor.a = pow(a, 3.0);
    } else {
        FragColor = vec4(0);
    }

}

