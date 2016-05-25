#ifndef PIECE_H
#define PIECE_H

#include <Urho3D/Urho3D.h>
#include "mastercontrol.h"
#include <bitset>

namespace Urho3D {
class Node;
}

using namespace Urho3D;

enum class PieceState {FREE, SELECTED, PUT};

#define NUM_ATTRIBUTES 4


class Piece : public Object
{
    URHO3D_OBJECT(Piece, Object);
public:
    typedef std::bitset<NUM_ATTRIBUTES> Attributes;
    Piece(Attributes);

    void SetPosition(Vector3 pos) { rootNode_->SetPosition(pos); }
    inline bool GetAttribute(int index) const noexcept { return attributes_[index]; }
    inline Attributes GetAttributes() const noexcept { return attributes_; }
    String GetCodon(int length = NUM_ATTRIBUTES) const;
    float GetAngle() const { return MC->AttributesToAngle(static_cast<int>(attributes_.to_ulong())); }
    void Select();
    void Deselect();
private:
    Node* rootNode_;
    SharedPtr<StaticModel> outlineModel_;

    Attributes attributes_;
    PieceState state_;
};

#endif // PIECE_H
