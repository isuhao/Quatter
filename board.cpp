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

#include "board.h"
#include "effectmaster.h"
#include "quattercam.h"

namespace Urho3D {
template <> unsigned MakeHash(const IntVector2& value)
  {
    return LucKey::IntVector2ToHash(value);
  }
}

Board::Board(): Object(MC->GetContext()),
    indicateSingle_{false},
    squares_{},
    selectedSquare_{nullptr},
    lastSelectedSquare_{nullptr}
{
    rootNode_ = MC->world_.scene_->CreateChild("Board");
    StringVector tag{}; tag.Push(String("Board"));
    rootNode_->SetTags(tag);
    model_ = rootNode_->CreateComponent<StaticModel>();
    model_->SetModel(MC->GetModel("Board"));
    model_->SetMaterial(MC->GetMaterial("Board"));
    model_->SetCastShadows(true);

    //Fill board with squares
    for (int i{0}; i < BOARD_WIDTH; ++i) for (int j{0}; j < BOARD_HEIGHT; ++j){
        //Create base
        Square* square{new Square};
        square->coords_ = IntVector2(i, j);
        square->node_ = rootNode_->CreateChild("Square");
        StringVector tag{}; tag.Push(String("Square"));
        square->node_->SetTags(tag);
        square->node_->SetPosition(CoordsToPosition(square->coords_));
        StaticModel* touchPlane{square->node_->CreateComponent<StaticModel>()};
        touchPlane->SetModel(MC->GetModel("Plane"));
        touchPlane->SetMaterial(MC->GetMaterial("Invisible"));
        square->free_ = true;
        square->piece_ = nullptr;
        //Create slot
        Node* slotNode{square->node_->CreateChild("Slot")};
        slotNode->SetPosition(Vector3::UP * 0.05f);
        square->slot_ = slotNode->CreateComponent<AnimatedModel>();
        square->slot_->SetModel(MC->GetModel("Slot"));
        square->slot_->SetMaterial(MC->GetMaterial("Glow")->Clone());
        square->slot_->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.0f, 0.0f, 0.0f, 0.0f));
        //Create light
        Node* lightNode{slotNode->CreateChild("Light")};
        lightNode->SetPosition(Vector3::UP * 0.23f);
        square->light_ = lightNode->CreateComponent<Light>();
        square->light_->SetColor(0.5f * COLOR_GLOW);
        square->light_->SetBrightness(0.023f);
        square->light_->SetRange(2.0f);
        square->light_->SetCastShadows(false);

        squares_[square->coords_] = square;

    }

    //Create Quatter indicators
    for (int i{0}; i < 6; ++i){
        //Create base
        Indicator* indicator{new Indicator};
        indicator->rootNode_ = rootNode_->CreateChild("Indicator");
        indicator->rootNode_->SetPosition(GetThickness() * Vector3::UP);
        if (i == 1)
            indicator->rootNode_->Rotate(Quaternion(90.0f, Vector3::UP));
        else if (i == 2)
            indicator->rootNode_->Rotate(Quaternion(45.0f, Vector3::UP));
        else if (i == 3)
            indicator->rootNode_->Rotate(Quaternion(-45.0f, Vector3::UP));
        else if (i == 5)
            indicator->rootNode_->Rotate(Quaternion(90.0f, Vector3::UP));

        indicator->glow_ = MC->GetMaterial("Glow")->Clone();
        //Create models
        Node* arrowNode1{indicator->rootNode_->CreateChild("Arrow")};
        arrowNode1->SetPosition(Vector3::LEFT * (2.3f + 0.95f * (i == 2 || i == 3)) + Vector3::FORWARD * (i >= 4));
        arrowNode1->Rotate(Quaternion(-90.0f, Vector3::UP));
        indicator->model1_ = arrowNode1->CreateComponent<AnimatedModel>();
        if (i < 4){
            indicator->model1_->SetModel(MC->GetModel("Arrow"));
            indicator->model1_->SetMorphWeight(0, static_cast<float>(i < 2));
        } else {
            indicator->model1_->SetModel(MC->GetModel("BlockIndicator"));
        }
        indicator->model1_->SetMaterial(indicator->glow_);
        Node* arrowNode2{indicator->rootNode_->CreateChild("Arrow")};
        arrowNode2->SetPosition(Vector3::RIGHT * (2.3f + 0.95f * (i == 2 || i == 3)));
        arrowNode2->Rotate(Quaternion(90.0f, Vector3::UP));
        indicator->model2_ = arrowNode2->CreateComponent<AnimatedModel>();
        if (i < 4){
            indicator->model2_->SetModel(MC->GetModel("Arrow"));
            indicator->model2_->SetMorphWeight(0, static_cast<float>(i < 2));
        } else {
            indicator->model2_->SetModel(MC->GetModel("BlockIndicator"));
        }
        indicator->model2_->SetMaterial(indicator->glow_);
        indicator->model2_->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.0f, 0.0f, 0.0f, 0.0f));
        //Create light
        Node* lightNode1{arrowNode1->CreateChild("Light")};
        lightNode1->SetPosition(Vector3::UP * 0.23f);
        indicator->light1_ = lightNode1->CreateComponent<Light>();
        indicator->light1_->SetColor(COLOR_GLOW);
        indicator->light1_->SetBrightness(0.023f);
        indicator->light1_->SetRange(2.0f);
        indicator->light1_->SetCastShadows(false);
        Node* lightNode2{arrowNode2->CreateChild("Light")};
        lightNode2->SetPosition(Vector3::UP * 0.23f);
        indicator->light2_ = lightNode2->CreateComponent<Light>();
        indicator->light2_->SetColor(COLOR_GLOW);
        indicator->light2_->SetBrightness(0.023f);
        indicator->light2_->SetRange(2.0f);
        indicator->light2_->SetCastShadows(false);

        indicators_.Push(SharedPtr<Indicator>(indicator));

    }

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Board, HandleSceneUpdate));
}
void Board::Reset()
{
    for (Square* s: squares_.Values()){

        s->free_ = true;
        s->piece_ = nullptr;
        s->light_->SetEnabled(true);

    }

    DeselectAll();
}

