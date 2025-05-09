//@ignore:on
#define UNIFORM_BLOCK uniform
#define UNIFORM uniform
#define INPUT in
#define OUTPUT out
#define OUT_COLOR gl_FragColor
//@ignore:off

INPUT mediump vec2 aPosition;
INPUT mediump vec2 aTexCoord;
OUTPUT mediump vec2 vTexCoord;
FLAT OUTPUT mediump int vInteger;
UNIFORM_BLOCK UtcVertBlock
{
  UNIFORM highp mat4 uMvpMatrix;
  UNIFORM highp vec3 uSize;
};
void main()
{
  gl_Position = uMvpMatrix * vec4(aPosition * uSize.xy, 0.0, 1.0);
  vInteger = 3;
  vTexCoord = aPosition + vec2(0.5);
}
