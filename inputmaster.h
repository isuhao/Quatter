/* Quatter
// Copyright (C) 2017 LucKey Productions (luckeyproductions.nl)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef INPUTMASTER_H
#define INPUTMASTER_H

#include "master.h"

#define VOLUME_STEP 0.1f
#define IDLE_THRESHOLD 5.0f
#define STEP_INTERVAL 0.23f
#define DEADZONE 0.34f
#define MOUSESPEED 0.23f

#define DRAG_THRESHOLD 0.023f

class Square;
class Yad;

class InputMaster : public Master
{
    URHO3D_OBJECT(InputMaster, Master);

public:
    InputMaster(Context* context);
    bool IsIdle() const noexcept { return idle_; }
    void ConstructYad();
    void UpdateYad();

private:
    HashSet<int> pressedKeys_;
    HashSet<int> pressedMouseButtons_;
    HashMap<int, HashSet<int>> pressedJoystickButtons_;

    bool idle_;
    bool drag_;
    bool actionDone_;
    bool boardClick_;
    bool tableClick_;
    float sinceStep_;
    float idleTime_;
    float mouseIdleTime_;
    Vector2 mousePos_;
    Vector2 mouseMoveSinceClick_;
    Vector2 smoothCamRotate_;
    float smoothCamZoom_;

    Yad* yad_;
    Piece* rayPiece_;
    Square* raySquare_;

    void ResetIdle();
    void SetIdle();
    Ray MouseRay();

    Piece* RaycastToPiece();
    Square* RaycastToSquare();
    bool RaycastToBoard();
    bool RaycastToTable();

    void HandleUpdate(StringHash eventType, VariantMap &eventData);
    void UpdateMousePos();
    Vector3 YadRaycast(bool& none);

    void HandleKeyDown(StringHash eventType, VariantMap &eventData);
    void HandleKeyUp(StringHash eventType, VariantMap &eventData);
    void HandleKeys();

    void HandleMouseMove(StringHash eventType, VariantMap& eventData);
    void HandleMouseButtonDown(StringHash eventType, VariantMap &eventData);
    void HandleMouseButtonUp(StringHash eventType, VariantMap &eventData);
    void HandleMouseWheel(StringHash eventType, VariantMap& eventData);

    JoystickState* GetActiveJoystick();
    void HandleJoystickButtonDown(StringHash eventType, VariantMap &eventData);
    void HandleJoystickButtonUp(StringHash eventType, VariantMap &eventData);
    void HandleJoystickButtons();

    void SmoothCameraMovement(Vector2 camRot, float camZoom);
    void HandleCameraMovement(float timeStep);

    void Screenshot();
    void ActionButtonPressed();
    void Step(Vector3 step);
    void HandleDownArrowPressed();
    void HandleRightArrowPressed();
    void HandleLeftArrowPressed();
    bool CorrectJoystickId(int joystickId);
    void SelectionButtonPressed();
    void AnyKey();
};

#endif // INPUTMASTER_H
