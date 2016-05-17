
#include "sdl-application.h"
#include <cassert>

#include <dali/public-api/dali-core.h>

#include <SDL/SDL.h>

// main loop function called by emscripten/browser
extern void emscripten_set_main_loop(void (*func)(), int fps, int simulate_infinite_loop);

namespace Dali
{
namespace Internal
{
namespace Emscripten
{
extern void RenderFinished();
};
};
};

Dali::SdlApplication *app = NULL;

void EmscriptenMouseEvent(double x, double y, int downUpMotion)
{
  DALI_ASSERT_ALWAYS(app);

  if(app)
  {
    app->SendTouchEvent(x,y, downUpMotion);
  }
}

void ProcessEvents()
{
  SDL_PumpEvents();

  //
  // This wasnt working prior to emscripten v1.25
  //
  // But it only gives event inside the gl window.
  // When dragging (for rotation etc) we want the drag/rotate to continue outside the window
  //
  // So we'll disable this handling for now
  //
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
  } // Poll(event)

} // ProcessEvents()


void EmscriptenUpdateOnce()
{
  DALI_ASSERT_ALWAYS(app);
  app->DoUpdate();
};

void EmscriptenRenderOnce()
{
  DALI_ASSERT_ALWAYS(app);

  static int w = 0;
  static int h = 0;

  // 'Module' here should be 'dali' with emcc switch -s EXPORT_NAME="dali"
  // but on upgrading to emscripten 1.34.2 it's broken.
  int _x = EM_ASM_INT_V({ return Module.canvas.width;  });
  int _y = EM_ASM_INT_V({ return Module.canvas.height; });

  bool resize = false;
  if( _x != w )
  {
    w = _x;
    resize = true;
  }
  if( _y != h )
  {
    h = _y;
    resize = true;
  }
  if( resize )
  {
    app->SetSurfaceWidth(w, h);
  }

  ProcessEvents();

  EmscriptenUpdateOnce();

  app->DoRender();

  Dali::Internal::Emscripten::RenderFinished();
}

int main(int argc, char *argv[])
{
  using namespace Dali;

  // need to reference everything as emscripten/llvm will cut it all out so put a Actor here
  Dali::Actor actor;

  app = new SdlApplication( 0, 0, SdlApplication::DEFAULT_HORIZONTAL_DPI,  SdlApplication::DEFAULT_VERTICAL_DPI );

  emscripten_set_main_loop(EmscriptenRenderOnce, 0, 1);

  return 1;
}
