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
    URHO3D_OBJECT(Master, Object);
public:
    Master(Context *context, MasterControl* masterControl);
protected:
    MasterControl* masterControl_;

//    virtual void HandleUpdate(StringHash eventType, VariantMap &eventData);
};

#endif // MASTER_H
