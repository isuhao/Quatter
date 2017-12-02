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

#include "mastercontrol.h"
#include "inputmaster.h"
#include "effectmaster.h"
#include "guimaster.h"
#include "quattercam.h"
#include "piece.h"
#include "board.h"
#include "square.h"
#include "indicator.h"
#include "yad.h"

URHO3D_DEFINE_APPLICATION_MAIN(MasterControl);

MasterControl* MasterControl::instance_ = NULL;

MasterControl* MasterControl::GetInstance()
{
    return MasterControl::instance_;
}

MasterControl::MasterControl(Context *context):
    Application(context),
    musicGain_{1.0f},
    gameMode_{GM_PVP_LOCAL},
    gameState_{GameState::SPLASH},
    previousGameState_{},
    startGameState_{gameState_},
    musicState_{MUSIC_SONG1},
    previousMusicState_{MUSIC_OFF},
    selectionMode_{SM_CAMERA},
    selectedPiece_{},
    lastSelectedPiece_{},
    pickedPiece_{},
    lastReset_{0.0f}
{
    instance_ = this;

    QuatterCam::RegisterObject(context_);
    Piece::RegisterObject(context_);
    Board::RegisterObject(context_);
    Square::RegisterObject(context_);
    Indicator::RegisterObject(context_);
    Yad::RegisterObject(context_);
}

void MasterControl::Setup()
{
    SetRandomSeed(TIME->GetSystemTime());

    engineParameters_["LogName"] = FILES->GetAppPreferencesDir("urho3d", "logs") + "Quatter.log";
    engineParameters_["WindowTitle"] = "Quatter";
    engineParameters_["WindowIcon"] = "icon.png";

    //Add resource paths
    String resourcePaths{};

    /*if (FILES->DirExists(FILES->GetAppPreferencesDir("luckey", "quatter")))
        resourcePaths = FILES->GetAppPreferencesDir("luckey", "quatter");
    else */if (FILES->DirExists("Resources"))
        resourcePaths = "Resources";
    else if (FILES->DirExists("../Quatter/Resources"))
        resourcePaths = "../Quatter/Resources";

    resourceFolder_ = resourcePaths;
    resourcePaths += ";";

    if (FILES->DirExists("Data"))
        resourcePaths += "Data;";
    if (FILES->DirExists("CoreData"))
        resourcePaths += "CoreData;";

    engineParameters_[EP_RESOURCE_PATHS] = resourcePaths;

    LoadSettings();


    //    engineParameters_["borderless"] = true;
}
void MasterControl::Start()
{
    context_->RegisterSubsystem(new InputMaster(context_));
    context_->RegisterSubsystem(new EffectMaster(context_));

    CreateScene();

    context_->RegisterSubsystem(new GUIMaster(context_));

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MasterControl, HandleUpdate));
}


void MasterControl::Stop()
{
    SaveSettings();

    engine_->DumpResources(true);
}
void MasterControl::Exit()
{
//    File file(context_, "Resources/Endgame.xml", FILE_WRITE);
//    world_.scene_->SaveXML(file);

    engine_->Exit();
}

void MasterControl::LoadSettings()
{
    if (FILES->FileExists("Resources/Settings.xml")){
        File file(context_, "Resources/Settings.xml", FILE_READ);
        XMLFile configFile(context_);
        configFile.Load(file);
        XMLElement graphics{ configFile.GetRoot().GetChild("Graphics") };
        XMLElement audio{ configFile.GetRoot().GetChild("Audio") };

        if (graphics) {

            engineParameters_[EP_WINDOW_WIDTH] = graphics.GetInt("Width");
            engineParameters_[EP_WINDOW_HEIGHT] = graphics.GetInt("Height");
            engineParameters_[EP_FULL_SCREEN] = graphics.GetBool("Fullscreen");
        }
        if (audio) {

            musicGain_ = audio.GetFloat("MusicGain");
        }
    }
}
void MasterControl::SaveSettings()
{
    XMLFile file(context_);
    XMLElement root(file.CreateRoot("Settings"));

    XMLElement graphicsElement(root.CreateChild("Graphics"));
    graphicsElement.SetInt("Width", GRAPHICS->GetWidth());
    graphicsElement.SetInt("Height", GRAPHICS->GetHeight());
    graphicsElement.SetBool("Fullscreen", GRAPHICS->GetFullscreen());

    XMLElement audioElement(root.CreateChild("Audio"));
    audioElement.SetFloat("MusicGain", musicGain_);

    file.SaveFile("Resources/Settings.xml");
}

