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

#pragma once

#include "KeySignatureEvent.h"
#include "ProjectListener.h"

class HybridRoll;
class ProjectTreeItem;
class TrackStartIndicator;
class TrackEndIndicator;

template< typename T >
class KeySignaturesTrackMap :
    public Component,
    public ProjectListener
{
public:

    KeySignaturesTrackMap(ProjectTreeItem &parentProject, HybridRoll &parentRoll);
    ~KeySignaturesTrackMap() override;

    void alignKeySignatureComponent(T *nc);

    //===------------------------------------------------------------------===//
    // Component
    //===------------------------------------------------------------------===//

    void resized() override;

    //===------------------------------------------------------------------===//
    // ProjectListener
    //===------------------------------------------------------------------===//

    void onChangeMidiEvent(const MidiEvent &oldEvent,
        const MidiEvent &newEvent) override;
    void onAddMidiEvent(const MidiEvent &event) override;
    void onRemoveMidiEvent(const MidiEvent &event) override;

    void onAddTrack(MidiTrack *const track) override;
    void onRemoveTrack(MidiTrack *const track) override;
    void onChangeTrackProperties(MidiTrack *const track) override;
    void onResetTrackContent(MidiTrack *const track) override;

    void onChangeProjectBeatRange(float firstBeat, float lastBeat) override;
    void onChangeViewBeatRange(float firstBeat, float lastBeat) override;

    //===------------------------------------------------------------------===//
    // Stuff for children
    //===------------------------------------------------------------------===//

    void onKeySignatureMoved(T *nc);
    void onKeySignatureTapped(T *nc);
    void showContextMenuFor(T *nc);
    void alternateActionFor(T *nc);
    float getBeatByXPosition(int x) const;
    
private:
    
    void reloadTrackMap();
    void applyKeySignatureBounds(T *nc, T *nextOne = nullptr);
    
    T *getPreviousEventComponent(int indexOfSorted) const;
    T *getNextEventComponent(int indexOfSorted) const;
    
    void updateTrackRangeIndicatorsAnchors();
    
private:
    
    float projectFirstBeat;
    float projectLastBeat;

    float rollFirstBeat;
    float rollLastBeat;
    
    HybridRoll &roll;
    ProjectTreeItem &project;
    
    ScopedPointer<TrackStartIndicator> trackStartIndicator;
    ScopedPointer<TrackEndIndicator> trackEndIndicator;
    
    ComponentAnimator animator;

    OwnedArray<T> keySignatureComponents;
    HashMap<KeySignatureEvent, T *, KeySignatureEventHashFunction> keySignaturesHash;
    
};

