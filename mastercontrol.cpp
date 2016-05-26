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
    musicState_{MUSIC_SONG1},
    previousMusicState_{MUSIC_OFF}
{
    instance_ = this;
}

void MasterControl::Setup()
{
    SetRandomSeed(GetSubsystem<Time>()->GetSystemTime());

    engineParameters_["WindowTitle"] = "Quatter";
    engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs")+"Quatter.log";
    engineParameters_["ResourcePaths"] = "Data;CoreData;Resources";
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
    Node* musicNode{world.scene->CreateChild("Music")};

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
    File file(context_, "Resources/Endgame.xml", FILE_WRITE);
    world.scene->SaveXML(file);

    engine_->Exit();
}

void MasterControl::CreateScene()
{
    world.scene = new Scene(context_);
    world.scene->CreateComponent<Octree>();
    CreateLights();

    //Create skybox
    Node* skyNode{world.scene->CreateChild("Sky")};
    Skybox* skybox{skyNode->CreateComponent<Skybox>()};
    skybox->SetModel(GetModel("Box"));
    skybox->SetMaterial(GetMaterial("LeafyKnoll"));

    world.camera = new QuatterCam();

    //Create table
    Node* tableNode = world.scene->CreateChild("Table");
    tableNode->SetRotation(Quaternion(23.5f, Vector3::UP));
    StaticModel* tableModel = tableNode->CreateComponent<StaticModel>();
    tableModel->SetModel(GetModel("Table"));
    tableModel->SetMaterial(GetMaterial("Table"));
    tableModel->GetMaterial()->SetShaderParameter("MatDiffColor", Vector4(0.32f, 0.40f, 0.42f, 1.0f));
    tableModel->SetCastShadows(true);

    //Create board and pieces
    world.board_ = new Board();

    for (int p{0}; p < NUM_PIECES; ++p){
        Piece* newPiece = new Piece(Piece::Attributes(p));
        world.pieces_.Push(SharedPtr<Piece>(newPiece));
        newPiece->SetPosition(AttributesToPosition(p) + Vector3(Random(0.05f), 0.0f, Random(0.05f)));
    }
}


void MasterControl::CreateLights()
{
    //Add leafy light source
    leafyLightNode_ = world.scene->CreateChild("DirectionalLight");
    leafyLightNode_->SetPosition(Vector3(6.0f, 96.0f, 9.0f));
    leafyLightNode_->LookAt(Vector3(0.0f, 0.0f, 0.0f));
    leafyLight_ = leafyLightNode_->CreateComponent<Light>();
    leafyLight_->SetLightType(LIGHT_SPOT);
    leafyLight_->SetRange(180.0f);
    leafyLight_->SetFov(34.0f);
    leafyLight_->SetCastShadows(false);
    leafyLight_->SetShapeTexture(static_cast<Texture*>(cache_->GetResource<Texture2D>("Textures/LeafyMask.png")));

    //Add a directional light to the world. Enable cascaded shadows on it
    Node* downardsLightNode{world.scene->CreateChild("DirectionalLight")};
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
        Node* pointLightNode_{world.scene->CreateChild("PointLight")};
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
    for (Piece* p: world.pieces_){
        if (p->GetState() == PieceState::SELECTED){
            return p;
        }
    }
    return nullptr;
}

