#version 130

uniform mat4 projection, modelview;
uniform mat3 normalMatrix;

in vec3 position;
in vec3 normal;
out vec3 normalFrag;
out vec3 lightIntensity; // Add new output variable

void main()
{
    // Transform matrix to viewspace
    normalFrag = normal;
    //normalFrag = normalMatrix * normal;

    // Transform position from pixel coordinates to clipping coordinates
    gl_Position = projection * modelview * vec4(position, 1.0);

    // Compute lighting at the vertex level
    vec3 lightDirection = normalize(vec3(1.0, 2.0, 3.0));
    vec3 lightDirection2 = normalize(vec3(-1.0, 2.0, -3.0));
    float ambient = 0.2;
    float diffuse = max(0.0, dot(normalize(normalFrag), lightDirection));
    diffuse += max(0.0, dot(normalize(normalFrag), lightDirection2));
    float lighting = 0.1f * ambient + 0.8f * diffuse;

    // Pass the lighting intensity to the fragment shader
    lightIntensity = vec3(lighting);
}
