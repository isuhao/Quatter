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

#include "square.h"
#include "piece.h"

void Square::RegisterObject(Context *context)
{
    context->RegisterFactory<Square>();
}

Square::Square(Context* context) : LogicComponent(context)
{
}

void Square::OnNodeSet(Node *node)
{ if (!node) return;

    node_->AddTag("Square");

    StaticModel* touchPlane{ node_->CreateComponent<StaticModel>() };
    touchPlane->SetModel(MC->GetModel("Plane"));
    touchPlane->SetMaterial(MC->GetMaterial("Invisible"));
    free_ = true;
    piece_ = nullptr;

    //Create slot
    Node* slotNode{ node_->CreateChild("Slot") };
    slotNode->SetPosition(Vector3::UP * 0.05f);
    slot_ = slotNode->CreateComponent<AnimatedModel>();
    slot_->SetModel(MC->GetModel("Slot"));
    slot_->SetMaterial(MC->GetMaterial("Glow")->Clone());
    slot_->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.0f, 0.0f, 0.0f, 0.0f));

    //Create light
    Node* lightNode{ slotNode->CreateChild("Light") };
    lightNode->SetPosition(Vector3::UP * 0.23f);
    light_ = lightNode->CreateComponent<Light>();
    light_->SetColor(0.5f * COLOR_GLOW);
    light_->SetBrightness(0.023f);
    light_->SetRange(2.0f);
    light_->SetCastShadows(false);
}
