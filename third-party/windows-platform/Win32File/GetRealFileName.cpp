#include <string>

namespace std
{
const char* GetRealName( const char *name )
{
  if( NULL != name && '*' == name[0] )
  {
    string envName;

    const char *p = name + 1;

    while( 0 != *p && '*' != *p )
    {
      envName.push_back( *p );
      p++;
    }

    p++;

    char *envValue = std::getenv( envName.c_str() );

    static std::string realName;
    realName = "";
    realName += envValue;
    realName += p;

    return realName.c_str();
  }
  else
  {
    return name;
  }
}
}
