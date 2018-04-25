#include "Common.hpp"

using namespace Dali::Accessibility;

std::vector< std::string > Accessible::GetInterfaces()
{
  std::vector< std::string > tmp;
  tmp.push_back( ATSPI_DBUS_INTERFACE_ACCESSIBLE );
  if( dynamic_cast< Collection* >( this ) )
    tmp.push_back( ATSPI_DBUS_INTERFACE_COLLECTION );
  if( dynamic_cast< Text* >( this ) )
    tmp.push_back( ATSPI_DBUS_INTERFACE_TEXT );
  if( dynamic_cast< Value* >( this ) )
    tmp.push_back( ATSPI_DBUS_INTERFACE_VALUE );
  if( dynamic_cast< Component* >( this ) )
    tmp.push_back( ATSPI_DBUS_INTERFACE_COMPONENT );
  if( auto d = dynamic_cast< Action* >( this ) )
  {
    if( d->GetActionCount() > 0 )
      tmp.push_back( ATSPI_DBUS_INTERFACE_ACTION );
  }
  return tmp;
}

thread_local std::atomic< Bridge* > threadLocalBridge{};
std::atomic< Bridge* > allThreads{};

Bridge* Bridge::GetCurrentBridge()
{
  auto p = threadLocalBridge.load();
  if( !p )
    p = allThreads.load();
  return p;
}

Accessible::Accessible()
{
}

Accessible::~Accessible()
{
  auto b = bridgeData.lock();
  if( b )
    b->objects.erase( it );
}

void Bridge::MakePublic( Visibility vis )
{
  bool res = false;
  Bridge* expected = this;

  switch( vis )
  {
    case Visibility::hidden:
    {
      threadLocalBridge.compare_exchange_strong( expected, nullptr );
      allThreads.compare_exchange_strong( expected, nullptr );
      break;
    }
    case Visibility::thisThreadOnly:
    {
      res = threadLocalBridge.exchange( this );
      assert( !res );
      break;
    }
    case Visibility::allThreads:
    {
      res = allThreads.exchange( this );
      assert( !res );
      break;
    }
  }
}

void Accessible::EmitShowing( bool showing )
{
  if( auto b = GetBridgeData() )
  {
    b->bridge->EmitStateChanged( this, State::Showing, showing ? 1 : 0, 0 );
  }
}

void Accessible::EmitVisible( bool visible )
{
  if( auto b = GetBridgeData() )
  {
    b->bridge->EmitStateChanged( this, State::Visible, visible ? 1 : 0, 0 );
  }
}

void Accessible::EmitHighlighted( bool set )
{
  if( auto b = GetBridgeData() )
  {
    b->bridge->EmitStateChanged( this, State::Highlighted, set ? 1 : 0, 0 );
  }
}

void Accessible::Emit( WindowEvent we, unsigned int detail1 )
{
  if( auto b = GetBridgeData() )
  {
    b->bridge->Emit( this, we, detail1 );
  }
}

std::vector< Accessible* > Accessible::GetChildren()
{
  std::vector< Accessible* > tmp( GetChildCount() );
  for( auto i = 0u; i < tmp.size(); ++i )
  {
    tmp[i] = GetChildAtIndex( i );
  }
  return tmp;
}

std::shared_ptr< Bridge::Data > Accessible::GetBridgeData()
{
  auto b = bridgeData.lock();
  if( !b )
  {
    auto p = Bridge::GetCurrentBridge();
    if( !p )
      return {};
    b = p->data;
  }
  return b;
}

Address Accessible::GetAddress()
{
  auto b = bridgeData.lock();
  if( !b )
  {
    b = GetBridgeData();
    assert( b );
    b->bridge->RegisterOnBridge( this );
  }
  return {b->busName, b->root == this ? "root" : std::to_string( it->first )};
}

void Bridge::RegisterOnBridge( Accessible* obj )
{
  assert( !obj->bridgeData.lock() || obj->bridgeData.lock() == data );
  if( !obj->bridgeData.lock() )
  {
    assert( data );
    auto oid = ++data->objectId;
    obj->it = data->objects.insert( {oid, obj} ).first;
    obj->bridgeData = data;
  }
}

bool Accessible::IsProxy()
{
  return false;
}
