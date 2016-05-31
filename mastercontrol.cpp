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

#include "mastercontrol.h"
#include "inputmaster.h"
#include "effectmaster.h"
#include "quattercam.h"
#include "board.h"
#include <Urho3D/Graphics/Texture2D.h>

URHO3D_DEFINE_APPLICATION_MAIN(MasterControl);

MasterControl* MasterControl::instance_ = NULL;

MasterControl* MasterControl::GetInstance()
{
    return MasterControl::instance_;
}

MasterControl::MasterControl(Context *context):
    Application(context),
    musicGain_{1.0f},
    gameState_{GameState::PLAYER1PICKS},
    previousGameState_{},
    startGameState_{gameState_},
    musicState_{MUSIC_SONG1},
    previousMusicState_{MUSIC_OFF},
    selectionMode_{SM_CAMERA},
    selectedPiece_{nullptr},
    lastSelectedPiece_{nullptr},
    pickedPiece_{nullptr},
    lastReset_{0.0f}
{
    instance_ = this;
}

void MasterControl::Setup()
{
    SetRandomSeed(GetSubsystem<Time>()->GetSystemTime());

    engineParameters_["WindowTitle"] = "Quatter";
    engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs")+"Quatter.log";
    engineParameters_["ResourcePaths"] = "Resources";
    engineParameters_["WindowIcon"] = "icon.png";

//    engineParameters_["FullScreen"] = false;
//    engineParameters_["WindowWidth"] = 960;
//    engineParameters_["WindowHeight"] = 540;
}
void MasterControl::Start()
{
    inputMaster_ = new InputMaster();
    effectMaster_ = new EffectMaster();
    cache_ = GetSubsystem<ResourceCache>();

    CreateScene();

    //Play music
    Sound* song1{GetMusic("Angelight - The Knowledge River")};
    Sound* song2{GetMusic("Cao Sao Vang - Days Of Yore")};
    Node* musicNode{world_.scene_->CreateChild("Music")};

    musicSource1_ = musicNode->CreateComponent<SoundSource>();
    musicSource1_->SetSoundType(SOUND_MUSIC);
    musicSource1_->SetGain(musicGain_);
    musicSource1_->Play(song1);

    musicSource2_ = musicNode->CreateComponent<SoundSource>();
    musicSource2_->SetSoundType(SOUND_MUSIC);
    musicSource2_->SetGain(0.0f);
    musicSource2_->Play(song2);

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MasterControl, HandleUpdate));
}
void MasterControl::Stop()
{
    engine_->DumpResources(true);
}
void MasterControl::Exit()
{
//    File file(context_, "Resources/Endgame.xml", FILE_WRITE);
//    world_.scene_->SaveXML(file);

    engine_->Exit();
}

void MasterControl::CreateScene()
{
    world_.scene_ = new Scene(context_);
    world_.scene_->CreateComponent<Octree>();
    CreateLights();

    //Create skybox
    Node* skyNode{world_.scene_->CreateChild("Sky")};
    Skybox* skybox{skyNode->CreateComponent<Skybox>()};
    skybox->SetModel(GetModel("Box"));
    skybox->SetMaterial(GetMaterial("LeafyKnoll"));

    world_.camera_ = new QuatterCam();

    //Create table
    Node* tableNode{world_.scene_->CreateChild("Table")};
    tableNode->SetRotation(Quaternion(23.5f, Vector3::UP));
    StaticModel* tableModel = tableNode->CreateComponent<StaticModel>();
    tableModel->SetModel(GetModel("Table"));
    tableModel->SetMaterial(GetMaterial("Table"));
    tableModel->GetMaterial()->SetShaderParameter("MatDiffColor", Vector4(0.32f, 0.40f, 0.42f, 1.0f));
    tableModel->SetCastShadows(true);
    Node* hitNode{world_.scene_->CreateChild("HitPlane")};
    hitNode->SetPosition(Vector3::DOWN * 1.23f);
    hitNode->SetScale(128.0f);
    StaticModel* hitPlane{hitNode->CreateComponent<StaticModel>()};
    hitPlane->SetModel(MC->GetModel("Plane"));
    hitPlane->SetMaterial(MC->GetMaterial("Invisible"));


    //Create board and pieces
    world_.board_ = new Board();
    world_.pieces_.Reserve(NUM_PIECES);
    for (int p{0}; p < NUM_PIECES; ++p){

        Piece* newPiece = new Piece(Piece::Attributes(p));
        world_.pieces_.Push(SharedPtr<Piece>(newPiece));
        newPiece->SetPosition(AttributesToPosition(newPiece->ToInt())
                              + Vector3(Random(0.05f),
                                        0.0f,
                                        Random(0.05f)));
    }
    inputMaster_->ConstructYad();
}


