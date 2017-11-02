
varying vec2 texCoord;

void main()
{
    gl_Position.xy = gl_Vertex.xy * 2.0 - 1.0;
    gl_Position.zw = gl_Vertex.zw;
    texCoord       = gl_Vertex.xy;
}