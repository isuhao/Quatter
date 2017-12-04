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

#ifndef BOARD_H
#define BOARD_H

#include <Urho3D/Urho3D.h>

#include "mastercontrol.h"
#include "square.h"
#include "quattercam.h"

namespace Urho3D {
class Node;
}

using namespace Urho3D;

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 4

class Piece;
class Indicator;

#define BOARD MC->world_.board_

class Board : public LogicComponent
{
    URHO3D_OBJECT(Board, LogicComponent);

public:
    Board(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);

    float GetThickness() const { return model_->GetBoundingBox().Size().y_; }

    bool PutPiece(Piece* piece, Square* square);
    bool PutPiece(Piece* piece);
    bool PutPiece(Square* square);
    bool PutPiece();

    bool CheckQuatter();

    void Step(IntVector2 step);
    Vector<Square*> GetSquares() const { return squares_.Values(); }
    Square* GetNearestSquare(Vector3 pos, bool free = true);
    Square* GetSelectedSquare() const { return selectedSquare_; }
    Square* GetLastSelectedSquare() const { return lastSelectedSquare_; }
    void Select(Square* square);
    void Deselect();
    void SelectNearestSquare(Vector3 pos);
    void SelectNearestFreeSquare(Vector3 pos);
    void SelectLast();
    void Reset();
    void Refuse();

    bool IsEmpty() const;
    bool IsFull() const;
    void HideIndicators();

private:
    StaticModel* model_;

    HashMap<IntVector2, Square*> squares_;
    Square* selectedSquare_;
    Square* lastSelectedSquare_;
    Vector<Indicator*> indicators_;
    Vector3 CoordsToPosition(IntVector2 coords);
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
    void Indicate(IntVector2 first, IntVector2 last = IntVector2(-1, -1));
    void CreateSquares();
    void CreateIndicators();
    void FadeInIndicator(Indicator* indicator, bool fast = false);
};

#endif // BOARD_H
