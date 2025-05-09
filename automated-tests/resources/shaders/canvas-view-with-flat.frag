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
FLAT INPUT mediump int vInteger;
UNIFORM sampler2D sTexture;
UNIFORM samplerCube sTextureCube;

void main()
{

  // Using GLSL100 semantics
  vec4 skyboxColor = textureCube(sTextureCube, vec3(0,0,0));

  // Using modern semantics
  vec4 skyboxColor2 = TEXTURE_CUBE(sTextureCube, vec3(0,0,0));

  gl_FragColor = TEXTURE(sTexture, vTexCoord) * uColor;

  if(vInteger < 2)
  {
    gl_FragColor.r = 1.0;
  }
}
