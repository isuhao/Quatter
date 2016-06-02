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
#include "effectmaster.h"
#include "quattercam.h"
#include "board.h"
#include "piece.h"

InputMaster::InputMaster() : Master(),
    input_{GetSubsystem<Input>()},
    idleTime_{IDLE_THRESHOLD},
    idle_{false},
    mousePos_{},
    mouseMoveSinceClick_{},
    drag_{false},
    mouseIdleTime_{0.0f},
    boardClick_{false},
    tableClick_{false},
    sinceStep_{STEP_INTERVAL},
    actionDone_{false},
    yad_{new Yad()},
    rayPiece_{},
    raySquare_{}
{
    input_->SetMouseMode(MM_FREE);
    UpdateMousePos(false);
    SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(InputMaster, HandleMouseMove));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleMouseButtonDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(InputMaster, HandleMouseButtonUp));
    SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(InputMaster, HandleMouseWheel));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(InputMaster, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(InputMaster, HandleKeyUp));
    SubscribeToEvent(E_JOYSTICKBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleJoystickButtonDown));
    SubscribeToEvent(E_JOYSTICKBUTTONUP, URHO3D_HANDLER(InputMaster, HandleJoystickButtonUp));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(InputMaster, HandleUpdate));
}

void InputMaster::ConstructYad()
{
    yad_->node_ = MC->world_.scene_->CreateChild("Yad");
    StringVector tag{}; tag.Push(String("Yad"));
    yad_->node_->SetTags(tag);
    Node* lightNode{yad_->node_->CreateChild("Light")};
    lightNode->SetPosition(Vector3::UP * 0.23f);
    yad_->model_ = yad_->node_->CreateComponent<AnimatedModel>();
    yad_->model_->SetModel(MC->GetModel("Yad"));
    yad_->material_ = MC->GetMaterial("Glow")->Clone();
    yad_->model_->SetMaterial(yad_->material_);
    yad_->light_ = lightNode->CreateComponent<Light>();
    yad_->light_->SetLightType(LIGHT_POINT);
    yad_->light_->SetCastShadows(true);
    yad_->light_->SetColor(COLOR_GLOW);
    yad_->light_->SetRange(1.0f);
    yad_->light_->SetBrightness(YAD_FULLBRIGHT);

    HideYad();
}

