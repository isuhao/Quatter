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

#include "inputmaster.h"
#include "quattercam.h"
#include "board.h"
#include "piece.h"

InputMaster::InputMaster() : Master(),
    input_{GetSubsystem<Input>()},
    idle_{false},
    sinceStep_{STEP_INTERVAL},
    actionDone_{false}
{
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleMouseButtonDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(InputMaster, HandleMouseButtonUp));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(InputMaster, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(InputMaster, HandleKeyUp));
    SubscribeToEvent(E_JOYSTICKBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleJoystickButtonDown));
    SubscribeToEvent(E_JOYSTICKBUTTONUP, URHO3D_HANDLER(InputMaster, HandleJoystickButtonUp));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(InputMaster, HandleUpdate));
}


void InputMaster::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    float t{eventData[Update::P_TIMESTEP].GetFloat()};
    idleTime_ += t;
    sinceStep_ += t;

    if (idleTime_ > IDLE_THRESHOLD)
        SetIdle();

    HandleCameraMovement(t);

    HandleKeys();
    HandleJoystickButtons();
}

void InputMaster::HandleKeys()
{
    for (int key: pressedKeys_){
        switch (key){
        case KEY_SPACE:{
            HandleActionButtonPressed();
        } break;
        case KEY_UP:{ Step(Vector3::UP);
        } break;
        case KEY_DOWN:{ Step(Vector3::DOWN);
        } break;
        case KEY_RIGHT:{ Step(Vector3::RIGHT);
        } break;
        case KEY_LEFT:{ Step(Vector3::LEFT);
        } break;
        default: break;
        }
    }
}
void InputMaster::HandleKeyDown(StringHash eventType, VariantMap &eventData)
{
    ResetIdle();

    int key{eventData[KeyDown::P_KEY].GetInt()};
    pressedKeys_.Insert(key);

    switch (key){
    case KEY_ESC:{
        if (!BOARD->IsEmpty() || MC->GetSelectedPiece() || MC->GetPickedPiece()){
            MC->Reset();
        } else MC->Exit();
    } break;
    case KEY_9:{
        MC->TakeScreenshot();
    } break;
    case KEY_M: {
        MC->NextMusicState();
    } break;
    case KEY_KP_PLUS: {
        MC->MusicGainUp(VOLUME_STEP);
    } break;
    case KEY_KP_MINUS: {
        MC->MusicGainDown(VOLUME_STEP);
    } break;
    default: break;
    }
}
void InputMaster::HandleKeyUp(StringHash eventType, VariantMap &eventData)
{
    ResetIdle();

    using namespace KeyUp;
    int key{eventData[P_KEY].GetInt()};

    if (key == KEY_SPACE) actionDone_ = false;

    if (key == KEY_UP    ||
        key == KEY_DOWN  ||
        key == KEY_LEFT  ||
        key == KEY_RIGHT)
    {
        sinceStep_ = STEP_INTERVAL;
    }

    if (pressedKeys_.Contains(key)) pressedKeys_.Erase(key);
}

void InputMaster::Step(Vector3 step)
{
    if (sinceStep_ < STEP_INTERVAL)
        return;

    if (MC->InPickState()){

            sinceStep_ = 0.0f;
            if (step == Vector3::RIGHT || step == Vector3::UP)
                MC->StepSelectPiece(true);
            else if (step == Vector3::LEFT || step == Vector3::DOWN)
                MC->StepSelectPiece(false);

    } else if (MC->InPutState()){

        //Correct for semantics
        step = Quaternion(90.0f, Vector3::RIGHT) * step; ///Asks for Vector3& operator *=(Quaternion rhs)
        //Correct step according to view angle
        float quadrant{0.0f};
        for (float angle: {90.0f, 180.0f, 270.0f}){

            if (LucKey::Delta(CAMERA->GetYaw(), angle, true) <
                LucKey::Delta(CAMERA->GetYaw(), quadrant, true))
            {
                quadrant = angle;
            }

        }
        Vector3 resultingStep{Quaternion(quadrant, Vector3::UP) * step};
        Square* selectedSquare{BOARD->GetSelectedSquare()};
        Square* lastSelectedSquare{BOARD->GetLastSelectedSquare()};
        if (selectedSquare){
            BOARD->SelectNearestSquare(selectedSquare->node_->GetPosition() + resultingStep);
            if (BOARD->GetSelectedSquare() != selectedSquare)
                sinceStep_ = 0.0f;
        } else if (lastSelectedSquare) {
            BOARD->SelectLast();
            sinceStep_ = 0.0f;
        } else {
            BOARD->SelectNearestFreeSquare();
            sinceStep_ = 0.0f;
        }
    }
}

void InputMaster::HandleMouseButtonDown(StringHash eventType, VariantMap &eventData)
{
    ResetIdle();

    using namespace MouseButtonDown;
    int button{eventData[P_BUTTON].GetInt()};
    pressedMouseButtons_.Insert(button);
}


