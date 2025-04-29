//@version 100

INPUT mediump vec2 aPosition;// This is postion
INPUT mediump vec2 aTexCoord;/// This is texcort
OUTPUT mediump vec2 vTexCoord;// some output
OUTPUT highp mat3 vMatrix;
OUTPUT medium vec2 vTexCoord2;// other output

// Some vertex block
UNIFORM_BLOCK UtcVertBlock//block comment
{
  UNIFORM highp mat4 uMvpMatrix;//comment0
  UNIFORM highp vec3 uSize;//comment1
};

// Main function
void main()
{

  // Compute position
  gl_Position = uMvpMatrix * vec4(aPosition * uSize.xy, 0.0, 1.0);

  vTexCoord = aPosition + vec2(0.5);// and set up tex coord
}
