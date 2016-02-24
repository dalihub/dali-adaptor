#include "sdl-gl-sync-abstraction.h"

using namespace Dali;


class SdlSyncObject : public Integration::GlSyncAbstraction::SyncObject
{
public:
  virtual bool IsSynced() { return true; }
};


SdlGlSyncAbstraction::~SdlGlSyncAbstraction()
{
}

bool SdlGlSyncAbstraction::SyncObject::IsSynced()
{
  return true;
}

Integration::GlSyncAbstraction::SyncObject* SdlGlSyncAbstraction::CreateSyncObject()
{
  return new SdlSyncObject();
}


void SdlGlSyncAbstraction::DestroySyncObject(Integration::GlSyncAbstraction::SyncObject* syncObject)
{
  delete static_cast<SdlSyncObject*>( syncObject );
}

