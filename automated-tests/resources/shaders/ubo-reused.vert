INPUT mediump vec2 aPosition;
INPUT mediump vec2 aTexCoord;
OUTPUT mediump vec2 vTexCoord;

UNIFORM_BLOCK UtcSharedBlock1
{
  UNIFORM highp vec4 uOther;
};

UNIFORM_BLOCK UtcVertBlock
{
  UNIFORM highp mat4 uMvpMatrix;
  UNIFORM highp vec3 uSize;
};

UNIFORM_BLOCK UtcSharedBlock0
{
  UNIFORM highp vec4 uColor;
};

void main()
{
  gl_Position = uMvpMatrix * vec4(aPosition * uSize.xy, 0.0, 1.0);
  vTexCoord = aPosition + vec2(0.5);
}