Piece* MasterControl::GetPickedPiece() const
{
    for (Piece* p: world.pieces_){
        if (p->GetState() == PieceState::PICKED){
            return p;
        }
    }
    return nullptr;
}
int MasterControl::CountFreePieces()
{
    int count{0};
    for (Piece* p: world.pieces_){
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

    UpdateSelectedPiece();
    //Wave leafy light
    leafyLightNode_->SetRotation(Quaternion(Sine(Sine(0.1f, 0.05f, 0.23f), -0.23f, 0.23f) + 90.0f, Vector3::RIGHT) *
                                 Quaternion(Sine(0.23f, 178.0f, 182.0f), Vector3::FORWARD));
    leafyLight_->SetBrightness(0.34f + Sine(0.011f, 0.05f, 0.23f) + Sine(0.02f, 0.05f, 0.13f));
}
void MasterControl::UpdateSelectedPiece()
{
    if (!inputMaster_->IsIdle()){
        Piece* nearest{selectedPiece_};
        for (Piece* p: world.pieces_){
            if (!nearest) {
                nearest = p;
            } else if (LucKey::Distance(CAMERA->GetPosition(), p->GetPosition()) <
                       LucKey::Distance(CAMERA->GetPosition(), nearest->GetPosition())
                       && (p->GetState() == PieceState::FREE || p->GetState() == PieceState::SELECTED))
            {
                nearest = p;
            }
        }
        if (nearest != selectedPiece_ || nearest->GetState() != PieceState::SELECTED){
            if (selectedPiece_)
                selectedPiece_->Deselect();

            selectedPiece_ = nearest;
            nearest->Select();
        }
    }
}

void MasterControl::DeselectPiece()
{
    if (selectedPiece_){
        selectedPiece_->Deselect();
    }
    selectedPiece_ = nullptr;
}

void MasterControl::NextPhase()
{
    DeselectPiece();

    switch (gameState_)    {
    case GameState::PLAYER1PICKS: gameState_ = GameState::PLAYER2PUTS;
        break;
    case GameState::PLAYER2PUTS:  gameState_ = GameState::PLAYER2PICKS;
        break;
    case GameState::PLAYER2PICKS: gameState_ = GameState::PLAYER1PUTS;
        break;
    case GameState::PLAYER1PUTS:  gameState_ = GameState::PLAYER1PICKS;
        break;
    default: break;
    }
}
void MasterControl::Quatter()
{
    gameState_ = GameState::QUATTER;
}
void MasterControl::Reset()
{
    for (Piece* p: world.pieces_){
        p->Reset();
    }
    world.board_->Reset();

    gameState_ = GameState::PLAYER1PICKS;
}

void MasterControl::NextMusicState()
{

    if (musicState_ == MUSIC_SONG1){
        effectMaster_->FadeOut(musicSource1_);
    } else if (musicState_ == MUSIC_SONG2){
        effectMaster_->FadeOut(musicSource2_);
    }

    switch (musicState_) {
    case MUSIC_SONG1: case MUSIC_SONG2:{
        previousMusicState_ = musicState_;
        musicState_ = MUSIC_OFF;
    } break;
    case MUSIC_OFF: {
        if (previousMusicState_ == MUSIC_SONG1)
            musicState_ = MUSIC_SONG2;
        else if (previousMusicState_ == MUSIC_SONG2)
            musicState_ = MUSIC_SONG1;
        previousMusicState_ = MUSIC_OFF;
    } break;
    default: {
        previousMusicState_ = MUSIC_OFF;
        musicState_ = MUSIC_SONG1;
    } break;
    }

    if (musicState_ == MUSIC_SONG1){
        effectMaster_->FadeTo(musicSource1_, musicGain_);
    } else if (musicState_ == MUSIC_SONG2){
        effectMaster_->FadeTo(musicSource2_, musicGain_);
    }
}

void MasterControl::MusicGainUp(float step)
{
    musicGain_ = Clamp(musicGain_ + step, step, 1.0f);

    if (musicState_ == MUSIC_SONG1)
        effectMaster_->FadeTo(musicSource1_, musicGain_, 0.23f);
    else if (musicState_ == MUSIC_SONG2)
        effectMaster_->FadeTo(musicSource2_, musicGain_, 0.23f);
}
void MasterControl::MusicGainDown(float step)
{
    musicGain_ = Clamp(musicGain_ - step, 0.0f, 1.0f);

    if (musicState_ == MUSIC_SONG1)
        effectMaster_->FadeTo(musicSource1_, musicGain_, 0.23f);
    else if (musicState_ == MUSIC_SONG2)
        effectMaster_->FadeTo(musicSource2_, musicGain_, 0.23f);
}

float MasterControl::Sine(const float freq, const float min, const float max, const float shift)
{
    float phase{freq * world.scene->GetElapsedTime() + shift};
    float add{0.5f * (min + max)};
    return LucKey::Sine(phase) * 0.5f * (max - min) + add;
}
float MasterControl::Cosine(const float freq, const float min, const float max, const float shift)
{
    float phase{freq * world.scene->GetElapsedTime() + shift};
    float add{0.5f * (min + max)};
    return LucKey::Cosine(phase) * 0.5f * (max - min) + add;
}
