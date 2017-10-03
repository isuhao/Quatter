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

#ifndef MASTERCONTROL_H
#define MASTERCONTROL_H

#include <Urho3D/Urho3D.h>
#include "luckey.h"

namespace Urho3D {
class Node;
class Scene;
}

using namespace Urho3D;

class QuatterCam;
class InputMaster;
class EffectMaster;
class Board;
class Piece;

enum class GameState{ PLAYER1PICKS, PLAYER2PUTS, PLAYER2PICKS, PLAYER1PUTS, QUATTER };
enum MusicState{ MUSIC_SONG1, MUSIC_SONG2, MUSIC_OFF };
enum SelectionMode{ SM_CAMERA, SM_STEP, SM_YAD };

typedef struct GameWorld
{
    SharedPtr<QuatterCam> camera_;
    SharedPtr<Scene> scene_;
    SharedPtr<Board> board_;
    Vector< SharedPtr<Piece> > pieces_;
} GameWorld;

#define MC MasterControl::GetInstance()
#define FX GetSubsystem<EffectMaster>()
#define CAMERA MC->world_.camera_
#define BOARD MC->world_.board_
#define NUM_PIECES 16
#define TABLE_DEPTH 0.21f
#define RESET_DURATION 1.23f

#define COLOR_GLOW MC->GetMaterial("Glow")->GetShaderParameter("MatDiffColor").GetColor()

class MasterControl : public Application
{
    URHO3D_OBJECT(MasterControl, Application);
    friend class InputMaster;

public:
    MasterControl(Context* context);
    static MasterControl* GetInstance();
    String GetResourceFolder() const { return resourceFolder_; }

    GameWorld world_;

    void Setup() override;
    void Start() override;
    void Stop() override;

    void Exit();
    void CreateLights();

    inline GameState GetGameState() const noexcept { return gameState_; }
    inline GameState GetPreviousGameState() const noexcept { return previousGameState_; }
    inline bool InPickState() const noexcept { return gameState_ == GameState::PLAYER1PICKS || gameState_ == GameState::PLAYER2PICKS; }
    inline bool InPutState() const noexcept { return gameState_ == GameState::PLAYER1PUTS || gameState_ == GameState::PLAYER2PUTS; }
    inline bool InPlayer1State() const noexcept { return gameState_ == GameState::PLAYER1PICKS || gameState_ == GameState::PLAYER1PUTS; }
    inline bool InPlayer2State() const noexcept { return gameState_ == GameState::PLAYER2PICKS || gameState_ == GameState::PLAYER2PUTS; }

    void NextPhase();
    void NextSelectionMode();
    void SetSelectionMode(SelectionMode mode);
    void NextMusicState();
    void TakeScreenshot();

    float AttributesToAngle(int attributes) const { return (360.0f/NUM_PIECES * attributes) + 180.0f / NUM_PIECES + 23.5f; }
    Vector3 AttributesToPosition(int attributes) const {
        return Quaternion(AttributesToAngle(attributes), Vector3::UP) * Vector3::BACK * 7.0f
                + Vector3::DOWN * TABLE_DEPTH;
    }

    Material* GetMaterial(String name) const { return CACHE->GetResource<Material>("Materials/" + name + ".xml"); }
    Model* GetModel(String name) const { return CACHE->GetResource<Model>("Models/" + name + ".mdl"); }
    Texture* GetTexture(String name) const { return CACHE->GetResource<Texture>("Textures/" + name + ".png"); }
    Sound* GetMusic(String name) const;
    Sound* GetSample(String name) const;

    void Quatter();
    void SetPickedPiece(Piece* piece) { pickedPiece_ = piece; }
    Piece* GetSelectedPiece() const { return selectedPiece_; }
    Piece* GetPickedPiece() const { return pickedPiece_; }
    void DeselectPiece();

    float Sine(const float freq, const float min = -1.0f, const float max = 1.0f, const float shift = 0.0f);
    float Cosine(const float freq, const float min = -1.0f, const float max = 1.0f, const float shift = 0.0f);

private:
    static MasterControl* instance_;
    String resourceFolder_;

    SharedPtr<Node> leafyLightNode_;
    SharedPtr<Light> leafyLight_;

    SharedPtr<SoundSource> musicSource1_;
    SharedPtr<SoundSource> musicSource2_;
    float musicGain_;

    GameState gameState_;
    GameState previousGameState_;
    GameState startGameState_;
    MusicState musicState_;
    MusicState previousMusicState_;
    SelectionMode selectionMode_;

    Piece* selectedPiece_;
    Piece* lastSelectedPiece_;
    Piece* pickedPiece_;

    void CreateScene();
    void Reset();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void CameraSelectPiece(bool force = false);
    void StepSelectPiece(bool next);
    void SelectPrevious();

    void MusicGainUp(float step);
    void MusicGainDown(float step);

    void SelectPiece(Piece* piece);
    bool SelectLastPiece();

    float lastReset_;
    bool IsLame() { return TIME->GetElapsedTime() - lastReset_ < (RESET_DURATION + 0.23f); }
    void CreateJukebox();
    void CreateSkybox();
    void CreateTable();
    void CreateBoardAndPieces();
};

#endif // MASTERCONTROL_H
