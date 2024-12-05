#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexColor;

layout(location = 3) in vec4 instanceMatrixRow0;
layout(location = 4) in vec4 instanceMatrixRow1;
layout(location = 5) in vec4 instanceMatrixRow2;
layout(location = 6) in vec4 instanceMatrixRow3;

uniform mat4 viewProjMatrix;

//out vec2 TexCoord;
out vec3 worldPosition;
out vec3 worldNormal;

void main() {
    mat4 instanceModel = mat4(
        instanceMatrixRow0,
        instanceMatrixRow1,
        instanceMatrixRow2,
        instanceMatrixRow3
    );

    worldPosition = (instanceModel * vec4(vertexPosition, 1.0)).xyz;
    worldNormal = mat3(transpose(inverse(instanceModel))) * vertexNormal;
    gl_Position = viewProjMatrix *  instanceModel *  vec4(vertexPosition, 1.0);
    //TexCoord = inTexCoord;
}
