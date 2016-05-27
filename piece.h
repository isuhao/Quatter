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

#ifndef PIECE_H
#define PIECE_H

#include <Urho3D/Urho3D.h>
#include <bitset>
#include "mastercontrol.h"

namespace Urho3D {
class Node;
}

using namespace Urho3D;

enum class PieceState {FREE, SELECTED, PICKED, PUT};

#define NUM_ATTRIBUTES 4

class Piece : public Object
{
    URHO3D_OBJECT(Piece, Object);
public:
    typedef std::bitset<NUM_ATTRIBUTES> Attributes;
    Piece(Attributes);

    void SetPosition(Vector3 pos) { rootNode_->SetPosition(pos); }
    Vector3 GetPosition() const { return rootNode_->GetPosition(); }
    bool GetAttribute(int index) const { return attributes_[index]; }
    Attributes GetAttributes() const { return attributes_; }
    String GetCodon(int length = NUM_ATTRIBUTES) const;
    float GetAngle() const { return MC->AttributesToAngle(ToInt()); }
    void Select();
    void Deselect();
    PieceState GetState() const noexcept { return state_; }
    void Pick();
    void Put(Vector3 position);
    void Reset();

    int ToInt() const { return static_cast<int>(attributes_.to_ulong()); }
private:
    SharedPtr<Node> rootNode_;
    SharedPtr<StaticModel> outlineModel_;
    SharedPtr<Light> light_;

    Attributes attributes_;
    PieceState state_;
};

#endif // PIECE_H
