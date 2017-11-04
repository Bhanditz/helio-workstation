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

//[Headers]
#include "Common.h"
#include "KeySignaturesSequence.h"
//[/Headers]

#include "KeySignatureLargeComponent.h"

//[MiscUserDefs]
//[/MiscUserDefs]

KeySignatureLargeComponent::KeySignatureLargeComponent(KeySignaturesTrackMap<KeySignatureLargeComponent> &parent, const KeySignatureEvent &targetEvent)
    : event(targetEvent),
      editor(parent),
      anchor(targetEvent),
      key(KEY_C5),
      mouseDownWasTriggered(false)
{

    //[UserPreSize]
    this->setOpaque(false);
    this->setInterceptsMouseClicks(true, false);
    this->setMouseCursor(MouseCursor::PointingHandCursor);
    //[/UserPreSize]

    setSize (128, 32);

    //[Constructor]
    //[/Constructor]
}

KeySignatureLargeComponent::~KeySignatureLargeComponent()
{
    //[Destructor_pre]
    //[/Destructor_pre]


    //[Destructor]
    //[/Destructor]
}

void KeySignatureLargeComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    {
        float x = 0, y = 0;
        Colour fillColour = Colour (0x25fefefe);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillPath (internalPath1, AffineTransform::translation(x, y));
    }

    //[UserPaint] Add your own custom painting code here..
    const Font labelFont(19.00f, Font::plain);
    g.setColour(Colour(0x88ffffff));

    GlyphArrangement arr;

    // TODO draw key and scale

    //arr.addFittedText(labelFont,
    //                  String(this->numerator),
    //                  this->boundsOffset.getX() + 4.f,
    //                  -3.f,
    //                  float(this->getWidth()),
    //                  24.f,
    //                  Justification::centredLeft,
    //                  1,
    //                  0.85f);

    //arr.addFittedText(labelFont,
    //                  String(this->denominator),
    //                  this->boundsOffset.getX() + 4.f,
    //                  11.f,
    //                  float(this->getWidth()),
    //                  24.f,
    //                  Justification::centredLeft,
    //                  1,
    //                  0.85f);
    arr.draw(g);

    //[/UserPaint]
}

void KeySignatureLargeComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    internalPath1.clear();
    internalPath1.startNewSubPath (0.0f, 0.0f);
    internalPath1.lineTo (5.0f, 0.0f);
    internalPath1.lineTo (1.0f, 5.0f);
    internalPath1.lineTo (0.0f, 6.0f);
    internalPath1.closeSubPath();

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void KeySignatureLargeComponent::mouseMove (const MouseEvent& e)
{
    //[UserCode_mouseMove] -- Add your code here...
    //[/UserCode_mouseMove]
}

void KeySignatureLargeComponent::mouseDown (const MouseEvent& e)
{
    //[UserCode_mouseDown] -- Add your code here...
    //Logger::writeToLog("KeySignatureEventComponent::mouseDown");
    this->mouseDownWasTriggered = true;

    if (e.mods.isLeftButtonDown())
    {
        // don't checkpoint right here, but only before the actual change
        //this->event.getSequence()->checkpoint();

        this->dragger.startDraggingComponent(this, e);
        this->draggingHadCheckpoint = false;
        this->draggingState = true;
        this->anchor = this->event;
    }
    else
    {
        this->editor.alternateActionFor(this);
        //this->editor.showContextMenuFor(this);
    }
    //[/UserCode_mouseDown]
}