void MasterControl::CreateLights()
{
    //Add leafy light source
    leafyLightNode_ = world_.scene_->CreateChild("DirectionalLight");
    leafyLightNode_->SetPosition(Vector3(6.0f, 96.0f, 9.0f));
    leafyLightNode_->LookAt(Vector3(0.0f, 0.0f, 0.0f));
    leafyLight_ = leafyLightNode_->CreateComponent<Light>();
    leafyLight_->SetLightType(LIGHT_SPOT);
    leafyLight_->SetRange(180.0f);
    leafyLight_->SetFov(34.0f);
    leafyLight_->SetCastShadows(false);
    leafyLight_->SetShapeTexture(static_cast<Texture*>(cache_->GetResource<Texture2D>("Textures/LeafyMask.png")));

    //Add a directional light to the world. Enable cascaded shadows on it
    Node* downardsLightNode{world_.scene_->CreateChild("DirectionalLight")};
    downardsLightNode->SetPosition(Vector3(2.0f, 23.0f, 3.0f));
    downardsLightNode->LookAt(Vector3(0.0f, 0.0f, 0.0f));
    Light* downwardsLight{downardsLightNode->CreateComponent<Light>()};
    downwardsLight->SetLightType(LIGHT_DIRECTIONAL);
    downwardsLight->SetBrightness(0.34f);
    downwardsLight->SetColor(Color(0.8f, 0.9f, 0.95f));
    downwardsLight->SetCastShadows(true);
    downwardsLight->SetShadowBias(BiasParameters(0.000025f, 0.5f));
    downwardsLight->SetShadowCascade(CascadeParameters(5.0f, 7.0f, 23.0f, 42.0f, 0.8f));

    //Create point lights
    for (Vector3 pos : {Vector3(-10.0f, 8.0f, -23.0f), Vector3(-20.0f, -8.0f, 23.0f), Vector3(20.0f, -7.0f, 23.0f)}){
        Node* pointLightNode_{world_.scene_->CreateChild("PointLight")};
        pointLightNode_->SetPosition(pos);
        Light* pointLight{pointLightNode_->CreateComponent<Light>()};
        pointLight->SetLightType(LIGHT_POINT);
        pointLight->SetBrightness(0.42f);
        pointLight->SetRange(42.0f);
        pointLight->SetColor(Color(0.75f, 1.0f, 0.75f));
        pointLight->SetCastShadows(true);
        pointLight->SetShadowResolution(0.25f);
        pointLight->SetShadowIntensity(0.6f);
    }
}

Piece* MasterControl::GetSelectedPiece() const
{
    for (Piece* p: world_.pieces_){

        if (p->GetState() == PieceState::SELECTED){
            return p;
        }
    }
    return nullptr;
}

Piece* MasterControl::GetPickedPiece() const
{
    for (Piece* p: world_.pieces_){

        if (p->GetState() == PieceState::PICKED){
            return p;
        }
    }
    return nullptr;
}
int MasterControl::CountFreePieces()
{
    int count{0};
    for (Piece* p: world_.pieces_){

        if (p->GetState() == PieceState::FREE){
            ++count;
        }
    }
    return count;
}

