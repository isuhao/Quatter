#include "inputmaster.h"
#include "quattercam.h"
#include "board.h"
#include "piece.h"

InputMaster::InputMaster() : Master(),
    input_{GetSubsystem<Input>()},
    idle_{false}
{
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleMouseButtonDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(InputMaster, HandleMouseButtonUp));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(InputMaster, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(InputMaster, HandleKeyUp));
    SubscribeToEvent(E_JOYSTICKBUTTONDOWN, URHO3D_HANDLER(InputMaster, HandleJoystickButtonDown));
    SubscribeToEvent(E_JOYSTICKBUTTONUP, URHO3D_HANDLER(InputMaster, HandleJoystickButtonUp));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(InputMaster, HandleUpdate));
}

void InputMaster::HandleKeyDown(StringHash eventType, VariantMap &eventData)
{
    int key{eventData[KeyDown::P_KEY].GetInt()};
    pressedKeys_.Insert(key);

    float volumeStep{0.1f};

    switch (key){
    case KEY_SPACE:{
        HandleActionButtonPressed();
    } break;
    //Exit when ESC is pressed
    case KEY_ESC:{
        MC->Exit();
    } break;
        //Take screenshot when 9 is pressed
    case KEY_9:{
        Screenshot();
    } break;
    case KEY_M: {
        MC->ToggleMusic();
    } break;
    case KEY_KP_PLUS: {
        MC->MusicGainUp(volumeStep);
    } break;
    case KEY_KP_MINUS: {
        MC->MusicGainDown(volumeStep);
    } break;
    default: break;
    }
}
void InputMaster::HandleKeyUp(StringHash eventType, VariantMap &eventData)
{
    using namespace KeyUp;
    int key{eventData[P_KEY].GetInt()};
    if (pressedKeys_.Contains(key)) pressedKeys_.Erase(key);
}

void InputMaster::HandleMouseButtonDown(StringHash eventType, VariantMap &eventData)
{
    using namespace MouseButtonDown;
    int button{eventData[P_BUTTON].GetInt()};
    pressedMouseButtons_.Insert(button);
}


void InputMaster::HandleMouseButtonUp(StringHash eventType, VariantMap &eventData)
{
    using namespace MouseButtonUp;
    int button{eventData[P_BUTTON].GetInt()};
    if (pressedMouseButtons_.Contains(button)) pressedMouseButtons_.Erase(button);
}

void InputMaster::HandleJoystickButtonDown(StringHash eventType, VariantMap &eventData)
{
    using namespace JoystickButtonDown;
    int joystickId{eventData[P_JOYSTICKID].GetInt()};
    int button{eventData[P_BUTTON].GetInt()};
    switch (button){
    case 14: HandleActionButtonPressed();
        break;
    default: break;
    }

    pressedJoystickButtons_[joystickId].Insert(button);
}

void InputMaster::HandleJoystickButtonUp(StringHash eventType, VariantMap &eventData)
{
    using namespace JoystickButtonUp;
    int joystickId{eventData[P_JOYSTICKID].GetInt()};
    int button{eventData[P_BUTTON].GetInt()};
    if (pressedJoystickButtons_[joystickId].Contains(button)) pressedJoystickButtons_.Erase(button);
}

void InputMaster::HandleActionButtonPressed()
{
    if (MC->GetGamePhase() == GamePhase::PLAYER1PICKS ||
        MC->GetGamePhase() == GamePhase::PLAYER2PICKS )
    {
        Piece* selectedPiece{MC->GetSelectedPiece()};
        if (selectedPiece){
            selectedPiece->Pick();
            MC->NextPhase();
        }
    } else if (MC->GetGamePhase() == GamePhase::PLAYER1PUTS ||
               MC->GetGamePhase() == GamePhase::PLAYER2PUTS )
    {
        MC->world.board_->PutPiece(MC->GetPickedPiece());
        MC->NextPhase();
    }
}

void InputMaster::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    float t{eventData[Update::P_TIMESTEP].GetFloat()};
    idleTime_ += t;

    HandleCameraMovement(t);
}

void InputMaster::HandleCameraMovement(float t)
{
    Vector2 camRot{};
    float camZoom{};

    float keyRotMultiplier{0.5f};
    float keyZoomSpeed{0.1f};

    float joyRotMultiplier{80.0f};
    float joyZoomSpeed{3.4f};

    if (pressedKeys_.Size()) idleTime_ = 0.0f;
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
        case KEY_Q:{ camZoom += keyZoomSpeed;
        } break;
        case KEY_E:{ camZoom += -keyZoomSpeed;
        } break;
        default: break;
        }
    }

    if (pressedMouseButtons_.Contains(MOUSEB_RIGHT)){
        idleTime_ = 0.0f;
        IntVector2 mouseMove = input_->GetMouseMove();
        camRot += Vector2(mouseMove.x_, mouseMove.y_) * 0.1f;
    }
    ///Should check whose turn it is when two joysticks are connected
    JoystickState* joy0{input_->GetJoystickByIndex(0)};
    if (joy0){
        Vector2 rotation{-Vector2(joy0->GetAxisPosition(0), joy0->GetAxisPosition(1))
                         -Vector2(joy0->GetAxisPosition(2), joy0->GetAxisPosition(3))};
        if (rotation.Length()){
            idleTime_ = 0.0f;
            camRot += rotation * t * joyRotMultiplier;
        }
        camZoom += t * joyZoomSpeed * (joy0->GetAxisPosition(12) - joy0->GetAxisPosition(13));
    }

    float idleThreshold{5.0f};
    float idleStartup{Min(0.5f * (idleTime_ - idleThreshold), 1.0f)};
    if (idleTime_ > idleThreshold){
        if (!idle_) {
            idle_ = true;
            for (Piece* p: MC->world.pieces_){
                p->Deselect();
            }
        }
        camRot += Vector2(t * idleStartup * -0.5f,
                          t * idleStartup * MC->Sine(0.23f, -0.042f, 0.042f));
    } else {
        if (idle_) idle_ = false;
    }
    //Speed up camera movement when shift key is held down
    if (pressedKeys_.Contains(KEY_SHIFT)){
        camRot *= 3.0f;
        camZoom *= 2.0f;
    }

    //Slow down up and down rotation when nearing extremes
    SmoothCameraMovement(camZoom, camRot);

    CAMERA->Rotate(smoothCamRotate_);
    CAMERA->Zoom(smoothCamZoom_);
}
void InputMaster::SmoothCameraMovement(float camZoom, Vector2 camRot)
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
    camRot.y_ *= pitchBrake;
    smoothCamRotate_.y_ *= pitchBrake;
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
    smoothCamZoom_ *= zoomBrake;

    smoothCamRotate_ = 0.0666f * (camRot  + smoothCamRotate_ * 14.0f);
    smoothCamZoom_   = 0.05f * (camZoom + smoothCamZoom_   * 19.0f);
}

void InputMaster::Screenshot()
{
    Graphics* graphics{GetSubsystem<Graphics>()};
    Image screenshot{context_};
    graphics->TakeScreenShot(screenshot);
    //Here we save in the Screenshots folder with date and time appended
    String fileName{GetSubsystem<FileSystem>()->GetProgramDir() + "Screenshots/Screenshot_" +
                Time::GetTimeStamp().Replaced(':', '_').Replaced('.', '_').Replaced(' ', '_')+".png"};
    Log::Write(1, fileName);
    screenshot.SavePNG(fileName);
}
