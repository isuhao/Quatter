/* Quatter
// Copyright (C) 2016 LucKey Productions (luckeyproductions.nl)
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

#include "yad.h"

#define YAD_FULLBRIGHT 0.5f

void Yad::RegisterObject(Context *context)
{
    context->RegisterFactory<Yad>();
}

Yad::Yad(Context* context) : LogicComponent(context),
    model_{},
    material_{},
    light_{},
    hidden_{true},
    dimmed_{false}
{
}

void Yad::OnNodeSet(Node *node)
{ (void)node;

    StringVector tag{}; tag.Push(String("Yad"));
    node_->SetTags(tag);
    Node* lightNode{node_->CreateChild("Light")};
    lightNode->SetPosition(Vector3::UP * 0.23f);
    model_ = node_->CreateComponent<AnimatedModel>();
    model_->SetModel(MC->GetModel("Yad"));
    material_ = MC->GetMaterial("Glow")->Clone();
    model_->SetMaterial(material_);
    light_ = lightNode->CreateComponent<Light>();
    light_->SetLightType(LIGHT_POINT);
    light_->SetCastShadows(true);
    light_->SetColor(COLOR_GLOW);
    light_->SetRange(1.0f);
    light_->SetBrightness(YAD_FULLBRIGHT);

}




