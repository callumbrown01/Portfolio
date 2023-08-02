attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec2 vTexCoord;

// for fragment shader
varying vec4 position;
varying vec3 normal;

varying vec2 texCoord;
varying vec4 color;

uniform mat4 ModelView;
uniform mat4 Projection;

void main()
{
    vec4 vpos = vec4(vPosition, 1.0);

    gl_Position = Projection * ModelView * vpos;

    // values for the fragshader
    texCoord = vTexCoord;
    position = vpos;
    normal = vNormal;
}