void Board::Refuse()
{
    if (selectedSquare_){
        Material* glow{selectedSquare_->slot_->GetMaterial()};
        glow->SetShaderParameter("MatDiffColor", Color(1.0f, 0.0f, 0.0f, 1.0f));
        if (selectedSquare_->free_)
            FX->FadeTo(glow, COLOR_GLOW, 0.23f);
        else
            FX->FadeTo(glow,Color(1.0f, 0.8f, 0.0f, 0.5f), 0.23f);
    }
}

bool Board::IsEmpty() const
{
    for (Square* s: squares_.Values())
        if (!s->free_) return false;

    return true;
}
bool Board::IsFull() const
{
    for (Square* s: squares_.Values())
        if (s->free_) return false;

    return true;
}

Vector3 Board::CoordsToPosition(IntVector2 coords)
{
    return Vector3(0.5f + coords.x_ - BOARD_WIDTH/2,
                   GetThickness(),
                   0.5f + coords.y_ - BOARD_HEIGHT/2);
}

void Board::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    for (Square* s: squares_.Values()){

        s->slot_->SetMorphWeight(0, MC->Sine(2.3f, 0.0f, 1.0f));

    }
}

bool Board::PutPiece(Piece* piece, Square* square)
{
    if (!square){
        return PutPiece(piece);
    }

    if (piece && square->free_){

        MC->DeselectPiece();

        square->piece_ = piece;
        square->free_ = false;
        square->light_->SetEnabled(false);

        piece->Put(square->node_->GetWorldPosition()
                   + Vector3(Random(-0.05f, 0.05f),
                             0.0f,
                             Random(-0.05f, 0.05f)));
        DeselectAll();
        if (CheckQuatter())
            MC->Quatter();

        MC->NextPhase();
        return true;

    } else {
        Refuse();
        return false;
    }
}

