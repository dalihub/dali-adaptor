//@ignore:on
#define UNIFORM_BLOCK uniform
#define UNIFORM uniform
#define INPUT in
#define OUTPUT out
#define OUT_COLOR gl_FragColor
//@ignore:off

UNIFORM_BLOCK UtcFragBlock
{
  UNIFORM lowp vec4 uColor;
};

INPUT mediump vec2 vTexCoord;
UNIFORM sampler2D sTexture;

OUTPUT mediump vec4 fragColor;

void main()
{
  fragColor = TEXTURE(sTexture, vTexCoord) * uColor;
}
