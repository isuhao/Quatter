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

class Piece : public Object
{
    URHO3D_OBJECT(Piece, Object);
public:
    Piece(std::bitset<4> attributes);

    void SetPosition(Vector3 pos) { rootNode_->SetPosition(pos); }
    inline bool GetAttribute(int index) const noexcept { return attributes_[index]; }
private:
    Node* rootNode_;
    String modelName_;

    std::bitset<4> attributes_;
    PieceState state_;
};

#endif // PIECE_H
