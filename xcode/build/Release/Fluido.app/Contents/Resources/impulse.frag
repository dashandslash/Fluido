#version 150

uniform vec2 uPoint;
uniform float uRadius;
uniform vec4 uColor;

out vec4 FragColor;

void main()
{
    float d = distance(uPoint, gl_FragCoord.xy);
    if (d < uRadius) {
        float a = (uRadius - d) * 0.5;
        a = min(a, 1.0);
        FragColor = vec4(uColor.rgb, a);
    } else {
        FragColor = vec4(0);
    }

}

