#ifndef MASTER_H
#define MASTER_H

#include <Urho3D/Urho3D.h>
#include "mastercontrol.h"

namespace Urho3D {
class Drawable;
class Node;
class Scene;
class Sprite;
}

using namespace Urho3D;

class Master : public Object
{
    friend class MasterControl;
    URHO3D_OBJECT(Master, Object);
public:
    Master();
protected:

//    virtual void HandleUpdate(StringHash eventType, VariantMap &eventData);
};

#endif // MASTER_H
