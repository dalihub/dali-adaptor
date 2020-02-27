#include <cstdlib>
#include <string>

using namespace std;

const char* app_get_data_path()
{
  static std::string envValue = "";

  if( true == envValue.empty() )
  {
    envValue = std::getenv( "DemoData" );
    envValue += "/";
  }

  return envValue.c_str();
}
