#version 130

in vec3 normalFrag;
in vec3 lightIntensity; // Add new input variable

uniform vec4 color;

out vec4 outColor;

void main()
{
    // Modulate the fragment color with the interpolated lighting intensity
    outColor = vec4(lightIntensity, 1.0);
}
