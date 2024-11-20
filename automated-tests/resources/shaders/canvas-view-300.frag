#version 300 es

uniform lowp vec4 uColor;
in mediump vec2 vTexCoord;
uniform sampler2D sTexture;
out vec4 fragColor;
void main()
{
  fragColor = texture2D(sTexture, vTexCoord) * uColor;
}
