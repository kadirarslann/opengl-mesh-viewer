#version 150

in vec4 vPosition;
in vec4 vColor;
out vec4 color;

uniform mat4 model_view;
uniform mat4 projection;
uniform mat4 rotation;
uniform mat4 scaling;
uniform mat4 translation;
uniform mat4 toorigin;
uniform mat4 fromorigin;


void main() {
    gl_Position = projection * model_view * translation*fromorigin * scaling*rotation*toorigin*vPosition;
    color = vColor;
}