void InputMaster::HandleMouseButtonUp(StringHash eventType, VariantMap &eventData)
{
    ResetIdle();

    using namespace MouseButtonUp;
    int button{eventData[P_BUTTON].GetInt()};
    if (pressedMouseButtons_.Contains(button)) pressedMouseButtons_.Erase(button);
}

void InputMaster::HandleJoystickButtons()
{
    for(int joystickId: {0, 1}) {
        HashSet<int>& buttons{pressedJoystickButtons_[joystickId]};

        if (buttons.Size())
            ResetIdle();

        if (buttons.Contains(LucKey::SB_START)){
            if (MC->GetGameState() == GameState::QUATTER
                || buttons.Contains(LucKey::SB_SELECT))
            {
                MC->Reset();
            }
        }

        if (!CorrectJoystickId(joystickId)) continue;

        for (int button: buttons){
            switch (button){
            case LucKey::SB_CROSS:{
                HandleActionButtonPressed();
            } break;
            case LucKey::SB_CIRCLE:{
                if (MC->InPickState())
                    MC->DeselectPiece();
                else if (MC->InPutState())
                    BOARD->Deselect();
            } break;
            case LucKey::SB_DPAD_UP:{ Step(Vector3::UP);
            } break;
            case LucKey::SB_DPAD_DOWN:{ Step(Vector3::DOWN);
            } break;
            case LucKey::SB_DPAD_RIGHT:{ Step(Vector3::RIGHT);
            } break;
            case LucKey::SB_DPAD_LEFT:{ Step(Vector3::LEFT);
            } break;
            case LucKey::SB_SELECT:{
                if (MC->InPickState())
                    MC->selectionMode_ = SM_CAMERA;
                else if (MC->InPutState())
                    BOARD->SelectNearestFreeSquare();
            } break;
            default: break;
            }
        }
    }
}
bool InputMaster::CorrectJoystickId(int joystickId)
{
    return input_->GetJoystickByIndex(joystickId) == GetActiveJoystick();
}
JoystickState* InputMaster::GetActiveJoystick()
{
    if (MultipleJoysticks()) {

        if (MC->GetGameState() == GameState::PLAYER1PICKS ||
            MC->GetGameState() == GameState::PLAYER1PUTS)
        {
            return input_->GetJoystickByIndex(0);

        } else if (MC->GetGameState() == GameState::PLAYER2PICKS ||
                   MC->GetGameState() == GameState::PLAYER2PUTS)
        {
            return input_->GetJoystickByIndex(1);
        }

    } else if (input_->GetJoystickByIndex(0)){

        return input_->GetJoystickByIndex(0);

    }
    return nullptr;
}
bool InputMaster::MultipleJoysticks()
{
    return input_->GetJoystickByIndex(0) && input_->GetJoystickByIndex(1);
}
void InputMaster::HandleJoystickButtonDown(StringHash eventType, VariantMap &eventData)
{
    using namespace JoystickButtonDown;
    int joystickId{eventData[P_JOYSTICKID].GetInt()};
    int button{eventData[P_BUTTON].GetInt()};

    if (   input_->GetJoystickByIndex(0)
        && input_->GetJoystickByIndex(1)
        && !CorrectJoystickId(joystickId)) return;

    pressedJoystickButtons_[joystickId].Insert(button);
}
void InputMaster::HandleJoystickButtonUp(StringHash eventType, VariantMap &eventData)
{
    using namespace JoystickButtonUp;
    int joystickId{eventData[P_JOYSTICKID].GetInt()};
    int button{eventData[P_BUTTON].GetInt()};

    if (CorrectJoystickId(joystickId) && button == LucKey::SB_CROSS)
        actionDone_ = false;

    if (pressedJoystickButtons_[joystickId].Contains(button))
        pressedJoystickButtons_[joystickId].Erase(button);
}

void InputMaster::HandleActionButtonPressed()
{
    if (actionDone_)
        return;
    actionDone_ = true;

    if (MC->InPickState()){

        Piece* selectedPiece{MC->GetSelectedPiece()};
        if (selectedPiece){

            selectedPiece->Pick();
            MC->NextPhase();
            BOARD->SelectNearestFreeSquare();
        } else if (MC->selectionMode_ == SM_STEP)
            MC->SelectLastPiece();
        else if (MC->selectionMode_ == SM_CAMERA)
            MC->CameraSelectPiece();

    } else if (MC->InPutState() ){

        if (BOARD->PutPiece(MC->GetPickedPiece())){
            MC->NextPhase();
        } else BOARD->Refuse();

    }
}

void InputMaster::SetIdle()
{
    if (!idle_){

        idle_ = true;

        if (MC->GetSelectedPiece())
            MC->DeselectPiece();

        Square* selectedSquare{BOARD->GetSelectedSquare()};
        if (selectedSquare)
            BOARD->Deselect(selectedSquare);
    }
}
void InputMaster::ResetIdle()
{
    if (idle_) {

        idle_ = false;
        if (MC->InPutState()){
            if (!BOARD->SelectLast())
                BOARD->SelectNearestFreeSquare();

        } else if (MC->InPickState()){
            if (!MC->SelectLastPiece())
                MC->CameraSelectPiece();
        }
    }

    idleTime_ = 0.0f;
}