bool Board::CheckQuatter()
{
    bool checkBlocks{true};

    //Check rows
    for (int j{0}; j < BOARD_HEIGHT; ++j){
        Piece::Attributes matching{};
        matching.flip();
        Piece::Attributes first{};
        for (int i{0}; i < BOARD_WIDTH; ++i){
            IntVector2 coords(i, j);
            Piece* piece{squares_[coords].Get()->piece_};
            if (piece){
                Piece::Attributes attributes(piece->GetAttributes());
                if (i == 0) {
                    first = attributes;
                } else {
                    for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                        if (first[a] != attributes[a])
                            matching[a] = false;
                }
            //Full row required
            } else {
                matching.reset();
                break;
            }
        }
        //Quatter!
        if (matching.any()){
            Indicate(IntVector2(0, j), IntVector2(BOARD_WIDTH - 1, j));
            return true;
        }
    }

    //Check columns
    for (int i{0}; i < BOARD_WIDTH; ++i){
        Piece::Attributes matching{};
        matching.flip();
        Piece::Attributes first{};
        for (int j{0}; j < BOARD_HEIGHT; ++j){
            IntVector2 coords(i, j);
            Piece* piece{squares_[coords].Get()->piece_};
            if (piece){
                Piece::Attributes attributes{piece->GetAttributes()};
                if (j == 0) {
                    first = attributes;
                } else {
                    for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                        if (first[a] != attributes[a])
                            matching[a] = false;
                }
            //Full column required
            } else {
                matching.reset();
                break;
            }
        }
        //Quatter!
        if (matching.any()){
            Indicate(IntVector2(i, 0), IntVector2(i, BOARD_HEIGHT - 1));
            return true;
        }
    }
    //Check diagonals
    for (bool direction : {true, false}){
        Piece::Attributes matching{};
        matching.flip();
        Piece::Attributes first{};
        for (int i{0}; i < BOARD_WIDTH; ++i){
            IntVector2 coords(i, direction ? i : (BOARD_WIDTH - i - 1));
            Piece* piece{squares_[coords].Get()->piece_};
            if (piece){
                Piece::Attributes attributes(piece->GetAttributes());
                if (i == 0) {
                    first = attributes;
                } else {
                    for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                        if (first[a] != attributes[a])
                            matching[a] = false;
                }
                //Full line required
            } else {
                matching.reset();
                break;
            }
        }
        //Quatter!
        if (matching.any()){
            Indicate(IntVector2(0, direction * (BOARD_HEIGHT - 1)),
                     IntVector2(BOARD_WIDTH - 1, !direction * (BOARD_HEIGHT - 1)));
            return true;
        }
    }
    //Check 2x2 blocks
    if (checkBlocks){
        for (int k{0}; k < BOARD_WIDTH - 1; ++k){
            for (int l{0}; l < BOARD_HEIGHT - 1; ++l){
                Piece::Attributes matching{};
                matching.flip();
                Piece::Attributes first{};
                for (int m : {0, 1}) for (int n : {0, 1}){
                    IntVector2 coords(k + m, l + n);
                    Piece* piece{squares_[coords].Get()->piece_};
                    if (piece){
                        Piece::Attributes attributes(piece->GetAttributes());
                        if (m == 0 && n == 0) {
                            first = attributes;
                        } else {
                            for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                                if (first[a] != attributes[a])
                                    matching[a] = false;
                        }
                        //Full block required
                    } else {
                        matching.reset();
                        break;
                    }
                }
                //Quatter!
                if (matching.any()){
                    Indicate(IntVector2(k, l),
                             IntVector2(k + 1, l + 1));
                    return true;
                }
            }
        }
    }

    //No Quatter
    return false;
}

Square* Board::GetNearestSquare(Vector3 pos, bool free)
{
    Square* nearest{};
    for (Square* s : squares_.Values()){
        if (!nearest ||
            LucKey::Distance(s->node_->GetWorldPosition(), pos) <
            LucKey::Distance(nearest->node_->GetWorldPosition(), pos))
            if (s->free_ || !free)
                nearest = s;
    }
    return nearest;
}
void Board::SelectNearestFreeSquare(Vector3 pos)
{
    Square* square{GetNearestSquare(pos, true)};
    if (square) Select(square);
}
void Board::SelectNearestSquare(Vector3 pos)
{
    Square* square{GetNearestSquare(pos, false)};
    if (square) Select(square);
}
bool Board::SelectLast()
{
    if (lastSelectedSquare_ && lastSelectedSquare_ != selectedSquare_) {
        Select(lastSelectedSquare_);
        return true;
    } else if (!selectedSquare_) {
        SelectNearestFreeSquare();
    } else return false;
}

