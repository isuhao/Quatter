#include "quattercam.h"
#include "board.h"
#include "piece.h"

Piece::Piece(Attributes attributes): Object(MC->GetContext()),
    state_{PieceState::FREE },
    attributes_{attributes},
    sinceStateChange_{0.0f}
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
            Material* glow{outlineModel_->GetMaterial()};
            Material* originalGlow{MC->GetMaterial("Glow")};
            Color glowColor{glow->GetShaderParameter("MatDiffColor").GetColor()};
            Color originalGlowColor{originalGlow->GetShaderParameter("MatDiffColor").GetColor()};
            ValueAnimation* fadeIn_{new ValueAnimation(context_)};
            fadeIn_->SetKeyFrame(0.0f, glowColor);
            fadeIn_->SetKeyFrame(0.23f, originalGlowColor);
            glow->SetShaderParameterAnimation("MatDiffColor", fadeIn_, WM_ONCE);
            outlineModel_->SetEnabled(true);
        }
    }
}
void Piece::Deselect()
{
    if (state_ == PieceState::SELECTED){
        state_ = PieceState::FREE;
        Material* glow{outlineModel_->GetMaterial()};
        Color glowColor{glow->GetShaderParameter("MatDiffColor").GetColor()};
        ValueAnimation* fadeOut_{new ValueAnimation(context_)};
        fadeOut_->SetKeyFrame(0.0f, glowColor);
        fadeOut_->SetKeyFrame(0.23f, glowColor * 0.0f);
        glow->SetShaderParameterAnimation("MatDiffColor", fadeOut_, WM_ONCE);
    }

//    outlineModel_->SetEnabled(false);
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

        //Fade out outline
        Material* glow{outlineModel_->GetMaterial()};
        Color glowColor{glow->GetShaderParameter("MatDiffColor").GetColor()};
        ValueAnimation* fadeOut{new ValueAnimation(context_)};
        fadeOut->SetKeyFrame(0.0f, glowColor);
        fadeOut->SetKeyFrame(0.23f, glowColor * 0.0f);
        glow->SetShaderParameterAnimation("MatDiffColor", fadeOut, WM_ONCE);
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