void InputMaster::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    float t{eventData[Update::P_TIMESTEP].GetFloat()};
    idleTime_ += t;
    mouseIdleTime_ += t;
    sinceStep_ += t;

    if (idleTime_ > IDLE_THRESHOLD)
        SetIdle();

    if (mouseIdleTime_ > IDLE_THRESHOLD * 0.23f)
        HideYad();

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
    case KEY_ESCAPE:{
        if (!BOARD->IsEmpty()
         || MC->GetSelectedPiece()
         || MC->GetPickedPiece())
        {
            MC->Reset();
//            SetIdle();
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

    if (key == KEY_UP
     || key == KEY_DOWN
     || key == KEY_LEFT
     || key == KEY_RIGHT)
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

void InputMaster::HandleMouseMove(StringHash eventType, VariantMap &eventData){
    UpdateMousePos(false);

    Graphics* graphics{GetSubsystem<Graphics>()};
    mousePos_.x_ += eventData[MouseMove::P_DX].GetFloat() / graphics->GetWidth();
    mousePos_.y_ += eventData[MouseMove::P_DY].GetFloat() / graphics->GetHeight();

    mouseMoveSinceClick_.x_ += eventData[MouseMove::P_DX].GetFloat() / graphics->GetWidth();
    mouseMoveSinceClick_.y_ += eventData[MouseMove::P_DY].GetFloat() / graphics->GetHeight();
    if (!drag_
     && mouseMoveSinceClick_.Length() > DRAG_THRESHOLD
     && (pressedMouseButtons_.Contains(MOUSEB_LEFT)
      || pressedMouseButtons_.Contains(MOUSEB_MIDDLE)
      || pressedMouseButtons_.Contains(MOUSEB_RIGHT)))
    {
        drag_ = true;
        HideYad();
    }

    MC->SetSelectionMode(SM_YAD);
    UpdateYad();

    mouseIdleTime_ = 0.0f;
    ResetIdle();
}

void InputMaster::HandleMouseButtonDown(StringHash eventType, VariantMap &eventData)
{
    using namespace MouseButtonDown;
    int button{eventData[P_BUTTON].GetInt()};
    pressedMouseButtons_.Insert(button);
    mouseMoveSinceClick_ = Vector2::ZERO;

    MC->SetSelectionMode(SM_YAD);
    UpdateMousePos(false);

    boardClick_ = RaycastToBoard();
    tableClick_ = RaycastToTable();

    UpdateYad();
    mouseIdleTime_ = 0.0f;
    ResetIdle();
}
void InputMaster::HandleMouseButtonUp(StringHash eventType, VariantMap &eventData)
{
    using namespace MouseButtonUp;
    int button{eventData[P_BUTTON].GetInt()};
    if (pressedMouseButtons_.Contains(button))
        pressedMouseButtons_.Erase(button);

    MC->SetSelectionMode(SM_YAD);
    UpdateMousePos(false);

    UpdateYad();
    mouseIdleTime_ = 0.0f;
    ResetIdle();

    if (!drag_){
        if (MC->InPickState() && MC->GetSelectedPiece()){
            MC->GetSelectedPiece()->Pick();
            RestoreYad();
        } else if (MC->InPutState() && BOARD->GetSelectedSquare()){
            MC->GetPickedPiece()->Put(BOARD->GetSelectedSquare());
            RestoreYad();
        //Zoom
        } else if (boardClick_ && RaycastToBoard()){
            CAMERA->ZoomToBoard();
        } else if (tableClick_ && RaycastToTable()){
            CAMERA->ZoomToTable();
        }
    }
    drag_ = false;
}
void InputMaster::HandleMouseWheel(StringHash eventType, VariantMap &eventData)
{
    CAMERA->SetDistance(CAMERA->GetDistance() - eventData[MouseWheel::P_WHEEL].GetInt() * 2.3f);
}
void InputMaster::UpdateYad()
{
    bool hide{false};
    Vector3 yadPos{YadRaycast(hide)};
    if (yadPos.Length())
        yad_->node_->SetPosition(Vector3(0.5f * (yadPos.x_ + yad_->node_->GetPosition().x_),
                                         yadPos.y_,
                                         0.5f * (yadPos.z_ + yad_->node_->GetPosition().z_)));
    if (!yad_->hidden_
     && (mouseIdleTime_ > IDLE_THRESHOLD * 0.5f
     || hide))
    {
        HideYad();
    }
}
void InputMaster::HideYad()
{
    yad_->hidden_ = true;
    FX->FadeOut(yad_->light_);
    FX->FadeOut(yad_->material_, 0.1f);
}
void InputMaster::RevealYad()
{
    yad_->hidden_ = false;
    RestoreYad();
}
Vector3 InputMaster::YadRaycast(bool& none)
{
    bool square{false};
    if (!drag_){
        //Select piece and hide yad when hovering over a piece in a pick state
        if (MC->InPickState()
                && RaycastToPiece()
                && (rayPiece_->GetState() == PieceState::FREE
                    || rayPiece_->GetState() == PieceState::SELECTED)){
            if (MC->InPickState()){
                MC->SelectPiece(rayPiece_);
                if (!yad_->hidden_)
                    HideYad();
                return yad_->node_->GetPosition(); //return
            }
            //Select square and dim yad when hovering over a square in a put state
        } else if (MC->InPutState() && RaycastToSquare()){
            square = true;
            if (MC->InPutState()){
                BOARD->Select(raySquare_);
                if (yad_->hidden_)
                    RevealYad();
                else if (!yad_->dimmed_)
                    DimYad();
            }
        }
    }

    Ray cameraRay{MouseRay()};
    PODVector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 1000.0f, DRAWABLE_GEOMETRY);
    MC->world_.scene_->GetComponent<Octree>()->Raycast(query);

    for (RayQueryResult r : results){
        if (MC->InPickState()){
            MC->DeselectPiece();
        } else if (MC->InPutState()
                && !square
                && BOARD->GetSelectedSquare())
        {
            BOARD->Deselect();
            if (yad_->dimmed_)
                RestoreYad();
        }
        if (yad_->hidden_ && !drag_)
            RevealYad();
        if (!r.node_->HasTag("Piece")
         && !r.node_->HasTag("Yad"))
            return r.position_; //return
    }
    none = true;
    return Vector3::ZERO; //return
}
void InputMaster::DimYad()
{
    FX->FadeTo(yad_->light_, YAD_DIMMED);
}
void InputMaster::RestoreYad()
{
    FX->FadeTo(yad_->light_, YAD_FULLBRIGHT);
    FX->FadeTo(yad_->material_, COLOR_GLOW, 0.1f);
}

void InputMaster::HandleJoystickButtons()
{
    for(int joystickId: {0, 1}) {
        HashSet<int>& buttons{pressedJoystickButtons_[joystickId]};

        if (buttons.Size())
            ResetIdle();

        if (buttons.Contains(LucKey::SB_START)){
            if (buttons.Contains(LucKey::SB_SELECT
             || BOARD->IsFull()
             || MC->GetGameState() == GameState::QUATTER))
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
                    MC->SetSelectionMode(SM_CAMERA);
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

        if (MC->InPlayer1State())
        {
            return input_->GetJoystickByIndex(0);

        } else if (MC->InPlayer2State())
        {
            return input_->GetJoystickByIndex(1);
        } else if (MC->GetGameState() == GameState::QUATTER) {
            if (MC->GetPreviousGameState() == GameState::PLAYER1PICKS ||
                MC->GetPreviousGameState() == GameState::PLAYER1PUTS)
            {
                return input_->GetJoystickByIndex(0);

            } else if (MC->GetPreviousGameState() == GameState::PLAYER2PICKS ||
                       MC->GetPreviousGameState() == GameState::PLAYER2PUTS)
            {
                return input_->GetJoystickByIndex(1);
            }
        }

    } else if (input_->GetJoystickByIndex(0)) {

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

    if (MultipleJoysticks() && !CorrectJoystickId(joystickId) && MC->GetGameState() != GameState::QUATTER) return;

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
            BOARD->SelectNearestFreeSquare();
        } else if (MC->selectionMode_ == SM_STEP || MC->selectionMode_ == SM_YAD)
            if (!MC->SelectLastPiece())
                    MC->CameraSelectPiece();
        else if (MC->selectionMode_ == SM_CAMERA)
            MC->CameraSelectPiece();

    } else if (MC->InPutState() ){
        BOARD->PutPiece(MC->GetPickedPiece());
    }
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
    if (drag_){
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
            BOARD->SelectLast();

        } else if (MC->InPickState()){
            if (!MC->SelectLastPiece())
                MC->CameraSelectPiece();
        }
    }

    idleTime_ = 0.0f;
}

