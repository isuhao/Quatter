#include "board.h"

Board::Board(): Object(MC->GetContext())
{
    Node* rootNode = MC->world.scene->CreateChild("Board");
    model_ = rootNode->CreateComponent<StaticModel>();
    model_->SetModel(MC->cache_->GetResource<Model>("Resources/Models/Board.mdl"));
    model_->SetMaterial(MC->cache_->GetResource<Material>("Resources/Materials/Board.xml"));
    model_->SetCastShadows(true);
}
