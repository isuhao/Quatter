#ifndef BOARD_H
#define BOARD_H

#include <Urho3D/Urho3D.h>
#include "mastercontrol.h"
#include "piece.h"

namespace Urho3D {
class Node;
}

using namespace Urho3D;

class Board : public Object
{
    URHO3D_OBJECT(Board, Object);
public:
    Board();
    float GetHeight() const { return model_->GetBoundingBox().Size().y_; }
private:
    StaticModel* model_;
};

#endif // BOARD_H