Sound* MasterControl::GetMusic(String name) const {
    Sound* song{cache_->GetResource<Sound>("Music/"+name+".ogg")};
    song->SetLooped(true);
    return song;
}

Sound*MasterControl::GetSample(String name) const {
    Sound* sample{cache_->GetResource<Sound>("Samples/"+name+".ogg")};
    sample->SetLooped(false);
    return sample;
}

void MasterControl::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    //    float t{eventData[Update::P_TIMESTEP].GetFloat()};

    if (selectionMode_ == SM_CAMERA && !inputMaster_->IsIdle())
        CameraSelectPiece();

    //Wave leafy light
    leafyLightNode_->SetRotation(Quaternion(MC->Sine(MC->Sine(0.1f, 0.05f, 0.23f), -0.23f, 0.23f) + 90.0f, Vector3::RIGHT) *
                                 Quaternion(MC->Sine(0.23f, 178.0f, 182.0f), Vector3::FORWARD));
    leafyLight_->SetBrightness(0.34f + MC->Sine(0.011f, 0.05f, 0.23f) + MC->Sine(0.02f, 0.05f, 0.13f));
}
void MasterControl::SelectPiece(Piece* piece)
{
    DeselectPiece();

    selectedPiece_ = piece;
    piece->Select();
}

void MasterControl::CameraSelectPiece()
{
    if (IsLame())
        return;

    Piece* nearest{selectedPiece_};
    for (Piece* piece: world_.pieces_){
        if (!nearest) {
            nearest = piece;
        } else if ((piece->GetState() == PieceState::FREE || piece->GetState() == PieceState::SELECTED) &&
                   LucKey::Distance(CAMERA->GetPosition(), piece->GetPosition())    <
                   LucKey::Distance(CAMERA->GetPosition(), nearest->GetPosition()))
        {
            nearest = piece;
        }
    }
    if (nearest != selectedPiece_){
        SelectPiece(nearest);
    }
}

bool MasterControl::SelectLastPiece()
{
    if (lastSelectedPiece_){
        SelectPiece(lastSelectedPiece_);
        return true;
    } else
        return false;
}

void MasterControl::StepSelectPiece(bool next)
{
    SetSelectionMode(SM_STEP);

    if (selectedPiece_){
        int selectInt{selectedPiece_->ToInt()};

        while (world_.pieces_.At(selectInt)->GetState() != PieceState::FREE){

            if (next){

                selectInt -= 1;
                if (selectInt < 0) selectInt = NUM_PIECES - 1;

            } else {

                selectInt += 1;
                if (selectInt > NUM_PIECES - 1) selectInt = 0;

            }
        }

        SelectPiece(world_.pieces_.At(selectInt));
    } else if (!SelectLastPiece()){
        CameraSelectPiece();
    }
}

void MasterControl::DeselectPiece()
{
    if (selectedPiece_){
        lastSelectedPiece_ = selectedPiece_;
        selectedPiece_->Deselect();
    }
    selectedPiece_ = nullptr;
}

void MasterControl::NextPhase()
{
    if (gameState_ == GameState::QUATTER)
        return;

    if (selectedPiece_){
        selectedPiece_->Deselect();
    }

    previousGameState_ = gameState_;

    switch (gameState_)    {
    case GameState::PLAYER1PICKS: {
        gameState_ = GameState::PLAYER2PUTS;
    } break;
    case GameState::PLAYER2PUTS: {
        gameState_ = GameState::PLAYER2PICKS;
        if (selectionMode_ != SM_YAD)
            CameraSelectPiece();
    } break;
    case GameState::PLAYER2PICKS: {
        gameState_ = GameState::PLAYER1PUTS;
    } break;
    default: case GameState::PLAYER1PUTS: {
        gameState_ = GameState::PLAYER1PICKS;
        if (selectionMode_ != SM_YAD)
            CameraSelectPiece();
    } break;
    }
}
void MasterControl::Quatter()
{
    previousGameState_ = gameState_;

    gameState_ = GameState::QUATTER;
}
void MasterControl::Reset()
{
    lastReset_ = GetSubsystem<Time>()->GetElapsedTime();

    for (Piece* p: world_.pieces_){

        p->Reset();
    }
    world_.board_->Reset();


    if (gameState_ == GameState::QUATTER){

        if (previousGameState_ == GameState::PLAYER1PUTS){
            gameState_ = GameState::PLAYER1PICKS;
        } else if (previousGameState_ == GameState::PLAYER2PUTS){
            gameState_ = GameState::PLAYER2PICKS;
        }
    } else {

        if (startGameState_ == GameState::PLAYER1PICKS){
            gameState_ = GameState::PLAYER1PICKS;
        } else if (startGameState_ == GameState::PLAYER2PICKS){
            gameState_ = GameState::PLAYER1PICKS;
        }
    }
    startGameState_ = gameState_;
}

