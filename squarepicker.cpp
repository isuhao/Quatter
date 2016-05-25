#include "squarepicker.h"

SquarePicker::SquarePicker() : Object(MC->GetContext())
{
    node_ = MC->world.scene->CreateChild("SquarePicker");
}





