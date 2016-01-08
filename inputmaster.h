#ifndef INPUTMASTER_H
#define INPUTMASTER_H

#include "master.h"

enum class JoystickButton {SELECT, LEFTSTICK, RIGHTSTICK, START, DPAD_UP, DPAD_RIGHT, DPAD_DOWN, DPAD_LEFT, L2, R2, L1, R1, TRIANGLE, CIRCLE, CROSS, SQUARE};

class InputMaster : public Master
{
    URHO3D_OBJECT(InputMaster, Master);
public:
    InputMaster(Context* context, MasterControl* masterControl);
    WeakPtr<Node> firstHit_;
private:
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
    void HandleUpdate(StringHash eventType, VariantMap &eventData);
};

#endif // INPUTMASTER_H
