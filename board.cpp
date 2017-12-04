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

#include "board.h"
#include "piece.h"
#include "indicator.h"
#include "effectmaster.h"

namespace Urho3D {
template <> unsigned MakeHash(const IntVector2& value)
  {
    return LucKey::IntVector2ToHash(value);
  }
}

void Board::RegisterObject(Context *context)
{
    context->RegisterFactory<Board>();
}

Board::Board(Context* context): LogicComponent(context),
    squares_{},
    selectedSquare_{},
    lastSelectedSquare_{},
    indicators_{}
{
}

void Board::OnNodeSet(Node *node)
{ if (!node) return;

    node_->AddTag("Board");
    model_ = node_->CreateComponent<StaticModel>();
    model_->SetModel(MC->GetModel("Board"));
    model_->SetMaterial(MC->GetMaterial("Board"));
    model_->SetCastShadows(true);

    CreateSquares();
    CreateIndicators();

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Board, HandleSceneUpdate));
}

void Board::CreateSquares()
{
    for (int i{0}; i < BOARD_WIDTH; ++i) {
        for (int j{0}; j < BOARD_HEIGHT; ++j) {

            Node* squareNode{ node_->CreateChild("Square", LOCAL) };
            Square* square{ squareNode->CreateComponent<Square>() };
            IntVector2 coords{ i, j };

            square->coords_ = coords;
            squareNode->SetPosition(CoordsToPosition(coords));
            squares_[square->coords_] = square;
        }
    }
}
void Board::CreateIndicators()
{
    for (int i{0}; i < 6; ++i) {

        Node* indicatorNode{ node_->CreateChild("Indicator", LOCAL) };
        indicatorNode->SetPosition(GetThickness() * Vector3::UP);
        Indicator* indicator{ indicatorNode->CreateComponent<Indicator>() };
        indicator->Init(i);

        indicators_.Push(indicator);
    }
}
void Board::Reset()
{
    for (Square* s : squares_.Values()) {

        s->free_ = true;
        s->piece_ = nullptr;
        s->light_->SetEnabled(true);

    }

    Deselect();
}

void Board::Refuse()
{
    if (selectedSquare_) {

        Material* glow{ selectedSquare_->slot_->GetMaterial() };
        glow->SetShaderParameter("MatDiffColor", Color(1.0f, 0.0f, 0.0f, 1.0f));

        if (selectedSquare_->free_)
            FX->FadeTo(glow, COLOR_GLOW, 0.23f);
        else
            FX->FadeTo(glow, Color(1.0f, 0.8f, 0.0f, 0.5f), 0.23f);
    }
}

bool Board::IsEmpty() const
{
    for (Square* s : squares_.Values()) {

        if (!s->free_)
            return false;
    }

    return true;
}
bool Board::IsFull() const
{
    for (Square* s : squares_.Values()) {

        if (s->free_)
            return false;
    }

    return true;
}

Vector3 Board::CoordsToPosition(IntVector2 coords)
{
    return Vector3(0.5f + coords.x_ - BOARD_WIDTH / 2,
                   GetThickness(),
                   0.5f + coords.y_ - BOARD_HEIGHT / 2);
}

void Board::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{ (void)eventType; (void)eventData;

    for (Square* s : squares_.Values()) {

        s->slot_->SetMorphWeight(0, MC->Sine(2.3f, 0.0f, 1.0f));

    }
}

bool Board::PutPiece(Piece* piece, Square* square)
{
    if (!square) {

        return PutPiece(piece);
    }

    if (piece && square->free_) {

        MC->DeselectPiece();

        square->piece_ = piece;
        square->free_ = false;
        square->light_->SetEnabled(false);

        piece->Put(square->GetNode()->GetWorldPosition()
                   + Vector3(Random(-0.05f, 0.05f),
                             0.0f,
                             Random(-0.05f, 0.05f)));

        Deselect();
        lastSelectedSquare_ = nullptr;

        if (CheckQuatter())
            MC->Quatter();

        MC->NextPhase();
        return true;

    } else {

        Refuse();
        return false;
    }
}
bool Board::PutPiece(Piece* piece)
{
    if (!selectedSquare_) {

        SelectLast();
        return false;

    } else {

        return PutPiece(piece, selectedSquare_);
    }
}
bool Board::PutPiece(Square* square)
{
    return PutPiece(MC->GetPickedPiece(), square);
}
bool Board::PutPiece()
{
    return PutPiece(MC->GetPickedPiece());
}