void InputMaster::HandleCameraMovement(float t)
{
    Vector2 camRot{};
    float camZoom{};

    float keyRotMultiplier{0.5f};
    float keyZoomSpeed{0.1f};

    float joyRotMultiplier{80.0f};
    float joyZoomSpeed{7.0f};

    for (int key : pressedKeys_){
        switch (key){
        case KEY_A:{ camRot += keyRotMultiplier * Vector2::RIGHT;
        } break;
        case KEY_D:{ camRot += keyRotMultiplier * Vector2::LEFT;
        } break;
        case KEY_W:{ camRot += keyRotMultiplier * Vector2::UP;
        } break;
        case KEY_S:{ camRot += keyRotMultiplier * Vector2::DOWN;
        } break;
        case KEY_Q:{ camZoom -= keyZoomSpeed;
        } break;
        case KEY_E:{ camZoom +=  keyZoomSpeed;
        } break;
        default: break;
        }
    }

    //Mouse rotate
    if (pressedMouseButtons_.Contains(MOUSEB_RIGHT)){
        IntVector2 mouseMove{input_->GetMouseMove()};
        camRot += Vector2(mouseMove.x_, mouseMove.y_) * 0.1f;
    }

    JoystickState* joy{GetActiveJoystick()};
    if (joy){
        Vector2 rotation{-Vector2(joy->GetAxisPosition(0), joy->GetAxisPosition(1))
                         -Vector2(joy->GetAxisPosition(2), joy->GetAxisPosition(3))};

        if (Abs(rotation.x_) < DEADZONE) rotation.x_ = 0;
        else {
            rotation.x_ -= DEADZONE * Sign(rotation.x_);
            rotation.x_ *= 1.0f / (1.0f - DEADZONE);
        }

        if (Abs(rotation.y_) < DEADZONE) rotation.y_ = 0;
        else {
            rotation.y_ -= DEADZONE * Sign(rotation.y_);
            rotation.y_ *= 1.0f / (1.0f - DEADZONE);
        }

        if (rotation.Length()){
            ResetIdle();
            camRot += rotation * t * joyRotMultiplier;
        }

        float zoom{Clamp(joy->GetAxisPosition(13) - joy->GetAxisPosition(12),
                         -1.0f, 1.0f)};

        if (Abs(zoom) < DEADZONE){
            zoom = 0.0f;
        } else {
            zoom -= DEADZONE * Sign(zoom);
            zoom *= 1.0f / (1.0f - DEADZONE);
            zoom *= zoom * zoom;
        }
        camZoom += t * joyZoomSpeed * zoom;
    }

    //Slowly spin camera when there hasn't been any input for a while
    if (idle_){
        float idleStartup{Min(0.5f * (idleTime_ - IDLE_THRESHOLD), 1.0f)};
        camRot += Vector2(t * idleStartup * -0.5f,
                          t * idleStartup * MC->Sine(0.23f, -0.042f, 0.042f));
    }
    //Speed up camera movement when shift key is held down
    if (pressedKeys_.Contains(KEY_SHIFT)){
        camRot *= 3.0f;
        camZoom *= 2.0f;
    }

    //Slow down up and down rotation when nearing extremes
    SmoothCameraMovement(camRot, camZoom);

    CAMERA->Rotate(smoothCamRotate_);
    CAMERA->Zoom(smoothCamZoom_);
}
void InputMaster::SmoothCameraMovement(Vector2 camRot, float camZoom)
{
    float pitchBrake{1.0f};
    if (Sign(camRot.y_) > 0.0f){
        float pitchLeft{LucKey::Delta(CAMERA->GetPitch(), PITCH_MAX)};
        if (pitchLeft < PITCH_EDGE)
            pitchBrake = pitchLeft / PITCH_EDGE;
    } else {
        float pitchLeft{LucKey::Delta(CAMERA->GetPitch(), PITCH_MIN)};
        if (pitchLeft < PITCH_EDGE)
            pitchBrake = pitchLeft / PITCH_EDGE;
    }
    camRot.y_ *= pitchBrake * pitchBrake;
    //Slow down zooming when nearing extremes
    float zoomBrake{1.0f};
    if (Sign(camZoom) < 0.0f){
        float zoomLeft{LucKey::Delta(CAMERA->GetDistance(), ZOOM_MAX)};
        if (zoomLeft < ZOOM_EDGE)
            zoomBrake = zoomLeft / ZOOM_EDGE;
    } else {
        float zoomLeft{LucKey::Delta(CAMERA->GetDistance(), ZOOM_MIN)};
        if (zoomLeft < ZOOM_EDGE)
            zoomBrake = zoomLeft / ZOOM_EDGE;
    }
    camZoom *= zoomBrake;

    smoothCamRotate_ = 0.0666f * (camRot  + smoothCamRotate_ * 14.0f);
    smoothCamZoom_   = 0.05f * (camZoom + smoothCamZoom_   * 19.0f);
}
