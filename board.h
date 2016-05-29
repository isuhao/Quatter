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

#ifndef BOARD_H
#define BOARD_H

#include <Urho3D/Urho3D.h>

#include "mastercontrol.h"
#include "piece.h"
#include "quattercam.h"

namespace Urho3D {
class Node;
}

using namespace Urho3D;

#define BOARD_WIDTH 4
#define BOARD_HEIGHT 4

typedef class Square : public Object{
    URHO3D_OBJECT(Square, Object);
public:
    Square() : Object(MC->GetContext()) {}
    IntVector2 coords_;
    SharedPtr<Node> node_;
    SharedPtr<AnimatedModel> slot_;
    SharedPtr<Light> light_;
    Piece* piece_;
    bool free_;
    bool selected_;
}Square;

class Board : public Object
{
    URHO3D_OBJECT(Board, Object);
public:
    Board();
    Node* GetRootNode() const { return rootNode_; }

    float GetThickness() const { return model_->GetBoundingBox().Size().y_; }

    bool CheckQuatter();
    bool PutPiece(Piece* piece, Square* square);
    bool PutPiece(Piece* piece) {
        if (!selectedSquare_){
            SelectLast();
            return false;
        } else
            return PutPiece(piece, selectedSquare_);
    }
    bool PutPiece(Square* square) {
        return PutPiece(MC->GetPickedPiece(), square);
    }

    bool PutPiece(){
        return PutPiece(MC->GetPickedPiece());
    }

    void Step(IntVector2 step);
    Vector<SharedPtr<Square>> GetSquares() const { return squares_.Values(); }
    Square* GetNearestSquare(Vector3 pos, bool free = true);
    Square* GetSelectedSquare() const { return selectedSquare_; }
    Square* GetLastSelectedSquare() const { return lastSelectedSquare_; }
    void Select(Square* square);
    void Deselect(Square* square);
    void Deselect() { Deselect(selectedSquare_); }
    void SelectNearestSquare(Vector3 pos = CAMERA->GetPosition());
    void SelectNearestFreeSquare(Vector3 pos = CAMERA->GetPosition());
    bool SelectLast();
    void DeselectAll();
    void Reset();
    void Refuse();

    bool IsEmpty() const;
    bool IsFull() const;
private:
    SharedPtr<Node> rootNode_;
    StaticModel* model_;

    HashMap<IntVector2, SharedPtr<Square>> squares_;
    Square* selectedSquare_;
    Square* lastSelectedSquare_;
    Vector3 CoordsToPosition(IntVector2 coords);
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
};

#endif // BOARD_H