void KeySignatureLargeComponent::mouseDrag (const MouseEvent& e)
{
    //[UserCode_mouseDrag] -- Add your code here...
    if (e.mods.isLeftButtonDown() && e.getDistanceFromDragStart() > 4)
    {
        if (this->draggingState)
        {
            this->setMouseCursor(MouseCursor::DraggingHandCursor);
            this->dragger.dragComponent(this, e, nullptr);
            const float newBeat = this->editor.getBeatByXPosition(this->getX());
            const bool beatHasChanged = (this->event.getBeat() != newBeat);

            if (beatHasChanged)
            {
                if (! this->draggingHadCheckpoint)
                {
                    this->event.getSequence()->checkpoint();
                    this->draggingHadCheckpoint = true;
                }

                Array<KeySignatureEvent> groupDragBefore, groupDragAfter;
                groupDragBefore.add(this->event);
                groupDragAfter.add(this->event.withBeat(newBeat));
                KeySignaturesSequence *autoLayer = static_cast<KeySignaturesSequence *>(this->event.getSequence());
                autoLayer->changeGroup(groupDragBefore, groupDragAfter, true);
            }

            this->editor.alignKeySignatureComponent(this);
        }
    }
    //[/UserCode_mouseDrag]
}

void KeySignatureLargeComponent::mouseUp (const MouseEvent& e)
{
    //[UserCode_mouseUp] -- Add your code here...
    if (e.mods.isLeftButtonDown())
    {
        if (this->draggingState)
        {
            this->setMouseCursor(MouseCursor::PointingHandCursor);
            this->draggingState = false;
            this->editor.onKeySignatureMoved(this);
        }

        if (e.getDistanceFromDragStart() < 10 &&
            this->mouseDownWasTriggered &&
            !this->draggingHadCheckpoint)
        {
            this->editor.onKeySignatureTapped(this);
        }
    }

    this->mouseDownWasTriggered = false;
    //[/UserCode_mouseUp]
}

void KeySignatureLargeComponent::mouseDoubleClick (const MouseEvent& e)
{
    //[UserCode_mouseDoubleClick] -- Add your code here...
    //Logger::writeToLog("KeySignatureLargeComponent::mouseDoubleClick");
    //[/UserCode_mouseDoubleClick]
}


//[MiscUserCode]

const KeySignatureEvent &KeySignatureLargeComponent::getEvent() const
{
    return this->event;
}

void KeySignatureLargeComponent::setRealBounds(const Rectangle<float> bounds)
{
    Rectangle<int> intBounds(bounds.toType<int>());
    this->boundsOffset = Rectangle<float>(bounds.getX() - float(intBounds.getX()),
                                          bounds.getY(),
                                          bounds.getWidth() - float(intBounds.getWidth()),
                                          bounds.getHeight());

    this->setBounds(intBounds);
}

float KeySignatureLargeComponent::getBeat() const
{
    return this->event.getBeat();
}

void KeySignatureLargeComponent::updateContent()
{
    if (this->key != this->event.getRootKey() ||
        this->scale != this->event.getScale())
    {
        this->key = this->event.getRootKey();
        this->scale = this->event.getScale();
        this->repaint();
    }
}

//[/MiscUserCode]

#if 0
/*
BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="KeySignatureLargeComponent"
                 template="../../../Template" componentName="" parentClasses="public Component"
                 constructorParams="KeySignaturesTrackMap&lt;KeySignatureLargeComponent&gt; &amp;parent, const KeySignatureEvent &amp;targetEvent"
                 variableInitialisers="event(targetEvent),&#10;editor(parent),&#10;anchor(targetEvent),&#10;numerator(0),&#10;denominator(0),&#10;mouseDownWasTriggered(false)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="128" initialHeight="32">
  <METHODS>
    <METHOD name="mouseDown (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDrag (const MouseEvent&amp; e)"/>
    <METHOD name="mouseUp (const MouseEvent&amp; e)"/>
    <METHOD name="mouseMove (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDoubleClick (const MouseEvent&amp; e)"/>
  </METHODS>
  <BACKGROUND backgroundColour="0">
    <PATH pos="0 0 100 100" fill="solid: 25fefefe" hasStroke="0" nonZeroWinding="1">s 0 0 l 5 0 l 1 5 l 0 6 x</PATH>
  </BACKGROUND>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
