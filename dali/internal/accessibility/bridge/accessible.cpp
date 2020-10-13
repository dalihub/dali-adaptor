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

 // CLASS HEADER

 //INTERNAL INCLUDES
#include <dali/internal/accessibility/bridge/accessibility-common.h>
#include <third-party/libunibreak/wordbreak.h>
#include <third-party/libunibreak/linebreak.h>

using namespace Dali::Accessibility;

std::vector< std::string > Accessible::GetInterfaces()
{
    std::vector< std::string > tmp;
    tmp.push_back(AtspiDbusInterfaceAccessible);
    if (dynamic_cast<Collection*>(this))
    {
        tmp.push_back(AtspiDbusInterfaceCollection);
    }
    if (dynamic_cast<Text*>(this))
    {
        tmp.push_back(AtspiDbusInterfaceText);
    }
    if (dynamic_cast<EditableText*>(this))
    {
        tmp.push_back(AtspiDbusInterfaceEditableText);
    }
    if (dynamic_cast<Value*>(this))
    {
        tmp.push_back(AtspiDbusInterfaceValue);
    }
    if (dynamic_cast<Component*>(this))
    {
        tmp.push_back(AtspiDbusInterfaceComponent);
    }
    if (auto d = dynamic_cast<Action*>(this))
    {
        if (d->GetActionCount() > 0)
        {
            tmp.push_back(AtspiDbusInterfaceAction);
        }
    }
    return tmp;
}

Accessible::Accessible()
{
}

Accessible::~Accessible()
{
    auto b = bridgeData.lock();
    if (b)
        b->knownObjects.erase(this);
}

void Accessible::EmitActiveDescendantChanged(Accessible* obj, Accessible* child)
{
    if (auto b = GetBridgeData())
    {
        b->bridge->EmitActiveDescendantChanged(obj, child);
    }
}

void Accessible::EmitStateChanged(State state, int newValue1, int newValue2)
{
    if (auto b = GetBridgeData())
    {
        b->bridge->EmitStateChanged(this, state, newValue1, newValue2);
    }
}

void Accessible::EmitShowing(bool showing)
{
    if (auto b = GetBridgeData())
    {
        b->bridge->EmitStateChanged(this, State::SHOWING, showing ? 1 : 0, 0);
    }
}

void Accessible::EmitVisible(bool visible)
{
    if (auto b = GetBridgeData())
    {
        b->bridge->EmitStateChanged(this, State::VISIBLE, visible ? 1 : 0, 0);
    }
}

void Accessible::EmitHighlighted(bool set)
{
    if (auto b = GetBridgeData())
    {
        b->bridge->EmitStateChanged(this, State::HIGHLIGHTED, set ? 1 : 0, 0);
    }
}

void Accessible::EmitFocused(bool set)
{
    if (auto b = GetBridgeData()) {
        b->bridge->EmitStateChanged(this, State::FOCUSED, set ? 1 : 0, 0);
    }
}
void Accessible::EmitTextInserted(unsigned int position, unsigned int length, const std::string& content)
{
    if (auto b = GetBridgeData()) {
        b->bridge->EmitTextChanged(this, TextChangedState::INSERTED, position, length, content);
    }
}
void Accessible::EmitTextDeleted(unsigned int position, unsigned int length, const std::string& content)
{
    if (auto b = GetBridgeData()) {
        b->bridge->EmitTextChanged(this, TextChangedState::DELETED, position, length, content);
    }
}
void Accessible::EmitTextCaretMoved(unsigned int cursorPosition)
{
    if (auto b = GetBridgeData()) {
        b->bridge->EmitCaretMoved(this, cursorPosition);
    }
}
void Accessible::Emit(WindowEvent we, unsigned int detail1)
{
    if (auto b = GetBridgeData())
    {
        b->bridge->Emit(this, we, detail1);
    }
}
void Accessible::Emit(ObjectPropertyChangeEvent ev)
{
    if (auto b = GetBridgeData())
    {
        b->bridge->Emit(this, ev);
    }
}

void Accessible::EmitBoundsChanged(Rect<> rect)
{
    if (auto b = GetBridgeData())
    {
        b->bridge->EmitBoundsChanged(this, rect);
    }
}

std::vector< Accessible* > Accessible::GetChildren()
{
    std::vector< Accessible* > tmp(GetChildCount());
    for (auto i = 0u; i < tmp.size(); ++i)
    {
        tmp[i] = GetChildAtIndex(i);
    }
    return tmp;
}

std::shared_ptr< Bridge::Data > Accessible::GetBridgeData()
{
    auto b = bridgeData.lock();
    if (!b)
    {
        auto p = Bridge::GetCurrentBridge();
        b = p->data;
    }
    return b;
}

Address Accessible::GetAddress()
{
    auto b = bridgeData.lock();
    if (!b)
    {
        b = GetBridgeData();
        if (b)
            b->bridge->RegisterOnBridge(this);
    }
    std::ostringstream tmp;
    tmp << this;
    return { b ? b->busName : "", tmp.str() };
}

void Bridge::RegisterOnBridge(Accessible* obj)
{
    assert(!obj->bridgeData.lock() || obj->bridgeData.lock() == data);
    if (!obj->bridgeData.lock())
    {
        assert(data);
        data->knownObjects.insert(obj);
        obj->bridgeData = data;
    }
}

bool Accessible::IsProxy()
{
    return false;
}

Accessible* Accessible::GetDefaultLabel()
{
    return this;
}

void Accessible::NotifyAccessibilityStateChange(Dali::Accessibility::States states, bool doRecursive)
{
    if (auto b = GetBridgeData())
    {
        auto s = GetStates() & states;
        for (auto i = 0u; i < s.size(); i++)
        {
            auto index = static_cast<Dali::Accessibility::State>(i);
            if (s[index])
                b->bridge->EmitStateChanged(this, index, 1, 0);
        }
        if (doRecursive)
        {
            auto children = GetChildren();
            for (auto c : children)
                c->NotifyAccessibilityStateChange(states, doRecursive);
        }
    }
}

void Accessible::FindWordSeparationsUtf8(const utf8_t* s, size_t length, const char* language, char* breaks)
{
    set_wordbreaks_utf8(s, length, language, breaks);
}

void Accessible::FindLineSeparationsUtf8(const utf8_t* s, size_t length, const char* language, char* breaks)
{
    set_linebreaks_utf8(s, length, language, breaks);
}
