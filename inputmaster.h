#ifndef INPUTMASTER_H
#define INPUTMASTER_H

#include "master.h"

enum JoystickButton {SELECT, LEFTSTICK, RIGHTSTICK, JB_START, JB_DPAD_UP, JB_DPAD_RIGHT, JB_DPAD_DOWN, JB_DPAD_LEFT, L2, R2, L1, R1, TRIANGLE, CIRCLE, JB_CROSS, SQUARE};

#define IDLE_THRESHOLD 5.0f

class InputMaster : public Master
{
    URHO3D_OBJECT(InputMaster, Master);
public:
    InputMaster();
    WeakPtr<Node> firstHit_;

    bool IsIdle() const noexcept { return idle_; }
    void ResetIdle();

    void SetIdle();

    JoystickState* GetActiveJoystick();
private:
    Input* input_;

    float idleTime_;
    bool idle_;
    Vector2 smoothCamRotate_;
    float smoothCamZoom_;

    HashSet<int> pressedKeys_;
    HashSet<int> pressedMouseButtons_;
    HashMap<int, HashSet<int>> pressedJoystickButtons_;

    void HandleKeyDown(StringHash eventType, VariantMap &eventData);
    void HandleKeyUp(StringHash eventType, VariantMap &eventData);
    void HandleMouseButtonDown(StringHash eventType, VariantMap &eventData);
    void HandleMouseButtonUp(StringHash eventType, VariantMap &eventData);
    void HandleJoystickButtonDown(StringHash eventType, VariantMap &eventData);
    void HandleJoystickButtonUp(StringHash eventType, VariantMap &eventData);
    void HandleUpdate(StringHash eventType, VariantMap &eventData);
    void SmoothCameraMovement(float camZoom, Vector2 camRot);
    void HandleCameraMovement(float t);

    void Screenshot();
    void HandleActionButtonPressed();
    void Step(Vector3 step);
    void HandleDownArrowPressed();
    void HandleRightArrowPressed();
    void HandleLeftArrowPressed();
    bool CorrectTurn(int joystickId);
};

#endif // INPUTMASTER_H
