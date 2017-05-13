
uniform sampler2D diffuse;

uniform vec2 lightPosition;

varying vec2 texCoordVar;
varying vec2 varPosition;

float attenuate(float dist, float a, float b)
{
    return 1.0 / (1.0 + a * dist + b * dist * dist);
}

void main() {
    float brightness = attenuate(distance(lightPosition, varPosition), 10.0, 10.0);
    vec4 textureColor = texture2D(diffuse, texCoordVar);
    gl_FragColor = textureColor * (0.7 + brightness);
    gl_FragColor.a = textureColor.a;
}
