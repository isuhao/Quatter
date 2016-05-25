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

typedef struct Square{
    IntVector2 coords_;
    SharedPtr<Node> node_;
    SharedPtr<Light> light_;
    Piece* piece_;
}Square;

class Board : public Object
{
    URHO3D_OBJECT(Board, Object);
public:
    Board();
    float GetThickness() const { return model_->GetBoundingBox().Size().y_; }
    bool CheckQuatter();
private:
    StaticModel* model_;

    HashMap<IntVector2, Square> squares_;
    Vector3 SquarePosition(IntVector2 coords);
};

#endif // BOARD_H
