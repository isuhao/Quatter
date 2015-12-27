#ifndef INPUTMASTER_H
#define INPUTMASTER_H

#include <Urho3D/Urho3D.h>
#include "mastercontrol.h"

namespace Urho3D {
class Drawable;
class Node;
class Scene;
class Sprite;
}

using namespace Urho3D;

enum class JoystickButton {SELECT, LEFTSTICK, RIGHTSTICK, START, DPAD_UP, DPAD_RIGHT, DPAD_DOWN, DPAD_LEFT, L2, R2, L1, R1, TRIANGLE, CIRCLE, CROSS, SQUARE};

class InputMaster : public Object
{
    URHO3D_OBJECT(InputMaster, Object);
public:
    InputMaster(Context* context, MasterControl* masterControl);
    WeakPtr<Node> firstHit_;
private:
    MasterControl* masterControl_;
    Input* input_;

    HashSet<int> pressedKeys_;
    HashSet<int> pressedMouseButtons_;
    HashSet<int> pressedJoystickButtons_;

    void HandleKeyDown(StringHash eventType, VariantMap &eventData);
    void HandleKeyUp(StringHash eventType, VariantMap &eventData);
    void HandleMouseButtonDown(StringHash eventType, VariantMap &eventData);
    void HandleMouseButtonUp(StringHash eventType, VariantMap &eventData);
    void HandleJoystickButtonDown(StringHash eventType, VariantMap &eventData);
    void HandleJoystickButtonUp(StringHash eventType, VariantMap &eventData);
};

#endif // INPUTMASTER_H
