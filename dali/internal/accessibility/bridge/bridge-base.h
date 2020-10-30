#ifndef DALI_INTERNAL_ACCESSIBILITY_BRIDGE_BASE_H
#define DALI_INTERNAL_ACCESSIBILITY_BRIDGE_BASE_H

/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// EXTERNAL INCLUDES
#include <dali/public-api/signals/connection-tracker.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali/internal/accessibility/bridge/accessibility-common.h>

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
      throw std::domain_error{"invalid index " + std::to_string( index ) + " for object with " + std::to_string( s ) + " children"};
    return children[index];
  }

  size_t GetIndexInParent() override
  {
    throw std::domain_error{"can't call GetIndexInParent on application object"};
  }

  Dali::Accessibility::Role GetRole() override
  {
    return Dali::Accessibility::Role::APPLICATION;
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

  bool DoGesture(const Dali::Accessibility::GestureInfo &gestureInfo) override
  {
      return false;
  }

  std::vector<Dali::Accessibility::Relation> GetRelationSet() override
  {
      return {};
  }

  Dali::Accessibility::Address GetAddress() override {
    return { "", "root" };
  }

};


enum class FilteredEvents {
  boundsChanged
};


namespace std {
  template <> struct hash<std::pair<FilteredEvents, Dali::Accessibility::Accessible*>> {
    size_t operator () (std::pair<FilteredEvents, Dali::Accessibility::Accessible*> v) const {
      return (static_cast<size_t>(v.first) * 131) ^ reinterpret_cast<size_t>(v.second);
    }
  };
}


class BridgeBase : public Dali::Accessibility::Bridge, public Dali::ConnectionTracker
{
  std::unordered_map<std::pair<FilteredEvents, Dali::Accessibility::Accessible*>, std::pair<unsigned int, std::function<void()>>> filteredEvents;

  bool tickFilteredEvents();

public:

  void addFilteredEvent(FilteredEvents kind, Dali::Accessibility::Accessible* obj, float delay, std::function<void()> functor);

  const std::string& GetBusName() const override;
  void AddTopLevelWindow( Dali::Accessibility::Accessible* window ) override;
  void RemoveTopLevelWindow( Dali::Accessibility::Accessible* window ) override;
  void AddPopup( Dali::Accessibility::Accessible* ) override;
  void RemovePopup( Dali::Accessibility::Accessible* ) override;

  Dali::Accessibility::Accessible* GetApplication() const override
  {
    return &application;
  }

  template < typename SELF, typename... RET, typename... ARGS >
  void AddFunctionToInterface(
      DBus::DBusInterfaceDescription& desc, const std::string& funcName,
      DBus::ValueOrError< RET... > ( SELF::*funcPtr )( ARGS... ) )
  {
    if ( auto self = dynamic_cast< SELF* >( this ) )
      desc.addMethod< DBus::ValueOrError< RET... >( ARGS... ) >( funcName,
                                                               [=]( ARGS... args ) -> DBus::ValueOrError< RET... > {
                                                                 try
                                                                 {
                                                                   return ( self->*funcPtr )( std::move( args )... );
                                                                 }
                                                                 catch( std::domain_error& e )
                                                                 {
                                                                   return DBus::Error{e.what()};
                                                                 }
                                                               } );
  }

  template < typename T, typename SELF >
  void AddGetPropertyToInterface( DBus::DBusInterfaceDescription& desc,
                                  const std::string& funcName, T ( SELF::*funcPtr )() )
  {
    if ( auto self = dynamic_cast< SELF* >( this ) )
      desc.addProperty< T >( funcName,
                           [=]() -> DBus::ValueOrError< T > {
                             try
                             {
                               return ( self->*funcPtr )();
                             }
                             catch( std::domain_error& e )
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
    if ( auto self = dynamic_cast< SELF* >( this ) )
      desc.addProperty< T >( funcName, {},
                           [=]( T t ) -> DBus::ValueOrError< void > {
                             try
                             {
                               ( self->*funcPtr )( std::move( t ) );
                               return {};
                             }
                             catch( std::domain_error& e )
                             {
                               return DBus::Error{e.what()};
                             }
                           } );
  }

  template < typename T, typename T1, typename SELF >
  void AddGetSetPropertyToInterface( DBus::DBusInterfaceDescription& desc,
                                     const std::string& funcName, T1 ( SELF::*funcPtrGet )(), DBus::ValueOrError< void > ( SELF::*funcPtrSet )( T ) )
  {
    if ( auto self = dynamic_cast< SELF* >( this ) )
      desc.addProperty< T >( funcName,
                           [=]() -> DBus::ValueOrError< T > {
                             try
                             {
                               return ( self->*funcPtrGet )();
                             }
                             catch( std::domain_error& e )
                             {
                               return DBus::Error{e.what()};
                             }
                           },
                           [=]( T t ) -> DBus::ValueOrError< void > {
                             try
                             {
                               ( self->*funcPtrSet )( std::move( t ) );
                               return {};
                             }
                             catch( std::domain_error& e )
                             {
                               return DBus::Error{e.what()};
                             }
                           } );
  }
  template < typename T, typename T1, typename SELF >
  void AddGetSetPropertyToInterface( DBus::DBusInterfaceDescription& desc,
                                     const std::string& funcName, T1 ( SELF::*funcPtrGet )(), void ( SELF::*funcPtrSet )( T ) )
  {
    if ( auto self = dynamic_cast< SELF* >( this ) )
      desc.addProperty< T >( funcName,
                           [=]() -> DBus::ValueOrError< T > {
                             try
                             {
                               return ( self->*funcPtrGet )();
                             }
                             catch( std::domain_error& e )
                             {
                               return DBus::Error{e.what()};
                             }
                           },
                           [=]( T t ) -> DBus::ValueOrError< void > {
                             try
                             {
                               ( self->*funcPtrSet )( std::move( t ) );
                               return {};
                             }
                             catch( std::domain_error& e )
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
  std::vector<Dali::Accessibility::Accessible*> popups;
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
  virtual ~BridgeBase();

  ForceUpResult ForceUp() override;
  void ForceDown() override;

  DBus::DBusServer dbusServer;
  DBusWrapper::ConnectionPtr con;
  int id = 0;
};

#endif // DALI_INTERNAL_ACCESSIBILITY_BRIDGE_BASE_H