void MasterControl::CreateScene()
{
    world_.scene_ = new Scene(context_);
    world_.scene_->CreateComponent<Octree>();

    CreateSkybox();

    Node* cameraNode{ world_.scene_->CreateChild("Camera", LOCAL) };
    world_.camera_ = cameraNode->CreateComponent<QuatterCam>();

    CreateLights();
    CreateJukebox();
    CreateTable();
    CreateBoardAndPieces();

    GetSubsystem<InputMaster>()->ConstructYad();
}
void MasterControl::CreateLights()
{
    //Add leafy light source
    leafyLightNode_ = world_.scene_->CreateChild("DirectionalLight", LOCAL);
    leafyLightNode_->SetPosition(Vector3(6.0f, 96.0f, 9.0f));
    leafyLightNode_->LookAt(Vector3(0.0f, 0.0f, 0.0f));
    leafyLight_ = leafyLightNode_->CreateComponent<Light>();
    leafyLight_->SetLightType(LIGHT_SPOT);
    leafyLight_->SetRange(180.0f);
    leafyLight_->SetFov(34.0f);
    leafyLight_->SetShapeTexture(static_cast<Texture*>(CACHE->GetResource<Texture2D>("Textures/LeafyMask.png")));

    //Add a directional light to the world. Enable cascaded shadows on it
    Node* downardsLightNode{ world_.scene_->CreateChild("DirectionalLight", LOCAL) };
    downardsLightNode->SetPosition(Vector3(2.0f, 23.0f, 3.0f));
    downardsLightNode->LookAt(Vector3::ZERO);
    Light* downwardsLight{ downardsLightNode->CreateComponent<Light>() };
    downwardsLight->SetLightType(LIGHT_DIRECTIONAL);
    downwardsLight->SetBrightness(0.34f);
    downwardsLight->SetColor(Color(0.8f, 0.9f, 0.95f));
    downwardsLight->SetCastShadows(true);
    downwardsLight->SetShadowBias(BiasParameters(0.000025f, 0.5f));
    downwardsLight->SetShadowCascade(CascadeParameters(7.0f, 13.0f, 23.0f, 42.0f, 0.6));

    //Create point lights
    for (Vector3 pos : { Vector3(-10.0f, 8.0f, -23.0f), Vector3(-20.0f, -8.0f, 23.0f), Vector3(20.0f, -7.0f, 23.0f) }) {

        Node* pointLightNode_{ world_.scene_->CreateChild("PointLight", LOCAL) };
        pointLightNode_->SetPosition(pos);

        Light* pointLight{ pointLightNode_->CreateComponent<Light>() };
        pointLight->SetLightType(LIGHT_POINT);
        pointLight->SetBrightness(0.42f);
        pointLight->SetRange(42.0f);
        pointLight->SetColor(Color(0.75f, 1.0f, 0.75f));
        pointLight->SetCastShadows(true);
        pointLight->SetShadowResolution(0.25f);
        pointLight->SetShadowIntensity(0.6f);
    }
}
void MasterControl::CreateSkybox()
{
    Node* skyNode{ world_.scene_->CreateChild("Sky", LOCAL) };
    Skybox* skybox{ skyNode->CreateComponent<Skybox>() };
    skybox->SetModel(GetModel("Sphere"));
    skybox->SetMaterial(GetMaterial("LeafyKnoll"));
}
void MasterControl::CreateJukebox()
{
    Sound* song1{ GetMusic("Angelight - The Knowledge River") };
    Sound* song2{ GetMusic("Angelight - The Knowledge River") };
    Node* musicNode{ world_.scene_->CreateChild("Music", LOCAL) };

    musicSource1_ = musicNode->CreateComponent<SoundSource>();
    musicSource1_->SetSoundType(SOUND_MUSIC);
    musicSource1_->SetGain(musicGain_);
    musicSource1_->Play(song1);

    musicSource2_ = musicNode->CreateComponent<SoundSource>();
    musicSource2_->SetSoundType(SOUND_MUSIC);
    musicSource2_->SetGain(0.0f);
    musicSource2_->Play(song2);
}
void MasterControl::CreateTable()
{
    world_.tableNode_ = world_.scene_->CreateChild("Table", LOCAL);
    world_.tableNode_->AddTag("Table");
    world_.tableNode_->SetRotation(Quaternion(23.5f, Vector3::UP));

    StaticModel* tableModel{ world_.tableNode_->CreateComponent<StaticModel>() };
    tableModel->SetModel(GetModel("Table"));
    tableModel->SetMaterial(GetMaterial("Table"));
    tableModel->GetMaterial()->SetShaderParameter("MatDiffColor", Vector4(0.32f, 0.40f, 0.42f, 1.0f));
    tableModel->SetCastShadows(true);

    Node* hitNode{ world_.scene_->CreateChild("HitPlane", LOCAL) };
    hitNode->SetPosition(Vector3::DOWN * 1.23f);
    hitNode->SetScale(128.0f);

    StaticModel* hitPlane{ hitNode->CreateComponent<StaticModel>() };
    hitPlane->SetModel(MC->GetModel("Plane"));
    hitPlane->SetMaterial(MC->GetMaterial("Invisible"));
}
void MasterControl::CreateBoardAndPieces()
{
    Node* boardNode{ world_.scene_->CreateChild("Board", LOCAL) };
    world_.board_ = boardNode->CreateComponent<Board>();
    world_.pieces_.Reserve(NUM_PIECES);

    for (int p{0}; p < NUM_PIECES; ++p){

        Node* pieceNode{ world_.scene_->CreateChild("Piece", LOCAL) };
        Piece* newPiece{ pieceNode->CreateComponent<Piece>() };
        newPiece->Init(Piece::PieceAttributes(p));

        world_.pieces_.Push(newPiece);
        newPiece->SetPosition(AttributesToPosition(newPiece->ToInt())
                              + Vector3(Random(0.05f),
                                        0.0f,
                                        Random(0.05f)));
    }
}

