/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

// CLASS  HEADER
#include <dali/internal/text/text-abstraction/bidirectional-support-impl.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/common/singleton-service.h>
#include <dali/integration-api/debug.h>
#include <fribidi/fribidi.h>
#include <memory.h>

namespace Dali
{
namespace TextAbstraction
{
namespace Internal
{
namespace
{
typedef unsigned char BidiDirection;

// Internal charcter's direction.
const BidiDirection LEFT_TO_RIGHT = 0u;
const BidiDirection NEUTRAL       = 1u;
const BidiDirection RIGHT_TO_LEFT = 2u;

/**
   * @param[in] paragraphDirection The FriBiDi paragraph's direction.
   *
   * @return Whether the paragraph is right to left.
   */
bool GetBidiParagraphDirection(FriBidiParType paragraphDirection)
{
  switch(paragraphDirection)
  {
    case FRIBIDI_PAR_RTL:  // Right-To-Left paragraph.
    case FRIBIDI_PAR_WRTL: // Weak Right To Left paragraph.
    {
      return true;
    }
    case FRIBIDI_PAR_LTR:  // Left-To-Right paragraph.
    case FRIBIDI_PAR_ON:   // DirectiOn-Neutral paragraph.
    case FRIBIDI_PAR_WLTR: // Weak Left To Right paragraph.
    {
      return false;
    }
  }

  return false;
}
} // namespace

struct BidirectionalSupport::Plugin
{
  /**
   * Stores bidirectional info per paragraph.
   */
  struct BidirectionalInfo
  {
    FriBidiCharType*    characterTypes;     ///< The type of each character (right, left, neutral, ...)
    FriBidiBracketType* bracketTypes;       ///< Input list of bracket types as returned by fribidi_get_bracket_types().
    FriBidiLevel*       embeddedLevels;     ///< Embedded levels.
    FriBidiParType      paragraphDirection; ///< The paragraph's direction.
  };

  Plugin()
  : mParagraphBidirectionalInfo(),
    mFreeIndices()
  {
  }

  ~Plugin()
  {
    // free all resources.
    for(Vector<BidirectionalInfo*>::Iterator it    = mParagraphBidirectionalInfo.Begin(),
                                             endIt = mParagraphBidirectionalInfo.End();
        it != endIt;
        ++it)
    {
      BidirectionalInfo* info = *it;

      free(info->embeddedLevels);
      free(info->characterTypes);
      free(info->bracketTypes);
      delete info;
    }
  }

  BidiInfoIndex CreateInfo(const Character* const paragraph,
                           Length                 numberOfCharacters,
                           bool                   matchLayoutDirection,
                           LayoutDirection::Type  layoutDirection)
  {
    // Reserve memory for the paragraph's bidirectional info.
    BidirectionalInfo* bidirectionalInfo = new BidirectionalInfo();

    bidirectionalInfo->characterTypes = reinterpret_cast<FriBidiCharType*>(malloc(numberOfCharacters * sizeof(FriBidiCharType)));
    if(!bidirectionalInfo->characterTypes)
    {
      delete bidirectionalInfo;
      return 0;
    }

    bidirectionalInfo->embeddedLevels = reinterpret_cast<FriBidiLevel*>(malloc(numberOfCharacters * sizeof(FriBidiLevel)));
    if(!bidirectionalInfo->embeddedLevels)
    {
      free(bidirectionalInfo->characterTypes);
      delete bidirectionalInfo;
      return 0;
    }

    // Retrieve the type of each character..
    fribidi_get_bidi_types(paragraph, numberOfCharacters, bidirectionalInfo->characterTypes);

    // Retrieve the paragraph's direction.
    bidirectionalInfo->paragraphDirection = matchLayoutDirection == true ? (layoutDirection == LayoutDirection::RIGHT_TO_LEFT ? FRIBIDI_PAR_RTL : FRIBIDI_PAR_LTR) : (fribidi_get_par_direction(bidirectionalInfo->characterTypes, numberOfCharacters));

    bidirectionalInfo->bracketTypes = reinterpret_cast<FriBidiBracketType*>(malloc(numberOfCharacters * sizeof(FriBidiBracketType)));
    if(!bidirectionalInfo->bracketTypes)
    {
      free(bidirectionalInfo->bracketTypes);
      delete bidirectionalInfo;
      return 0;
    }

    fribidi_get_bracket_types(paragraph, numberOfCharacters, bidirectionalInfo->characterTypes, bidirectionalInfo->bracketTypes);

    // Retrieve the embedding levels.
    if(fribidi_get_par_embedding_levels_ex(bidirectionalInfo->characterTypes, bidirectionalInfo->bracketTypes, numberOfCharacters, &bidirectionalInfo->paragraphDirection, bidirectionalInfo->embeddedLevels) == 0)
    {
      free(bidirectionalInfo->characterTypes);
      free(bidirectionalInfo->bracketTypes);
      delete bidirectionalInfo;
      return 0;
    }

    // Store the bidirectional info and return the index.
    BidiInfoIndex index = 0u;
    if(0u != mFreeIndices.Count())
    {
      Vector<BidiInfoIndex>::Iterator it = mFreeIndices.End() - 1u;

      index = *it;

      mFreeIndices.Remove(it);

      *(mParagraphBidirectionalInfo.Begin() + index) = bidirectionalInfo;
    }
    else
    {
      index = static_cast<BidiInfoIndex>(mParagraphBidirectionalInfo.Count());

      mParagraphBidirectionalInfo.PushBack(bidirectionalInfo);
    }

    return index;
  }

