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
    gamePhase_{GamePhase::PLAYER1PICKS}
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

    engineParameters_["FullScreen"] = false;
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
    Sound* music{cache_->GetResource<Sound>("Resources/Music/Angelight - The Knowledge River.ogg")};
    music->SetLooped(true);
    Node* musicNode{world.scene->CreateChild("Music")};
    musicSource_ = musicNode->CreateComponent<SoundSource>();
    musicSource_->SetSoundType(SOUND_MUSIC);
    musicSource_->SetGain(musicGain_);
    musicSource_->Play(music);

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
//    tableNode->SetScale(19.0f);
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

    //Add main light source
    leafyLightNode_ = world.scene->CreateChild("DirectionalLight");
    leafyLightNode_->SetPosition(Vector3(6.0f, 96.0f, 9.0f));
    leafyLightNode_->LookAt(Vector3(0.0f, 0.0f, 0.0f));
    leafyLight_ = leafyLightNode_->CreateComponent<Light>();
    leafyLight_->SetLightType(LIGHT_SPOT);
    leafyLight_->SetRange(180.0f);
    leafyLight_->SetFov(34.0f);
    leafyLight_->SetCastShadows(false);
    leafyLight_->SetShadowIntensity(0.23f);
    leafyLight_->SetShapeTexture(static_cast<Texture*>(cache_->GetResource<Texture2D>("Textures/LeafyMask.png")));
    leafyLight_->SetShadowBias(BiasParameters(0.000025f, 0.5f));
    leafyLight_->SetShadowCascade(CascadeParameters(5.0f, 7.0f, 23.0f, 42.0f, 0.8f));
//    leafyLight_->SetShadowCascade(CascadeParameters(64.0f, 86.0f, 128.0f, 192.0f, 0.8f));
//    leafyLight_->SetShadowCascade(CascadeParameters(5.0f, 7.0f, 23.0f, 42.0f, 0.8f));

    //Add a directional light to the world. Enable cascaded shadows on it
    Node* downardsLightNode{world.scene->CreateChild("DirectionalLight")};
    downardsLightNode->SetPosition(Vector3(2.0f, 23.0f, 3.0f));
    downardsLightNode->LookAt(Vector3(0.0f, 0.0f, 0.0f));
    Light* downwardsLight{downardsLightNode->CreateComponent<Light>()};
    downwardsLight->SetLightType(LIGHT_DIRECTIONAL);
    downwardsLight->SetBrightness(0.34f);
    downwardsLight->SetColor(Color(0.8f, 0.9f, 0.95f));
    downwardsLight->SetCastShadows(true);
//    downwardsLight->SetShadowIntensity(0.23f);
    downwardsLight->SetShadowBias(BiasParameters(0.000025f, 0.5f));
    downwardsLight->SetShadowCascade(CascadeParameters(5.0f, 7.0f, 23.0f, 42.0f, 0.8f));

    //Create a point lights.
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

void MasterControl::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
//    float t{eventData[Update::P_TIMESTEP].GetFloat()};
    if (!inputMaster_->IsIdle()){
        for (Piece* p: world.pieces_){
            if (LucKey::Delta(CAMERA->GetYaw(), p->GetAngle(), true) < 180.0f / NUM_PIECES){
                p->Select();
            } else if (p->GetState() != PieceState::PICKED){
                p->Deselect();
            }
        }
    }
    //Wave leafy light
    leafyLightNode_->SetRotation(Quaternion(Sine(Sine(0.1f, 0.05f, 0.23f), -0.23f, 0.23f) + 90.0f, Vector3::RIGHT) *
                                 Quaternion(Sine(0.23f, 178.0f, 182.0f), Vector3::FORWARD));
    leafyLight_->SetBrightness(0.34f + Sine(0.011f, 0.05f, 0.23f) + Sine(0.02f, 0.05f, 0.13f));
}

void MasterControl::NextPhase()
{
    switch (gamePhase_)    {
    case GamePhase::PLAYER1PICKS: gamePhase_ = GamePhase::PLAYER2PUTS;
        break;
    case GamePhase::PLAYER2PUTS:  gamePhase_ = GamePhase::PLAYER2PICKS;
        break;
    case GamePhase::PLAYER2PICKS: gamePhase_ = GamePhase::PLAYER1PUTS;
        break;
    case GamePhase::PLAYER1PUTS:  gamePhase_ = GamePhase::PLAYER1PICKS;
        break;
    }
}
void MasterControl::Quatter()
{
    gamePhase_ = GamePhase::QUATTER;
}

void MasterControl::ToggleMusic()
{
    ValueAnimation* fade{musicSource_->GetAttributeAnimation("Gain")};
    float fadeEndValue{-1.0f};
    if (fade)
        fadeEndValue = fade->GetAnimationValue(fade->GetEndTime()).GetFloat();

    if (musicSource_->GetGain() == 0.0f || fadeEndValue == 0.0f){
        ValueAnimation* fadeIn_{new ValueAnimation(context_)};
        fadeIn_->SetKeyFrame(0.0f, 0.0f);
        fadeIn_->SetKeyFrame(1.0f, 0.5f * musicGain_);
        fadeIn_->SetKeyFrame(2.3f, Min(musicGain_, 0.1f));
        musicSource_->SetAttributeAnimation("Gain", fadeIn_, WM_ONCE);
    }
    else{
        float lastGain_{musicSource_->GetGain()};
        ValueAnimation* fadeOut_ = new ValueAnimation(context_);
        fadeOut_->SetKeyFrame(0.0f, lastGain_);
        fadeOut_->SetKeyFrame(1.0f, 0.5f * lastGain_);
        fadeOut_->SetKeyFrame(2.3f, 0.1f * lastGain_);
        fadeOut_->SetKeyFrame(5.0f, 0.0f);
        musicSource_->SetAttributeAnimation("Gain", fadeOut_, WM_ONCE);
    }
}

void MasterControl::MusicGainUp(float step)
{
    musicGain_ = Clamp(musicGain_ + step, step, 1.0f);

    ValueAnimation* fadeIn_{new ValueAnimation(context_)};
    fadeIn_->SetKeyFrame(0.0f, musicSource_->GetGain());
    fadeIn_->SetKeyFrame(0.23f, musicGain_);
    musicSource_->SetAttributeAnimation("Gain", fadeIn_, WM_ONCE);
}
void MasterControl::MusicGainDown(float step)
{
    musicGain_ = Clamp(musicGain_ - step, 0.0f, 1.0f);

    ValueAnimation* fadeOut_{new ValueAnimation(context_)};
    fadeOut_->SetKeyFrame(0.0f, musicSource_->GetGain());
    fadeOut_->SetKeyFrame(0.23f, musicGain_);
    musicSource_->SetAttributeAnimation("Gain", fadeOut_, WM_ONCE);
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
