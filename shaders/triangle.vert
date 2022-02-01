#version 310 es
precision mediump float;
layout(location = 0) in vec4 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec3 Normal;
layout(location = 0) out vec4 vColor;
layout(location = 1) out vec3 worldNormal;
layout(location = 2) out vec3 vertPos;

layout(std140, set = 0, binding = 0) uniform UBO
{
   mat4 MVP;
   mat4 MV;
};

layout(std140, set = 1, binding = 0) uniform MeshInfo
{
   mat4 model_transform;
};

void main()
{
   gl_Position = MVP * model_transform * Position;
   vColor = Color;

   vec4 transformedNormal = transpose(inverse(MV)) * vec4(Normal, 0);
   worldNormal = transformedNormal.xyz;

   vertPos = (MV * Position).xyz;
}