  void DestroyInfo(BidiInfoIndex bidiInfoIndex)
  {
    if(bidiInfoIndex >= mParagraphBidirectionalInfo.Count())
    {
      return;
    }

    // Retrieve the paragraph's bidirectional info.
    Vector<BidirectionalInfo*>::Iterator it                = mParagraphBidirectionalInfo.Begin() + bidiInfoIndex;
    BidirectionalInfo*                   bidirectionalInfo = *it;

    if(NULL != bidirectionalInfo)
    {
      // Free resources and destroy the container.
      free(bidirectionalInfo->embeddedLevels);
      free(bidirectionalInfo->characterTypes);
      free(bidirectionalInfo->bracketTypes);
      delete bidirectionalInfo;

      *it = NULL;
    }

    // Add the index to the free indices vector.
    mFreeIndices.PushBack(bidiInfoIndex);
  }

  void Reorder(BidiInfoIndex   bidiInfoIndex,
               CharacterIndex  firstCharacterIndex,
               Length          numberOfCharacters,
               CharacterIndex* visualToLogicalMap)
  {
    const FriBidiFlags flags = FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC;

    // Retrieve the paragraph's bidirectional info.
    const BidirectionalInfo* const bidirectionalInfo = *(mParagraphBidirectionalInfo.Begin() + bidiInfoIndex);

    // Initialize the visual to logical mapping table to the identity. Otherwise fribidi_reorder_line fails to retrieve a valid mapping table.
    for(CharacterIndex index = 0u; index < numberOfCharacters; ++index)
    {
      visualToLogicalMap[index] = index;
    }

    // Copy embedded levels as fribidi_reorder_line() may change them.
    const size_t  embeddedLevelsSize = static_cast<std::size_t>(numberOfCharacters) * sizeof(FriBidiLevel);
    FriBidiLevel* embeddedLevels     = reinterpret_cast<FriBidiLevel*>(malloc(embeddedLevelsSize));
    if(embeddedLevels)
    {
      memcpy(embeddedLevels, bidirectionalInfo->embeddedLevels + firstCharacterIndex, embeddedLevelsSize);

      // Reorder the line.
      if(fribidi_reorder_line(flags,
                              bidirectionalInfo->characterTypes + firstCharacterIndex,
                              numberOfCharacters,
                              0u,
                              bidirectionalInfo->paragraphDirection,
                              embeddedLevels,
                              NULL,
                              reinterpret_cast<FriBidiStrIndex*>(visualToLogicalMap)) == 0)
      {
        DALI_LOG_ERROR("fribidi_reorder_line is failed\n");
      }

      // Free resources.
      free(embeddedLevels);
    }
  }

  bool GetMirroredText(Character*          text,
                       CharacterDirection* directions,
                       Length              numberOfCharacters) const
  {
    bool updated = false;

    for(CharacterIndex index = 0u; index < numberOfCharacters; ++index)
    {
      // Get a reference to the character inside the text.
      Character& character = *(text + index);

      // Retrieve the mirrored character.
      FriBidiChar mirroredCharacter = character;
      bool        mirrored          = false;
      if(*(directions + index))
      {
        mirrored = fribidi_get_mirror_char(character, &mirroredCharacter);
      }
      updated = updated || mirrored;

      // Update the character inside the text.
      character = mirroredCharacter;
    }

    return updated;
  }

  bool GetParagraphDirection(BidiInfoIndex bidiInfoIndex) const
  {
    // Retrieve the paragraph's bidirectional info.
    const BidirectionalInfo* const bidirectionalInfo = *(mParagraphBidirectionalInfo.Begin() + bidiInfoIndex);

    return GetBidiParagraphDirection(bidirectionalInfo->paragraphDirection);
  }

