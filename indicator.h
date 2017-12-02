/* Quatter
// Copyright (C) 2017 LucKey Productions (luckeyproductions.nl)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef INDICATOR_H
#define INDICATOR_H

#include <Urho3D/Urho3D.h>

#include "mastercontrol.h"

class Indicator : public LogicComponent
{
    URHO3D_OBJECT(Indicator, LogicComponent);
    friend class Board;

public:
    Indicator(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);

private:
    Node* arrowNode1_;
    Node* arrowNode2_;
    SharedPtr<Material> glow_;
    SharedPtr<AnimatedModel> model1_;
    SharedPtr<AnimatedModel> model2_;
    SharedPtr<Light> light1_;
    SharedPtr<Light> light2_;

    void Init(int nth);
};

#endif // INDICATOR_H
