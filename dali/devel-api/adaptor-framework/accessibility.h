#ifndef ATSPI_BRIDGE_HPP
#define ATSPI_BRIDGE_HPP

#include <dali/public-api/actors/actor.h>
#include <dali/public-api/adaptor-framework/accessibility.h>

#include <atomic>
#include <bitset>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Dali
{
namespace Accessibility
{
class Accessible;
class Text;
class Value;
class Component;
class Collection;
class Action;

using ObjectsMapType = std::unordered_map< unsigned int, Accessible* >;

struct DALI_IMPORT_API Bridge
{
  enum class Visibility
  {
    hidden,
    thisThreadOnly,
    allThreads
  };
  enum class ForceUpResult
  {
    justStarted,
    alreadyUp
  };
  virtual ~Bridge() = default;

  virtual const std::string& GetBusName() const = 0;
  virtual void SetApplicationChild( Accessible* ) = 0;
  virtual void SetApplicationName( std::string ) = 0;
  virtual Accessible* GetApplication() const = 0;
  virtual Accessible* FindByPath( const std::string& ) const = 0;
  virtual void ApplicationShown() = 0;
  virtual void ApplicationHidden() = 0;
  virtual void Initialize() = 0;
  virtual ForceUpResult ForceUp()
  {
    if( data )
      return ForceUpResult::alreadyUp;
    data = std::make_shared< Data >();
    data->bridge = this;
    return ForceUpResult::justStarted;
  }
  virtual void ForceDown()
  {
    data = {};
  }
  bool IsUp() const { return bool(data); }

  virtual void EmitStateChanged( Accessible* obj, State state, int val1, int val2 = 0 ) = 0;
  virtual void Emit( Accessible* obj, WindowEvent we, unsigned int detail1 = 0 ) = 0;
  virtual Consumed Emit( KeyEventType type, unsigned int keyCode, const std::string& keyName, unsigned int timeStamp, bool isText ) = 0;
  void MakePublic( Visibility );

  static Bridge* GetCurrentBridge();

protected:
  struct Data
  {
    ObjectsMapType objects;
    std::atomic< unsigned int > objectId;
    std::string busName;
    Accessible* root = nullptr;
    Bridge* bridge = nullptr;
  };
  std::shared_ptr< Data > data;
  friend class Accessible;

  void RegisterOnBridge( Accessible* );
  void SetIsOnRootLevel( Accessible* );
};

inline bool IsUp()
{
  return Bridge::GetCurrentBridge()->IsUp();
}

class DALI_IMPORT_API Accessible
{
protected:
  Accessible();

  Accessible( const Accessible& ) = delete;
  Accessible( Accessible&& ) = delete;

  Accessible& operator=( const Accessible& ) = delete;
  Accessible& operator=( Accessible&& ) = delete;

public:
  virtual ~Accessible();

  void EmitShowing( bool showing );
  void EmitVisible( bool visible );
  void EmitHighlighted( bool set );
  void Emit( WindowEvent we, unsigned int detail1 = 0 );

  virtual std::string GetName() = 0;
  virtual std::string GetDescription() = 0;
  virtual Accessible* GetParent() = 0;
  virtual size_t GetChildCount() = 0;
  virtual std::vector< Accessible* > GetChildren();
  virtual Accessible* GetChildAtIndex( size_t index ) = 0;
  virtual size_t GetIndexInParent() = 0;
  virtual Role GetRole() = 0;
  virtual std::string GetRoleName();
  virtual std::string GetLocalizedRoleName();
  virtual States GetStates() = 0;
  virtual Attributes GetAttributes() = 0;
  virtual bool IsProxy();
  virtual Address GetAddress();

  std::vector< std::string > GetInterfaces();
  bool GetIsOnRootLevel() const { return isOnRootLevel; }

  static void RegisterControlAccessibilityGetter( std::function< Accessible*( Dali::Actor ) > );
  static Accessible* Get( Dali::Actor actor );

protected:
  std::shared_ptr< Bridge::Data > GetBridgeData();

private:
  friend class Bridge;

  std::weak_ptr< Bridge::Data > bridgeData;
  ObjectsMapType::iterator it;
  bool isOnRootLevel = false;
};

class DALI_IMPORT_API Action : public virtual Accessible
{
public:
  virtual std::string GetActionName( size_t index ) = 0;
  virtual std::string GetLocalizedActionName( size_t index ) = 0;
  virtual std::string GetActionDescription( size_t index ) = 0;
  virtual std::string GetActionKeyBinding( size_t index ) = 0;
  virtual size_t GetActionCount() = 0;
  virtual bool DoAction( size_t index ) = 0;
};

class DALI_IMPORT_API Collection : public virtual Accessible
{
public:
};

class DALI_IMPORT_API Component : public virtual Accessible
{
public:
  virtual Rectangle GetExtents( CoordType ctype ) = 0;
  virtual ComponentLayer GetLayer() = 0;
  virtual int GetMdiZOrder() = 0;
  virtual bool GrabFocus() = 0;
  virtual double GetAlpha() = 0;
  virtual bool SetExtents( Rectangle rect, CoordType ctype ) = 0;
  virtual bool GrabHighlight() = 0;
  virtual bool ClearHighlight() = 0;
  virtual int GetHighlightIndex() = 0;
  virtual bool IsScrollable();
  virtual Component* GetAccessibleAtPoint( Point p, CoordType ctype );
  virtual bool Contains( Point p, CoordType ctype );
};

class DALI_IMPORT_API Value : public virtual Accessible
{
public:
  virtual double GetMinimum() = 0;
  virtual double GetCurrent() = 0;
  virtual double GetMaximum() = 0;
  virtual bool SetCurrent( double ) = 0;
  virtual double GetMinimumIncrement() = 0;
};

class DALI_IMPORT_API Text : public virtual Accessible
{
public:
  virtual std::string GetText( size_t startOffset, size_t endOffset ) = 0;
  virtual size_t GetCharacterCount() = 0;
  virtual Range GetTextAtOffset( size_t offset, TextBoundary boundary ) = 0;
  virtual Range GetSelection( size_t selectionNum ) = 0;
  virtual bool RemoveSelection( size_t selectionNum ) = 0;
  virtual bool SetSelection( size_t selectionNum, size_t startOffset, size_t endOffset ) = 0;
};

class DALI_IMPORT_API EditableText : public virtual Accessible
{
public:
  virtual bool CopyText( size_t startPosition, size_t endPosition ) = 0;
  virtual bool CutText( size_t startPosition, size_t endPosition ) = 0;
};

/**
         * @brief minimalistic, always empty Accessible object with settable address
         *
         * For those situations, where you want to return address in different bridge
         * (embedding for example), but the object itself ain't planned to be used otherwise.
         * This object has null parent, no children, empty name and so on
         */
class DALI_IMPORT_API EmptyAccessibleWithAddress : public virtual Accessible
{
public:
  EmptyAccessibleWithAddress() = default;
  EmptyAccessibleWithAddress( Address address ) : address( std::move( address ) ) {}

  void SetAddress( Address address ) { this->address = std::move( address ); }

  std::string GetName() override { return ""; }
  std::string GetDescription() override { return ""; }
  Accessible* GetParent() override { return nullptr; }
  size_t GetChildCount() override { return 0; }
  std::vector< Accessible* > GetChildren() override { return {}; }
  Accessible* GetChildAtIndex( size_t index ) override
  {
    throw AccessibleError{"out of bounds index (" + std::to_string( index ) + ") - no children"};
  }
  size_t GetIndexInParent() override { return static_cast< size_t >( -1 ); }
  Role GetRole() override { return {}; }
  std::string GetRoleName() override;
  States GetStates() override { return {}; }
  Attributes GetAttributes() override { return {}; }
  Address GetAddress() override
  {
    return address;
  }

private:
  Address address;
};

std::shared_ptr< Bridge > CreateBridge();
}
}

#endif
