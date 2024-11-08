//@legacy-prefix-end 00368
#version 320 es
#define INPUT in
#define FLAT flat
#define OUT_COLOR fragColor
#define TEXTURE texture
#ifdef GL_KHR_blend_equation_advanced
#extension GL_KHR_blend_equation_advanced : enable
#endif
#if defined(GL_KHR_blend_equation_advanced) || __VERSION__>=320
  layout(blend_support_all_equations) out;
#endif
out mediump vec4 fragColor;
