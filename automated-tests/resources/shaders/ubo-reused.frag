
UNIFORM_BLOCK SharedBlock0
{
  UNIFORM highp vec4 uOther;
};

UNIFORM_BLOCK FragBlock
{
  UNIFORM lowp vec4 uColor;
};

INPUT mediump vec2 vTexCoord;
UNIFORM sampler2D sTexture;
UNIFORM samplerCube sTextureCube;

UNIFORM_BLOCK SharedBlock1
{
  UNIFORM highp vec4 uColor;
};

void main()
{

  // Using GLSL100 semantics
  vec4 skyboxColor = textureCube(sTextureCube, vec3(0,0,0));

  // Using modern semantics
  vec4 skyboxColor2 = TEXTURE_CUBE(sTextureCube, vec3(0,0,0));

  gl_FragColor = TEXTURE(sTexture, vTexCoord) * uColor;
}