Sound* MasterControl::GetMusic(String name) const
{
    Sound* song{ CACHE->GetResource<Sound>("Music/" + name + ".ogg") };
    song->SetLooped(true);

    return song;
}
Sound* MasterControl::GetSample(String name) const
{
    Sound* sample{ CACHE->GetResource<Sound>("Samples/" + name + ".ogg") };
    sample->SetLooped(false);

    return sample;
}

void MasterControl::HandleUpdate(StringHash eventType, VariantMap& eventData)
{ (void)eventType; (void)eventData;

    if (selectionMode_ == SM_CAMERA && !GetSubsystem<InputMaster>()->IsIdle())
        CameraSelectPiece();

    //Wave leafy light
    leafyLightNode_->SetRotation(Quaternion(MC->Sine(0.1f, 90.0f-0.23f, 90.23f), Vector3::RIGHT) *
                                 Quaternion(MC->Sine(0.23f, 178.0f, 182.0f), Vector3::FORWARD));
    leafyLight_->SetBrightness(0.34f + MC->Sine(0.011f, 0.05f, 0.23f) + MC->Sine(0.02f, 0.05f, 0.13f));
}

void MasterControl::SelectPiece(Piece* piece)
{
    DeselectPiece();
    selectedPiece_ = piece;
    piece->Select();
}
void MasterControl::CameraSelectPiece(bool force)
{
    if (!force && IsLame())
        return;

    Piece* nearest{ selectedPiece_ };

    for (Piece* piece : world_.pieces_) {

        if (!nearest
         && (piece->GetState() == PieceState::FREE || piece->GetState() == PieceState::SELECTED))
        {
            nearest = piece;
        } else if ((piece->GetState() == PieceState::FREE || piece->GetState() == PieceState::SELECTED) &&
                   LucKey::Distance(CAMERA->GetPosition(), piece->GetPosition())    <
                   LucKey::Distance(CAMERA->GetPosition(), nearest->GetPosition()))
        {
            nearest = piece;
        }
    }

    if (nearest != selectedPiece_) {
        SelectPiece(nearest);
    }
}
bool MasterControl::SelectLastPiece()
{
    if (lastSelectedPiece_) {

        SelectPiece(lastSelectedPiece_);
        return true;

    } else

        return false;
}
void MasterControl::StepSelectPiece(bool next)
{
    SetSelectionMode(SM_STEP);

    if (selectedPiece_) {
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
        CameraSelectPiece(true);
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

    DeselectPiece();

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
    lastReset_ = TIME->GetElapsedTime();

    for (Piece* p : world_.pieces_){

        p->Reset();
    }
    world_.board_->Reset();

    lastSelectedPiece_ = nullptr;

    if (gameState_ == GameState::MENU) {

        GetSubsystem<GUIMaster>()->SetLogoVisible(false);
        gameState_ = GameState::PLAYER1PICKS;

    } else if (gameState_ == GameState::QUATTER) {

        if (previousGameState_ == GameState::PLAYER1PUTS) {
            gameState_ = GameState::PLAYER1PICKS;
        } else if (previousGameState_ == GameState::PLAYER2PUTS) {
            gameState_ = GameState::PLAYER2PICKS;
        }
    } else {

        if (startGameState_ == GameState::PLAYER1PICKS) {
            gameState_ = GameState::PLAYER2PICKS;
        } else if (startGameState_ == GameState::PLAYER2PICKS) {
            gameState_ = GameState::PLAYER1PICKS;
        }
    }
    startGameState_ = gameState_;
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
        break;
    case SM_YAD:
        DeselectPiece();
        break;
    }
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
    //Pick song
    } else if (musicState_ == MUSIC_OFF){
        if (previousMusicState_ == MUSIC_SONG1)
            musicState_ = MUSIC_SONG2;
        else if (previousMusicState_ == MUSIC_SONG2)
            musicState_ = MUSIC_SONG1;
        previousMusicState_ = musicState_;
    }

    //Fade in
    if (musicState_ == MUSIC_SONG1){
        FX->FadeTo(musicSource1_, Max(musicGain_, 0.1f));
    } else if (musicState_ == MUSIC_SONG2){
        FX->FadeTo(musicSource2_, Max(musicGain_, 0.1f));
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

void MasterControl::EnterMenu()
{
    gameState_ = GameState::MENU;
    world_.camera_->TargetMenu();

    GetSubsystem<GUIMaster>()->SetLogoVisible(true);
}

void MasterControl::TakeScreenshot()
{
    Image screenshot{ context_ };
    GRAPHICS->TakeScreenShot(screenshot);
    //Here we save in the Screenshots folder with date and time appended
    String fileName{ GetSubsystem<FileSystem>()->GetProgramDir() + "Screenshots/Screenshot_" +
                Time::GetTimeStamp().Replaced(':', '_').Replaced('.', '_').Replaced(' ', '_') + ".png" };
    Log::Write(1, fileName);
    screenshot.SavePNG(fileName);
}

float MasterControl::Sine(const float freq, const float min, const float max, const float shift)
{
    float phase{ freq * world_.scene_->GetElapsedTime() + shift };
    float add{ 0.5f * (min + max) };

    return LucKey::Sine(phase) * 0.5f * (max - min) + add;
}
float MasterControl::Cosine(const float freq, const float min, const float max, const float shift)
{
    float phase{ freq * world_.scene_->GetElapsedTime() + shift };
    float add{ 0.5f * (min + max) };

    return LucKey::Cosine(phase) * 0.5f * (max - min) + add;
}
