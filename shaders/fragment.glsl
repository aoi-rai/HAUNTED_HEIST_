#version 120

varying vec3 vNormal;

void main()
{
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
    vec3 normal = normalize(vNormal);

    float diffuse = max(dot(normal, lightDir), 0.2);

    vec3 baseColor = vec3(0.25, 0.45, 0.35);
    vec3 color = baseColor * diffuse;

    gl_FragColor = vec4(color, 1.0);
}
