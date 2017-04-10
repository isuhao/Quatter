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

#include "indicator.h"

void Indicator::RegisterObject(Context *context)
{
    context->RegisterFactory<Indicator>();
}

Indicator::Indicator(Context* context) : LogicComponent(context)
{

}

void Indicator::OnNodeSet(Node *node)
{ (void)node;

    glow_ = MC->GetMaterial("Glow")->Clone();
    //Create nodes and models
    arrowNode1_ = node_->CreateChild("Arrow");
    arrowNode1_->Rotate(Quaternion(-90.0f, Vector3::UP));
    model1_ = arrowNode1_->CreateComponent<AnimatedModel>();

    arrowNode2_ = node_->CreateChild("Arrow");
    arrowNode2_->Rotate(Quaternion(90.0f, Vector3::UP));
    model2_ = arrowNode2_->CreateComponent<AnimatedModel>();

    //Create lights
    Node* lightNode1{arrowNode1_->CreateChild("Light")};
    lightNode1->SetPosition(Vector3::UP * 0.23f);
    light1_ = lightNode1->CreateComponent<Light>();
    light1_->SetColor(COLOR_GLOW);
    light1_->SetBrightness(0.023f);
    light1_->SetRange(2.0f);
    light1_->SetCastShadows(false);

    Node* lightNode2{arrowNode2_->CreateChild("Light")};
    lightNode2->SetPosition(Vector3::UP * 0.23f);
    light2_ = lightNode2->CreateComponent<Light>();
    light2_->SetColor(COLOR_GLOW);
    light2_->SetBrightness(0.023f);
    light2_->SetRange(2.0f);
    light2_->SetCastShadows(false);
}

void Indicator::Init(int nth)
{
    arrowNode1_->SetPosition(Vector3::LEFT  * (2.3f + 0.95f * (nth == 2 || nth == 3)) + Vector3::FORWARD * (nth >= 4));
    arrowNode2_->SetPosition(Vector3::RIGHT * (2.3f + 0.95f * (nth == 2 || nth == 3)));

    switch (nth) {
    case 1: case 5:
        node_->Rotate(Quaternion(90.0f, Vector3::UP));
        break;
    case 2:
        node_->Rotate(Quaternion(45.0f, Vector3::UP));
        break;
    case 3:
        node_->Rotate(Quaternion(-45.0f, Vector3::UP));
        break;
    default:
        break;
    }

    if (nth < 4) {

        model1_->SetModel(MC->GetModel("Arrow"));
        model1_->SetMorphWeight(0, static_cast<float>(nth < 2));

        model2_->SetModel(MC->GetModel("Arrow"));
        model2_->SetMorphWeight(0, static_cast<float>(nth < 2));

    } else {

        model1_->SetModel(MC->GetModel("BlockIndicator"));
        model2_->SetModel(MC->GetModel("BlockIndicator"));
    }

    model1_->SetMaterial(glow_);
    model2_->SetMaterial(glow_);
    model2_->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.0f, 0.0f, 0.0f, 0.0f));
}


