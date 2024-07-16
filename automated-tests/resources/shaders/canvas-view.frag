//@ignore:on
#define UNIFORM_BLOCK uniform
#define UNIFORM uniform
#define INPUT in
#define OUTPUT out
#define OUT_COLOR gl_FragColor
//@ignore:off

UNIFORM_BLOCK FragBlock
{
  UNIFORM lowp vec4 uColor;
};

INPUT mediump vec2 vTexCoord;
UNIFORM sampler2D sTexture;

void main()
{
  gl_FragColor = TEXTURE(sTexture, vTexCoord) * uColor;
}
