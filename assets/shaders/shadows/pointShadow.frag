#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    // distance between fragment and light source
    float distance = length(FragPos.xyz - lightPos);

    // map to [0,1] range by dividing by far_plane
    distance = distance / far_plane;

    gl_FragDepth = distance;
}