void MasterControl::NextMusicState()
{
    //Fade out
    if (musicState_ == MUSIC_SONG1){
        previousMusicState_ = musicState_;
        musicState_ = MUSIC_OFF;
        FX->FadeOut(musicSource1_);
    } else if (musicState_ == MUSIC_SONG2){
        previousMusicState_ = musicState_;
        musicState_ = MUSIC_OFF;
        FX->FadeOut(musicSource2_);
    } else if (musicState_ == MUSIC_OFF){
        previousMusicState_ = musicState_;
        if (previousMusicState_ == MUSIC_SONG1)
            musicState_ = MUSIC_SONG2;
        else if (previousMusicState_ == MUSIC_SONG2)
            musicState_ = MUSIC_SONG1;
    }

    //Fade in
    if (musicState_ == MUSIC_SONG1){
        FX->FadeTo(musicSource1_, Max(musicGain_, 0.1f));
    } else if (musicState_ == MUSIC_SONG2){
        FX->FadeTo(musicSource2_, Max(musicGain_, 0.1f));
    }
}

void MasterControl::NextSelectionMode()
{
    switch (selectionMode_){
    case SM_CAMERA:
        SetSelectionMode(SM_YAD);
        break;
    case SM_STEP:
        SetSelectionMode(SM_CAMERA);
        break;
    case SM_YAD:
        SetSelectionMode(SM_CAMERA);
        break;
    default: break;
    }
}
void MasterControl::SetSelectionMode(SelectionMode mode)
{
    if (selectionMode_ == mode) return;

    selectionMode_ = mode;
    switch (mode){
    default: case SM_CAMERA:
        CameraSelectPiece();
        break;
    case SM_STEP:
        CameraSelectPiece();
        break;
    case SM_YAD:
        DeselectPiece();
        break;
    }
}

void MasterControl::MusicGainUp(float step)
{
    musicGain_ = Clamp(musicGain_ + step, step, 1.0f);

    if (musicState_ == MUSIC_SONG1)
        FX->FadeTo(musicSource1_, musicGain_, 0.23f);
    else if (musicState_ == MUSIC_SONG2)
        FX->FadeTo(musicSource2_, musicGain_, 0.23f);
}
void MasterControl::MusicGainDown(float step)
{
    musicGain_ = Clamp(musicGain_ - step, 0.0f, 1.0f);

    if (musicState_ == MUSIC_SONG1)
        FX->FadeTo(musicSource1_, musicGain_, 0.23f);
    else if (musicState_ == MUSIC_SONG2)
        FX->FadeTo(musicSource2_, musicGain_, 0.23f);
}

void MasterControl::TakeScreenshot()
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

float MasterControl::Sine(const float freq, const float min, const float max, const float shift)
{
    float phase{freq * world_.scene_->GetElapsedTime() + shift};
    float add{0.5f * (min + max)};
    return LucKey::Sine(phase) * 0.5f * (max - min) + add;
}
float MasterControl::Cosine(const float freq, const float min, const float max, const float shift)
{
    float phase{freq * world_.scene_->GetElapsedTime() + shift};
    float add{0.5f * (min + max)};
    return LucKey::Cosine(phase) * 0.5f * (max - min) + add;
}