Square* Board::GetNearestSquare(Vector3 pos, bool free)
{
    Square* nearest{};

    for (Square* s : squares_.Values()) {

        if (!nearest
         || LucKey::Distance(s->node_->GetWorldPosition(), pos) <
            LucKey::Distance(nearest->node_->GetWorldPosition(), pos))
        {
            if (s->free_ || !free)
                nearest = s;
        }
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
void Board::SelectLast()
{
    if (lastSelectedSquare_ && lastSelectedSquare_ != selectedSquare_)
        Select(lastSelectedSquare_);
    else if (!selectedSquare_)
        SelectNearestFreeSquare(CAMERA->GetPosition());
}
void Board::Select(Square* square)
{
    Deselect();

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
void Board::Deselect()
{
    HideIndicators();
    if (!selectedSquare_) return;

    //Fade out slot and light
    FX->FadeOut(selectedSquare_->slot_->GetMaterial());
    FX->FadeTo(selectedSquare_->light_, 0.023f);

    lastSelectedSquare_ = selectedSquare_;
    selectedSquare_->selected_ = false;
    selectedSquare_ = nullptr;
}

void Board::Step(IntVector2 step)
{
    if (selectedSquare_) {

        IntVector2 newCoords{ selectedSquare_->coords_ + step };

        if (squares_.Contains(newCoords)){

            Select(squares_[newCoords]);
        }
    } else {

        SelectLast();
    }
}

bool Board::CheckQuatter()
{
    bool checkBlocks{ true };

    //Check rows
    for (int j{0}; j < BOARD_HEIGHT; ++j) {

        Piece::PieceAttributes matching{};
        matching.flip();
        Piece::PieceAttributes first{};

        for (int i{0}; i < BOARD_WIDTH; ++i) {

            IntVector2 coords(i, j);
            Piece* piece{squares_[coords]->piece_};

            if (piece) {

                Piece::PieceAttributes attributes{ piece->GetPieceAttributes() };

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
        if (matching.any()) {

            Indicate(IntVector2(0, j),
                     IntVector2(BOARD_WIDTH - 1, j));
            return true;
        }
    }

    //Check columns
    for (int i{0}; i < BOARD_WIDTH; ++i) {

        Piece::PieceAttributes matching{};
        matching.flip();
        Piece::PieceAttributes first{};

        for (int j{0}; j < BOARD_HEIGHT; ++j) {

            IntVector2 coords{ i, j };
            Piece* piece{ squares_[coords]->piece_ };

            if (piece) {

                Piece::PieceAttributes attributes{ piece->GetPieceAttributes() };

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
        if (matching.any()) {

            Indicate(IntVector2(i, 0),
                     IntVector2(i, BOARD_HEIGHT - 1));
            return true;
        }
    }

    //Check diagonals
    for (bool direction : { true, false }) {

        Piece::PieceAttributes matching{};
        matching.flip();
        Piece::PieceAttributes first{};

        for (int i{0}; i < BOARD_WIDTH; ++i) {

            IntVector2 coords{ i, direction ? i : (BOARD_WIDTH - i - 1) };
            Piece* piece{ squares_[coords]->piece_ };

            if (piece) {

                Piece::PieceAttributes attributes{ piece->GetPieceAttributes() };

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
        if (matching.any()) {

            Indicate(IntVector2( 0,                direction * (BOARD_HEIGHT - 1) ),
                     IntVector2( BOARD_WIDTH - 1, !direction * (BOARD_HEIGHT - 1) ));
            return true;
        }
    }

    //Check 2x2 blocks
    if (checkBlocks) {

        for (int k{0}; k < BOARD_WIDTH - 1; ++k) {
            for (int l{0}; l < BOARD_HEIGHT - 1; ++l) {

                Piece::PieceAttributes matching{};
                matching.flip();
                Piece::PieceAttributes first{};

                for (int m : {0, 1}) for (int n : {0, 1}) {

                    IntVector2 coords{ k + m, l + n };
                    Piece* piece{ squares_[coords]->piece_ };

                    if (piece) {

                        Piece::PieceAttributes attributes{ piece->GetPieceAttributes() };

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
                if (matching.any()) {

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

void Board::Indicate(IntVector2 first, IntVector2 last)
{
    //Indicate single square (for keyboard selection)
    if (last == IntVector2(-1, -1)) {
        FadeInIndicator(indicators_[0], true);
        FX->TransformTo(indicators_[0]->GetNode(),
                CoordsToPosition(first) * Vector3(0.0f, 1.0f, 1.0f),
                indicators_[0]->GetNode()->GetRotation(),
                0.05f);
        indicators_[0]->model1_->SetMorphWeight(1, static_cast<float>(first.y_ > 0 && first.y_ < 3));
        indicators_[0]->model2_->SetMorphWeight(1, static_cast<float>(first.y_ > 0 && first.y_ < 3));

        FadeInIndicator(indicators_[1], true);
        FX->TransformTo(indicators_[1]->GetNode(),
                CoordsToPosition(first) * Vector3(1.0f, 1.0f, 0.0f),
                indicators_[1]->GetNode()->GetRotation(),
                0.05f);
        indicators_[1]->model1_->SetMorphWeight(1, static_cast<float>(first.x_ > 0 && first.x_ < 3));
        indicators_[1]->model2_->SetMorphWeight(1, static_cast<float>(first.x_ > 0 && first.x_ < 3));

        //Indicate row
    } else if (first.y_ == last.y_) {

        FadeInIndicator(indicators_[0]);
        indicators_[0]->GetNode()->SetPosition(CoordsToPosition(first) * Vector3(0.0f, 1.0f, 1.0f));
        indicators_[0]->model1_->SetMorphWeight(1, static_cast<float>(first.y_ > 0 && first.y_ < 3));
        indicators_[0]->model2_->SetMorphWeight(1, static_cast<float>(first.y_ > 0 && first.y_ < 3));

    //Indicate column
    } else if (first.x_ == last.x_) {

        FadeInIndicator(indicators_[1]);
        indicators_[1]->GetNode()->SetPosition(CoordsToPosition(first) * Vector3(1.0f, 1.0f, 0.0f));
        indicators_[1]->model1_->SetMorphWeight(1, static_cast<float>(first.x_ > 0 && first.x_ < 3));
        indicators_[1]->model2_->SetMorphWeight(1, static_cast<float>(first.x_ > 0 && first.x_ < 3));

    //Indicate first diagonal
    } else if (first.x_ == 0 && last.y_ == 0) {

        FadeInIndicator(indicators_[3]);

    //Indicate 2x2 blocks
    } else if (last.x_ - first.x_ == 1) {

        FadeInIndicator(indicators_[4]);
        indicators_[4]->GetNode()->SetPosition(CoordsToPosition(first) * Vector3(0.0f, 1.0f, 1.0f));
        FadeInIndicator(indicators_[5]);
        indicators_[5]->GetNode()->SetPosition(CoordsToPosition(first) * Vector3(1.0f, 1.0f, 0.0f));

    //Indicate other diagonal
    } else

        FadeInIndicator(indicators_[2]);
}
void Board::FadeInIndicator(Indicator* indicator, bool fast)
{
    FX->FadeTo(indicator->glow_, COLOR_GLOW, fast? 0.23f : 2.3f, fast ? 0.0f : 1.0f);
}
void Board::HideIndicators()
{
    for (Indicator* i : indicators_){
        FX->FadeOut(i->glow_);
    }
}
