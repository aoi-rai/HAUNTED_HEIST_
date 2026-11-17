
#version 120

varying vec3 vNormal;
varying vec2 vTexCoord;

uniform vec4 materialColor;
uniform sampler2D diffuseTexture;
uniform int useTexture;

void main()
{
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
    vec3 normal = normalize(vNormal);

    float diffuse = max(dot(normal, lightDir), 0.2);

    vec4 baseColor = materialColor;

    if (useTexture == 1) {
        baseColor *= texture2D(diffuseTexture, vTexCoord);
    }

    vec3 color = baseColor.rgb * diffuse;

    gl_FragColor = vec4(color, baseColor.a);
}
