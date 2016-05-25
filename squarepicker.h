#ifndef SQUAREPICKER_H
#define SQUAREPICKER_H

#include <Urho3D/Urho3D.h>
#include "mastercontrol.h"

using namespace Urho3D;

class SquarePicker : public Object
{
    URHO3D_OBJECT(SquarePicker, Object);
public:
    SquarePicker();
private:
    SharedPtr<Node> node_;
    SharedPtr<StaticModel> model_;
};

#endif // SQUAREPICKER_H
