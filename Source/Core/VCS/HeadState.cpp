/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "HeadState.h"
#include "Head.h"
#include "Diff.h"
#include "DiffLogic.h"

using namespace VCS;

HeadState::HeadState() {}

HeadState::HeadState(const HeadState &other) :
    items(other.items) {}

HeadState::HeadState(const HeadState *other) :
    items(other->items) {}

void HeadState::addItem(RevisionItem::Ptr item)
{
    RevisionItem::Ptr ownItem = this->getItemWithSameUuid(item);

    if (ownItem == nullptr)
    {
        this->items.addIfNotAlreadyThere(item);
    }
    else
    {
        // ситуация, когда в состоянии есть removed запись, которую нужно заменить на added
        this->items.removeAllInstancesOf(ownItem);
        this->items.addIfNotAlreadyThere(item);
    }
}

void HeadState::removeItem(RevisionItem::Ptr item)
{
    this->items.removeAllInstancesOf(item);

    RevisionItem::Ptr ownItem = this->getItemWithSameUuid(item);

    if (ownItem != nullptr)
    {
        this->items.removeAllInstancesOf(ownItem);
    }

    // removed-запись
    this->items.addIfNotAlreadyThere(item);
}

void HeadState::mergeItem(RevisionItem::Ptr newItem)
{
    RevisionItem::Ptr stateItem = this->getItemWithSameUuid(newItem);

    if (stateItem != nullptr) // есть куда мержить
    {
        ScopedPointer<Diff> diff(newItem->getDiffLogic()->createMergedItem(*stateItem));

        if (diff->hasAnyChanges())
        {
            RevisionItem::Ptr mergedItem(new RevisionItem(stateItem->getPackPtr(), stateItem->getType(), diff));
            this->items.removeAllInstancesOf(stateItem);
            this->items.add(mergedItem);
        }
    }
    else
    {
        Logger::writeToLog("Cannot merge revision item on nothing!");
        //this->items.addIfNotAlreadyThere(newItem);
    }
}


//===----------------------------------------------------------------------===//
// TrackedItemsSource
//===----------------------------------------------------------------------===//

int HeadState::getNumTrackedItems()
{
    return this->items.size();
}

TrackedItem *HeadState::getTrackedItem(int index)
{
    return this->items[index].get();
}

RevisionItem::Ptr HeadState::getItemWithSameUuid(RevisionItem::Ptr item) const
{
    return this->getItemWithUuid(item->getUuid());
}

RevisionItem::Ptr HeadState::getItemWithUuid(const Uuid &uuid) const
{
    for (auto && item : this->items)
    {
        if (item->getUuid() == uuid)
        {
            return item;
        }
    }

    return nullptr;
}
