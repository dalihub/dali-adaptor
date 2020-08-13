#include <cstdlib>
#include <string>

const char* app_get_data_path()
{
  static std::string envValue;

  if( envValue.empty() )
  {
    envValue = std::getenv( "DemoData" );
    envValue += "/";
  }

  return envValue.c_str();
}
