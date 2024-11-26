//@version 100

// Some amazing uniform block
UNIFORM_BLOCK FragBlock// another breaking comment
{
  UNIFORM lowp vec4 uColor;// This is color (used to cause a parsing error!)
};

// Input coords
INPUT mediump vec2 vTexCoord;
INPUT mediump vec2 vTexCoord2;
INPUT highp mat3 vMatrix;

UNIFORM sampler2D sTexture;// Sampler two-dimensional
UNIFORM samplerCube sTextureCube;// Cube sampler for fun

void main()
{

  // Using GLSL100 semantics
  vec4 skyboxColor = textureCube(sTextureCube, vec3(0,0,0));

  // Using modern semantics
  vec4 skyboxColor2 = TEXTURE_CUBE(sTextureCube, vec3(0,0,0));

  gl_FragColor = TEXTURE(sTexture, vTexCoord) * uColor;
}