void Board::Select(Square* square)
{
    if (selectedSquare_)
        Deselect(selectedSquare_);

    selectedSquare_ = square;
    square->selected_ = true;

    //Fade in slot and light
    if (square->free_){
        FX->FadeTo(square->slot_->GetMaterial(),
                   COLOR_GLOW);
    } else {
        FX->FadeTo(square->slot_->GetMaterial(),
                   Color(1.0f, 0.8f, 0.0f, 0.5f));
    }

    Indicate(square->coords_);

    FX->FadeTo(square->light_, 0.42f);
}
void Board::Deselect(Square* square)
{
    HideIndicators();
    if (!square) return;

    if (selectedSquare_ == square){
        lastSelectedSquare_ = selectedSquare_;
        selectedSquare_ = nullptr;
    }

    square->selected_ = false;

    //Fade out slot and light
    FX->FadeOut(square->slot_->GetMaterial());
    FX->FadeTo(square->light_, 0.023f);
}
void Board::DeselectAll()
{
    for (Square* s: squares_.Values()){
        Deselect(s);
    }
}

void Board::Step(IntVector2 step)
{
    if (selectedSquare_){
        IntVector2 newCoords{selectedSquare_->coords_ + step};
        if (squares_.Contains(newCoords)){
            Select(squares_[newCoords].Get());
        }
    } else SelectLast();
}

void Board::Indicate(IntVector2 first, IntVector2 last)
{
    //Indicate single square
    if (last == IntVector2(-1, -1)){
        if (indicateSingle_){
            FX->FadeTo(indicators_[0]->glow_, COLOR_GLOW, 2.3f, 1.0f);
            FX->TransformTo(indicators_[0]->rootNode_,
                    CoordsToPosition(first) * Vector3(0.0f, 1.0f, 1.0f),
                    indicators_[0]->rootNode_->GetRotation(),
                    0.23f);
            FX->FadeTo(indicators_[1]->glow_, COLOR_GLOW, 2.3f, 1.0f);
            FX->TransformTo(indicators_[1]->rootNode_,
                    CoordsToPosition(first) * Vector3(1.0f, 1.0f, 0.0f),
                    indicators_[1]->rootNode_->GetRotation(),
                    0.23f);
        }
    //Indicate row
    } else if (first.y_ == last.y_){
        FX->FadeTo(indicators_[0]->glow_, COLOR_GLOW, 2.3f, 1.0f);
        indicators_[0]->rootNode_->SetPosition(CoordsToPosition(first) * Vector3(0.0f, 1.0f, 1.0f));
        indicators_[0]->model1_->SetMorphWeight(1, static_cast<float>(first.y_ > 0 && first.y_ < 3));
        indicators_[0]->model2_->SetMorphWeight(1, static_cast<float>(first.y_ > 0 && first.y_ < 3));
    //Indicate column
    } else if (first.x_ == last.x_){
        FX->FadeTo(indicators_[1]->glow_, COLOR_GLOW, 2.3f, 1.0f);
        indicators_[1]->rootNode_->SetPosition(CoordsToPosition(first) * Vector3(1.0f, 1.0f, 0.0f));
        indicators_[1]->model1_->SetMorphWeight(1, static_cast<float>(first.x_ > 0 && first.x_ < 3));
        indicators_[1]->model2_->SetMorphWeight(1, static_cast<float>(first.x_ > 0 && first.x_ < 3));
    //Indicate first diagonal
    } else if (first.x_ == 0 && last.y_ == 0){
        FX->FadeTo(indicators_[3]->glow_, COLOR_GLOW, 2.3f, 1.0f);
    //Indicate 2x2 blocks
    } else if (last.x_ - first.x_ == 1) {
        FX->FadeTo(indicators_[4]->glow_, COLOR_GLOW, 2.3f, 1.0f);
        indicators_[4]->rootNode_->SetPosition(CoordsToPosition(first) * Vector3(0.0f, 1.0f, 1.0f));
        FX->FadeTo(indicators_[5]->glow_, COLOR_GLOW, 2.3f, 1.0f);
        indicators_[5]->rootNode_->SetPosition(CoordsToPosition(first) * Vector3(1.0f, 1.0f, 0.0f));
    //Indicate other diagonal
    } else
        FX->FadeTo(indicators_[2]->glow_, COLOR_GLOW, 2.3f, 1.0f);
}

void Board::HideIndicators()
{
    for (SharedPtr<Indicator> i: indicators_){
        FX->FadeOut(i.Get()->glow_);
    }
}
