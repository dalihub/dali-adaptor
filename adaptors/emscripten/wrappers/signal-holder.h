#ifndef DALI_SIGNAL_HOLDER_H
#define DALI_SIGNAL_HOLDER_H

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

struct BaseSignalSlot : public Dali::ConnectionTracker
{
};

/**
 * A wrapper class to let JS hold Signals
 */
class SignalHolder : public Dali::ConnectionTracker
{
public:
  typedef Dali::Vector<BaseSignalSlot*> Slots;
  Slots mSlots;
  SignalHolder() {}

  ~SignalHolder()
  {
    for(Slots::Iterator iter = mSlots.Begin(); iter != mSlots.End(); ++iter)
    {
      delete *iter;
    }
  }

  /*
   * Adds a base signal to the list of slots
   */
  void add(BaseSignalSlot* s) { mSlots.PushBack(s); }

private:
  SignalHolder(const SignalHolder& nocopy);
  SignalHolder& operator=(const SignalHolder& noassign);
};


}; // Emscripten
}; // Internal
}; // Dali


#endif // header
