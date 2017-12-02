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

#include "quattercam.h"

#include "guimaster.h"

GUIMaster::GUIMaster(Context* context) : Master(context),
    logoSprite_{},
    activeSide_{0}
{
    Init3DGUI();
    CreateLogo();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GUIMaster, HandleUpdate));
}

void GUIMaster::Init3DGUI()
{
    Node* tableNode_{ MC->world_.tableNode_ };

    for (int s{0}; s < 4; ++s) {

        faces_.Push(tableNode_->CreateChild("MenuPlane"));

        faces_[s]->SetPosition(Quaternion(-90.0f * (s + 2), Vector3::UP) * Vector3(0.0f, -13.0f, 9.5f));
        faces_[s]->SetRotation(Quaternion(-90.0f * (s + 2), Vector3::UP) * Quaternion(180.0f, Vector3::FORWARD) * Quaternion(90.0f, Vector3::RIGHT));
        faces_[s]->SetScale({16.0f, 0.1f, 16.0f});
        faces_[s]->CreateComponent<StaticModel>()->SetModel(MC->GetModel("Plane"));

        UIComponent* uiComponent{ faces_[s]->CreateComponent<UIComponent>() };
        uiComponent->GetMaterial()->SetTechnique(0, CACHE->GetResource<Technique>("Techniques/DiffAdd.xml"));
        UIElement* uiRoot{ uiComponent->GetRoot() };
        uiRoot->SetDefaultStyle(CACHE->GetResource<XMLFile>("UI/DefaultStyle.xml"));


        switch (s) {
        case 0: {
            // Create the Window and add it to the UI's root node
            uiRoot->SetSize(2048, 2048);

            uiRoot->SetLayout(LM_FREE);
            // Set Window size and layout settings
            //            window->SetMinWidth(1024);
            //            window->SetMinHeight(1024);
            //            window->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
            //            window->SetAlignment(HA_CENTER, VA_CENTER);
            //            window->SetName("Window");

            //            window->SetStyleAuto();

                for (int b{0}; b < 5; ++b) {

                    Button* button{ new Button(context_) };
                    uiRoot->AddChild(button);

                    String name{};
                    switch (b) {
                    case 0:
                        name = "Multi";
                        break;
                    case 1:
                        name = "Local";
                        break;
                    case 2:
                        name = "Settings";
                        break;
                    case 3:
                        name = "Exit";
                        break;
                    case 4:
                        name = "AI";
                        break;
                    default:
                        break;
                    }
                    button->SetName(name);
                    button->SetTexture(CACHE->GetResource<Texture2D>("Textures/UI/" + name + ".png"));
                    button->SetAlignment(HA_CENTER, VA_CENTER);
                    button->SetPosition(IntVector2::RIGHT * uiRoot->GetWidth() / 23 + uiRoot->GetSize() * 0.5f + VectorFloorToInt(Vector2(1.0f, 0.8f) * LucKey::Rotate(Vector2::LEFT * uiRoot->GetHeight() * 0.34f, b * 360.0f / 5)));
                    button->SetSize(button->GetTexture()->GetWidth(), button->GetTexture()->GetHeight());
                    button->SetColor(C_TOPLEFT, COLOR_GLOW);

                    SubscribeToEvent(button, E_CLICK, URHO3D_HANDLER(GUIMaster, HandleClick));
                }

        } break;
        default: {}
            break;
        }



//        // Create Window 'titlebar' container
//        UIElement* titleBar = new UIElement(context_);
//        titleBar->SetMinSize(0, 24);
//        titleBar->SetVerticalAlignment(VA_TOP);
//        titleBar->SetLayoutMode(LM_HORIZONTAL);

//        // Create the Window title Text
//        Text* windowTitle = new Text(context_);
//        windowTitle->SetName("WindowTitle");
//        windowTitle->SetText("Hello GUI!");

//        // Create the Window's close button
//        Button* buttonClose = new Button(context_);
//        buttonClose->SetName("CloseButton");

//        // Add the controls to the title bar
//        titleBar->AddChild(windowTitle);
//        titleBar->AddChild(buttonClose);

//        // Add the title bar to the Window
//        window->AddChild(titleBar);

//        windowTitle->SetStyleAuto();
//        buttonClose->SetStyle("CloseButton");

    }
}

void GUIMaster::CreateLogo()
{
    Texture2D* logoTexture{ CACHE->GetResource<Texture2D>("Textures/Logo.png") };
    if (!logoTexture)
        return;

    logoSprite_ = GUI->GetRoot()->CreateChild<Sprite>();

    // Set logo sprite texture
    logoSprite_->SetTexture(logoTexture);

    int textureWidth{ logoTexture->GetWidth() };
    int textureHeight{ logoTexture->GetHeight() };

    logoSprite_->SetScale(Min(1.0f, (GRAPHICS->GetHeight() / static_cast<float>(textureHeight)) * 0.42f));
    logoSprite_->SetSize(textureWidth, textureHeight);
    logoSprite_->SetHotSpot(textureWidth / 2, 0);
    logoSprite_->SetAlignment(HA_CENTER, VA_TOP);
    logoSprite_->SetOpacity(0.99f);
    logoSprite_->SetPriority(-100);
}

void GUIMaster::SetLogoVisible(bool enable)
{
    logoSprite_->SetVisible(enable);
    INPUT->SetMouseVisible(enable);

    for (Node* face : faces_) {

        face->SetEnabled(enable);
    }
}

void GUIMaster::HandleUpdate(StringHash eventType, VariantMap& eventData)
{ (void)eventType;

    for (Node* node : faces_) {
        UIComponent* uiComponent{ node->GetComponent<UIComponent>() };
        UIElement* uiRoot{ uiComponent->GetRoot() };

        for (SharedPtr<UIElement> el : uiRoot->GetChildren()) {

            for (int c{0}; c < 4; ++c)
                el->SetColor(static_cast<Corner>(c), (el->GetColor(static_cast<Corner>(c)) + COLOR_GLOW * Random(0.23f, 2.3f)) * 0.5f);
        }
    }
}

void GUIMaster::HandleClick(StringHash eventType, VariantMap& eventData)
{ (void)eventType;

    UIElement* element{ static_cast<UIElement*>(eventData[Click::P_ELEMENT].GetPtr()) };

    if (!element)
        return;

    if (element->GetName() == "Exit")
        MC->Exit();
    else if (element->GetName() == "Settings")
        activeSide_ = 1;
    else if (element->GetName() == "Multi")
        activeSide_ = 3;
    else if (element->GetName() == "Local"){
        MC->world_.camera_->TargetBoard();
        MC->Reset();
    }
}
