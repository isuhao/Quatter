#include "board.h"

Board::Board(Context* context, MasterControl *masterControl): Object(context),
    masterControl_{masterControl}
{
    Node* rootNode = masterControl_->world.scene->CreateChild("Board");
    model_ = rootNode->CreateComponent<StaticModel>();
    model_->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Board.mdl"));
    model_->SetMaterial(masterControl_->cache_->GetResource<Material>("Resources/Materials/Board.xml"));
    model_->SetCastShadows(true);
}