  void GetCharactersDirection(BidiInfoIndex       bidiInfoIndex,
                              CharacterDirection* directions,
                              Length              numberOfCharacters)
  {
    const BidirectionalInfo* const bidirectionalInfo = *(mParagraphBidirectionalInfo.Begin() + bidiInfoIndex);

    for(CharacterIndex index = 0u; index < numberOfCharacters; ++index)
    {
      CharacterDirection& characterDirection = *(directions + index);

      // Checks if the character is rtl oriented.
      // I.e even a neutral character can become rtl if surrounded by rtl characters.
      characterDirection = FRIBIDI_IS_RTL(*(bidirectionalInfo->embeddedLevels + index));

      // NOTE
      // We are discontinuing the previous character direction determination logic.
      // The previous logic was too heuristic and had many shortcomings in handling various RTL cases.
      // The key change in this update is that character direction is determined based on embedding levels,
      // including bracket information.
      // The character direction determined here will affect the behavior of the GetMirroredText() function.
      // BTW, Harfbuzz(hb_shape) also supports text mirroring.
      // To use this, we need to pass the character direction at the embedding level to hb_buffer_set_direction,
      // which is currently challenging for us.
      // If this is implemented, we will no longer need to perform GetMirroredText().
    }
  }

  Vector<BidirectionalInfo*> mParagraphBidirectionalInfo; ///< Stores the bidirectional info per paragraph.
  Vector<BidiInfoIndex>      mFreeIndices;                ///< Stores indices of free positions in the bidirectional info vector.
};

BidirectionalSupport::BidirectionalSupport()
: mPlugin(NULL)
{
}

BidirectionalSupport::~BidirectionalSupport()
{
  delete mPlugin;
}

TextAbstraction::BidirectionalSupport BidirectionalSupport::Get()
{
  TextAbstraction::BidirectionalSupport bidirectionalSupportHandle;

  SingletonService service(SingletonService::Get());
  if(service)
  {
    // Check whether the singleton is already created
    BaseHandle handle = service.GetSingleton(typeid(TextAbstraction::BidirectionalSupport));
    if(handle)
    {
      // If so, downcast the handle
      BidirectionalSupport* impl = dynamic_cast<Internal::BidirectionalSupport*>(handle.GetObjectPtr());
      bidirectionalSupportHandle = TextAbstraction::BidirectionalSupport(impl);
    }
    else // create and register the object
    {
      bidirectionalSupportHandle = TextAbstraction::BidirectionalSupport(new BidirectionalSupport);
      service.Register(typeid(bidirectionalSupportHandle), bidirectionalSupportHandle);
    }
  }

  return bidirectionalSupportHandle;
}

BidiInfoIndex BidirectionalSupport::CreateInfo(const Character* const      paragraph,
                                               Length                      numberOfCharacters,
                                               bool                        matchLayoutDirection,
                                               Dali::LayoutDirection::Type layoutDirection)
{
  CreatePlugin();

  return mPlugin->CreateInfo(paragraph,
                             numberOfCharacters,
                             matchLayoutDirection,
                             layoutDirection);
}

void BidirectionalSupport::DestroyInfo(BidiInfoIndex bidiInfoIndex)
{
  CreatePlugin();

  mPlugin->DestroyInfo(bidiInfoIndex);
}

void BidirectionalSupport::Reorder(BidiInfoIndex   bidiInfoIndex,
                                   CharacterIndex  firstCharacterIndex,
                                   Length          numberOfCharacters,
                                   CharacterIndex* visualToLogicalMap)
{
  CreatePlugin();

  mPlugin->Reorder(bidiInfoIndex,
                   firstCharacterIndex,
                   numberOfCharacters,
                   visualToLogicalMap);
}

bool BidirectionalSupport::GetMirroredText(Character*          text,
                                           CharacterDirection* directions,
                                           Length              numberOfCharacters)
{
  CreatePlugin();

  return mPlugin->GetMirroredText(text, directions, numberOfCharacters);
}

bool BidirectionalSupport::GetParagraphDirection(BidiInfoIndex bidiInfoIndex) const
{
  if(!mPlugin)
  {
    return false;
  }

  return mPlugin->GetParagraphDirection(bidiInfoIndex);
}

void BidirectionalSupport::GetCharactersDirection(BidiInfoIndex       bidiInfoIndex,
                                                  CharacterDirection* directions,
                                                  Length              numberOfCharacters)
{
  CreatePlugin();

  mPlugin->GetCharactersDirection(bidiInfoIndex,
                                  directions,
                                  numberOfCharacters);
}

void BidirectionalSupport::CreatePlugin()
{
  if(!mPlugin)
  {
    mPlugin = new Plugin();
  }
}

} // namespace Internal

} // namespace TextAbstraction

} // namespace Dali
