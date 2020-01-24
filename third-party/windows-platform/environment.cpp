#include <cstdlib>
#include <string>

using namespace std;

int setenv( const char *__name, const char *__value, int __replace )
{
  string value = __name;
  value += "=";
  value += __value;

  return putenv( value.c_str() );
}

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