Piece* InputMaster::RaycastToPiece()
{
    Ray cameraRay{MouseRay()};

    PODVector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 1000.0f, DRAWABLE_GEOMETRY);
    MC->world_.scene_->GetComponent<Octree>()->Raycast(query);

    for (RayQueryResult r : results){

        for (Piece* p : MC->world_.pieces_)
            if (r.node_ == p->GetNode()){
                rayPiece_ = p;
                return p;
            }
    }
    rayPiece_ = nullptr;
    return nullptr;
}

Square* InputMaster::RaycastToSquare()
{
    Ray cameraRay{MouseRay()};

    PODVector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 1000.0f, DRAWABLE_GEOMETRY);
    MC->world_.scene_->GetComponent<Octree>()->Raycast(query);

    for (RayQueryResult r : results){

        for (Square* s : BOARD->GetSquares())
            if (r.node_ == s->node_){
                raySquare_ = s;
                return s;
            }
    }
    raySquare_ = nullptr;
    return nullptr;
}
bool InputMaster::RaycastToBoard()
{
    Ray cameraRay{MouseRay()};

    PODVector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 1000.0f, DRAWABLE_GEOMETRY);
    MC->world_.scene_->GetComponent<Octree>()->Raycast(query);

    for (RayQueryResult r : results){

            if (r.node_->HasTag("Board")){
                return true;
            }
    }
    return false;
}
bool InputMaster::RaycastToTable()
{
    Ray cameraRay{MouseRay()};

    PODVector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 1000.0f, DRAWABLE_GEOMETRY);
    MC->world_.scene_->GetComponent<Octree>()->Raycast(query);

    for (RayQueryResult r : results){

            if (r.node_->HasTag("Table")){
                return true;
            }
    }
    return false;
}

Ray InputMaster::MouseRay()
{
    Ray mouseRay{CAMERA->camera_->GetScreenRay(mousePos_.x_, mousePos_.y_)};

    return mouseRay;
}

void InputMaster::UpdateMousePos(bool delta)
{
    if (delta){
        IntVector2 mouseDelta{GetSubsystem<Input>()->GetMouseMove()};
        Graphics* graphics{GetSubsystem<Graphics>()};
        mousePos_.x_ += MOUSESPEED * static_cast<float>(mouseDelta.x_) / graphics->GetWidth();
        mousePos_.y_ += MOUSESPEED * static_cast<float>(mouseDelta.y_) / graphics->GetHeight();
    } else {
        IntVector2 mousePos{GetSubsystem<Input>()->GetMousePosition()};
        Graphics* graphics{GetSubsystem<Graphics>()};
        mousePos_.x_ = static_cast<float>(mousePos.x_) / graphics->GetWidth();
        mousePos_.y_ = static_cast<float>(mousePos.y_) / graphics->GetHeight();
    }
    mousePos_.x_ = Clamp(mousePos_.x_, 0.0f, 1.0f);
    mousePos_.y_ = Clamp(mousePos_.y_, 0.0f, 1.0f);
}
