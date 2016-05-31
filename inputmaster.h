/* Quatter
// Copyright (C) 2016 LucKey Productions (luckeyproductions.nl)
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

#define YAD_FULLBRIGHT 0.5f
#define YAD_DIMMED 0.1f

class Square;

typedef class Yad : public Object{
    URHO3D_OBJECT(Yad, Object);
public:
    Yad() : Object(MC->GetContext()) {}
    SharedPtr<Node> node_;
    SharedPtr<AnimatedModel> model_;
    SharedPtr<Material> material_;
    SharedPtr<Light> light_;
    bool hidden_{true};
}Yad;

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
    bool MultipleJoysticks();
    Ray MouseRay();


    void ConstructYad();
    void HideYad();
    void RevealYad();
private:
    Input* input_;

    Vector2 mousePos_;

    float idleTime_;
    bool idle_;
    float mouseIdleTime_;

    Vector2 smoothCamRotate_;
    float smoothCamZoom_;

    HashSet<int> pressedKeys_;
    HashSet<int> pressedMouseButtons_;
    HashMap<int, HashSet<int>> pressedJoystickButtons_;
    float sinceStep_;
    bool actionDone_;

    Yad* yad_;
    Piece* RaycastToPiece();
    Square* RaycastToSquare();
    Piece* rayPiece_;
    Square* raySquare_;

    void HandleUpdate(StringHash eventType, VariantMap &eventData);
    void UpdateMousePos(bool delta);
    void UpdateYad();
    Vector3 YadRaycast();

    void HandleKeyDown(StringHash eventType, VariantMap &eventData);
    void HandleKeyUp(StringHash eventType, VariantMap &eventData);
    void HandleKeys();

    void HandleMouseMove(StringHash eventType, VariantMap& eventData);
    void HandleMouseButtonDown(StringHash eventType, VariantMap &eventData);
    void HandleMouseButtonUp(StringHash eventType, VariantMap &eventData);

    void HandleJoystickButtonDown(StringHash eventType, VariantMap &eventData);
    void HandleJoystickButtonUp(StringHash eventType, VariantMap &eventData);
    void HandleJoystickButtons();

    void SmoothCameraMovement(Vector2 camRot, float camZoom);
    void HandleCameraMovement(float t);

    void Screenshot();
    void HandleActionButtonPressed();
    void Step(Vector3 step);
    void HandleDownArrowPressed();
    void HandleRightArrowPressed();
    void HandleLeftArrowPressed();
    bool CorrectJoystickId(int joystickId);
    void DimYad();
    void RestoreYad();
};

#endif // INPUTMASTER_H
