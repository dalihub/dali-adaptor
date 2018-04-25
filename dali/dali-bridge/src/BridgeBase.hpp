#ifndef BRIDGE_HPP
#define BRIDGE_HPP

#include "Common.hpp"
#include <memory>

class AppAccessible : public virtual Dali::Accessibility::Accessible, public virtual Dali::Accessibility::Collection
{
public:
  Dali::Accessibility::EmptyAccessibleWithAddress parent;
  std::vector< Dali::Accessibility::Accessible* > children;
  std::string name;

  std::string GetName() override
  {
    return name;
  }
  std::string GetDescription() override
  {
    return "";
  }
  Dali::Accessibility::Accessible* GetParent() override
  {
    return &parent;
  }
  size_t GetChildCount() override
  {
    return children.size();
  }
  Dali::Accessibility::Accessible* GetChildAtIndex( size_t index ) override
  {
    auto s = children.size();
    if( index >= s )
      throw Dali::Accessibility::AccessibleError{"invalid index " + std::to_string( index ) + " for object with " + std::to_string( s ) + " children"};
    return children[index];
  }
  size_t GetIndexInParent() override
  {
    throw Dali::Accessibility::AccessibleError{"can't call GetIndexInParent on application object"};
  }
  Dali::Accessibility::Role GetRole() override
  {
    return Dali::Accessibility::Role::Application;
  }
  Dali::Accessibility::States GetStates() override
  {
    return {};
  }
  Dali::Accessibility::Attributes GetAttributes() override
  {
    return {};
  }
  Dali::Accessibility::Accessible* getActiveWindow()
  {
    return children.empty() ? nullptr : children[0];
  }
};

class BridgeBase : public Dali::Accessibility::Bridge
{
public:
  const std::string& GetBusName() const override;
  void SetApplicationChild( Dali::Accessibility::Accessible* root ) override;
  Dali::Accessibility::Accessible* GetApplication() const override
  {
    return &application;
  }

  template < typename SELF, typename... RET, typename... ARGS >
  void AddFunctionToInterface(
      DBus::DBusInterfaceDescription& desc, const std::string& funcName,
      DBus::ValueOrError< RET... > ( SELF::*funcPtr )( ARGS... ) )
  {
    desc.addMethod< DBus::ValueOrError< RET... >( ARGS... ) >( funcName,
                                                               [=]( ARGS... args ) -> DBus::ValueOrError< RET... > {
                                                                 try
                                                                 {
                                                                   return ( dynamic_cast< SELF* >( this )->*funcPtr )( std::move( args )... );
                                                                 }
                                                                 catch( Dali::Accessibility::AccessibleError& e )
                                                                 {
                                                                   return DBus::Error{e.what()};
                                                                 }
                                                               } );
  }
  template < typename T, typename SELF >
  void AddGetPropertyToInterface( DBus::DBusInterfaceDescription& desc,
                                  const std::string& funcName, T ( SELF::*funcPtr )() )
  {
    desc.addProperty< T >( funcName,
                           [=]() -> DBus::ValueOrError< T > {
                             try
                             {
                               return ( dynamic_cast< SELF* >( this )->*funcPtr )();
                             }
                             catch( Dali::Accessibility::AccessibleError& e )
                             {
                               return DBus::Error{e.what()};
                             }
                           },
                           {} );
  }
  template < typename T, typename SELF >
  void AddSetPropertyToInterface( DBus::DBusInterfaceDescription& desc,
                                  const std::string& funcName, void ( SELF::*funcPtr )( T ) )
  {
    desc.addProperty< T >( funcName, {},
                           [=]( T t ) -> DBus::ValueOrError< void > {
                             try
                             {
                               ( dynamic_cast< SELF* >( this )->*funcPtr )( std::move( t ) );
                               return {};
                             }
                             catch( Dali::Accessibility::AccessibleError& e )
                             {
                               return DBus::Error{e.what()};
                             }
                           } );
  }
  template < typename T, typename T1, typename SELF >
  void AddGetSetPropertyToInterface( DBus::DBusInterfaceDescription& desc,
                                     const std::string& funcName, T1 ( SELF::*funcPtrGet )(), DBus::ValueOrError< void > ( SELF::*funcPtrSet )( T ) )
  {
    desc.addProperty< T >( funcName,
                           [=]() -> DBus::ValueOrError< T > {
                             try
                             {
                               return ( dynamic_cast< SELF* >( this )->*funcPtrGet )();
                             }
                             catch( Dali::Accessibility::AccessibleError& e )
                             {
                               return DBus::Error{e.what()};
                             }
                           },
                           [=]( T t ) -> DBus::ValueOrError< void > {
                             try
                             {
                               ( dynamic_cast< SELF* >( this )->*funcPtrSet )( std::move( t ) );
                               return {};
                             }
                             catch( Dali::Accessibility::AccessibleError& e )
                             {
                               return DBus::Error{e.what()};
                             }
                           } );
  }
  template < typename T, typename T1, typename SELF >
  void AddGetSetPropertyToInterface( DBus::DBusInterfaceDescription& desc,
                                     const std::string& funcName, T1 ( SELF::*funcPtrGet )(), void ( SELF::*funcPtrSet )( T ) )
  {
    desc.addProperty< T >( funcName,
                           [=]() -> DBus::ValueOrError< T > {
                             try
                             {
                               return ( dynamic_cast< SELF* >( this )->*funcPtrGet )();
                             }
                             catch( Dali::Accessibility::AccessibleError& e )
                             {
                               return DBus::Error{e.what()};
                             }
                           },
                           [=]( T t ) -> DBus::ValueOrError< void > {
                             try
                             {
                               ( dynamic_cast< SELF* >( this )->*funcPtrSet )( std::move( t ) );
                               return {};
                             }
                             catch( Dali::Accessibility::AccessibleError& e )
                             {
                               return DBus::Error{e.what()};
                             }
                           } );
  }
  static std::string StripPrefix( const std::string& path );

  Dali::Accessibility::Accessible* Find( const std::string& path ) const;
  Dali::Accessibility::Accessible* Find( const Dali::Accessibility::Address& ptr ) const;
  Dali::Accessibility::Accessible* FindSelf() const;
  Dali::Accessibility::Accessible* FindByPath( const std::string& name ) const override;
  void SetApplicationName( std::string name ) override
  {
    application.name = std::move( name );
  }

protected:
  mutable AppAccessible application;

private:
  void IdSet( int id );
  int IdGet();

  using CacheElementType = std::tuple<
      Dali::Accessibility::Address, Dali::Accessibility::Address, Dali::Accessibility::Address,
      std::vector< Dali::Accessibility::Address >,
      std::vector< std::string >,
      std::string,
      Dali::Accessibility::Role,
      std::string,
      std::array< uint32_t, 2 > >;
  DBus::ValueOrError< std::vector< CacheElementType > > GetItems();
  CacheElementType CreateCacheElement( Dali::Accessibility::Accessible* item );

protected:
  BridgeBase();
  virtual ~BridgeBase() = default;

  ForceUpResult ForceUp() override;
  void ForceDown() override;

  DBus::DBusServer dbusServer;
  std::shared_ptr< DBus::EldbusConnection > con;
  int id = 0;
};

#endif
