#include "quattercam.h"
#include "effectmaster.h"
#include "board.h"
#include "piece.h"

Piece::Piece(Attributes attributes): Object(MC->GetContext()),
    attributes_{attributes},
    state_{PieceState::FREE }
{
    rootNode_ = MC->world.scene->CreateChild("Piece"+GetCodon(4));
    rootNode_->SetRotation(Quaternion(Random(360.0f), Vector3::UP));

    StaticModel* pieceModel{rootNode_->CreateComponent<StaticModel>()};
    pieceModel->SetCastShadows(true);
    pieceModel->SetModel(MC->GetModel("Piece_"+GetCodon(3)));
    if (attributes[3]){
        pieceModel->SetMaterial(MC->GetMaterial("Wood_light"));
    }
    else pieceModel->SetMaterial(MC->GetMaterial("Wood_dark"));

    outlineModel_ = rootNode_->CreateComponent<StaticModel>();
    outlineModel_->SetCastShadows(false);
    outlineModel_->SetModel(MC->GetModel("Piece_"+GetCodon(2)+"_outline"));
    outlineModel_->SetMaterial(MC->GetMaterial("Glow")->Clone());
    outlineModel_->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.0f, 0.0f, 0.0f));
    outlineModel_->SetEnabled(false);

    Node* lightNode{rootNode_->CreateChild("Light")};
    lightNode->SetPosition(Vector3::UP * 0.5f);
    light_ = lightNode->CreateComponent<Light>();
    light_->SetColor(Color(0.0f, 0.8f, 0.5f));
    light_->SetBrightness(0.0f);
    light_->SetRange(3.0f);
}

String Piece::GetCodon(int length) const
{
    if (length > static_cast<int>(attributes_.size()) || length < 1)
        length = static_cast<int>(attributes_.size());

    String codon{};

        codon += attributes_[0] ? "T" : "S"; //Tall  : Short
    if (length > 1)
        codon += attributes_[1] ? "R" : "S"; //Round : Square
    if (length > 2)
        codon += attributes_[2] ? "H" : "S"; //Hole  : Solid
    if (length == NUM_ATTRIBUTES)
        codon += attributes_[3] ? "L" : "D"; //Light : Dark

    return codon;
}

void Piece::Select()
{
    if (MC->GetGamePhase() == GamePhase::PLAYER1PICKS ||
        MC->GetGamePhase() == GamePhase::PLAYER2PICKS )
    {
        outlineModel_->SetEnabled(true);
        if (state_ == PieceState::FREE){
            state_ = PieceState::SELECTED;
            MC->effectMaster_->FadeTo(outlineModel_->GetMaterial(),
                                      MC->GetMaterial("Glow")->GetShaderParameter("MatDiffColor").GetColor());
            MC->effectMaster_->FadeTo(light_, 0.666f);
        }
    }
}
void Piece::Deselect()
{
    if (state_ == PieceState::SELECTED){
        state_ = PieceState::FREE;
        MC->effectMaster_->FadeOut(outlineModel_->GetMaterial());
        MC->effectMaster_->FadeOut(light_);
    }
}
void Piece::Pick()
{
    if (state_ == PieceState::SELECTED){
        state_ = PieceState::PICKED;
        if (MC->GetGamePhase() == GamePhase::PLAYER1PICKS)
            rootNode_->SetParent(CAMERA->GetPocket(false));
        if (MC->GetGamePhase() == GamePhase::PLAYER2PICKS)
            rootNode_->SetParent(CAMERA->GetPocket(true));

        ValueAnimation* intoPocketPos{new ValueAnimation(context_)};
        intoPocketPos->SetKeyFrame(0.0f, rootNode_->GetPosition());
        intoPocketPos->SetKeyFrame(1.0f, Vector3::DOWN);
        rootNode_->SetAttributeAnimation("Position", intoPocketPos, WM_ONCE);

        ValueAnimation* intoPocketRot{new ValueAnimation(context_)};
        intoPocketRot->SetKeyFrame(0.0f, rootNode_->GetRotation());
        intoPocketRot->SetKeyFrame(1.0f, Quaternion(10.0f, Vector3(1.0f, 0.0f, 0.5f)));
        rootNode_->SetAttributeAnimation("Rotation", intoPocketRot, WM_ONCE);

        MC->effectMaster_->FadeOut(outlineModel_->GetMaterial());
        MC->effectMaster_->FadeOut(light_);
    }
}
void Piece::Put(Vector3 position)
{
    if (state_ == PieceState::PICKED){
        state_ = PieceState::PUT;

        rootNode_->SetParent(MC->world.scene);

        ValueAnimation* intoBoardPos{new ValueAnimation(context_)};
        intoBoardPos->SetKeyFrame(0.0f, rootNode_->GetPosition());
        intoBoardPos->SetKeyFrame(1.0f, position);
        rootNode_->SetAttributeAnimation("Position", intoBoardPos, WM_ONCE);

        ValueAnimation* intoBoardRot{new ValueAnimation(context_)};
        intoBoardRot->SetKeyFrame(0.0f, rootNode_->GetRotation());
        intoBoardRot->SetKeyFrame(1.0f, Quaternion(Random(-13.0f, 13.0f), Vector3::UP));
        rootNode_->SetAttributeAnimation("Rotation", intoBoardRot, WM_ONCE);
    }
}
