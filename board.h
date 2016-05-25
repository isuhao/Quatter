#ifndef BOARD_H
#define BOARD_H

#include <Urho3D/Urho3D.h>
#include "mastercontrol.h"
#include "piece.h"

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
    float GetThickness() const { return model_->GetBoundingBox().Size().y_; }
    bool CheckQuatter();
    void PutPiece(Piece* piece, IntVector2 coords);
    void PutPiece(Piece* piece) { PutPiece(piece, IntVector2{Random(BOARD_HEIGHT), Random(BOARD_HEIGHT)}); }//GetSelectedSquare().coords_); }
    Node* GetRootNode() const { return rootNode_; }
private:
    SharedPtr<Node> rootNode_;
    StaticModel* model_;

    HashMap<IntVector2, SharedPtr<Square>> squares_;
    Vector3 SquarePosition(IntVector2 coords);
    Square* GetSelectedSquare();

};

#endif // BOARD_H
