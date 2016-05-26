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
    void PutPiece(Piece* piece, Square* square);
    void PutPiece(Piece* piece) { PutPiece(piece, GetSelectedSquare()); }

    Square*GetNearestFreeSquare(Vector3 pos);
    Square* GetSelectedSquare() const { return selectedSquare_; }
    void Select(Square* square);
    void SelectNearestFreeSquare(Vector3 pos);
    void DeselectAll();
private:
    SharedPtr<Node> rootNode_;
    StaticModel* model_;

    HashMap<IntVector2, SharedPtr<Square>> squares_;
    Square* selectedSquare_;
    Vector3 SquarePosition(IntVector2 coords);
    void Deselect(Square* square);
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
};

#endif // BOARD_H
