#include "board.h"
#include "effectmaster.h"

namespace Urho3D {
template <> unsigned MakeHash(const IntVector2& value)
  {
    return LucKey::IntVector2ToHash(value);
  }
}

Board::Board(): Object(MC->GetContext()),
    squares_{},
    selectedSquare_{nullptr}
{
    rootNode_ = MC->world.scene->CreateChild("Board");
    model_ = rootNode_->CreateComponent<StaticModel>();
    model_->SetModel(MC->cache_->GetResource<Model>("Resources/Models/Board.mdl"));
    model_->SetMaterial(MC->cache_->GetResource<Material>("Resources/Materials/Board.xml"));
    model_->SetCastShadows(true);

    //Fill board with squares
    for (int i{0}; i < BOARD_HEIGHT; ++i)
        for (int j{0}; j < BOARD_WIDTH; ++j){
            Square* square{new Square};
            square->coords_ = IntVector2(i, j);
            square->node_ = rootNode_->CreateChild("Square");
            square->node_->SetPosition(SquarePosition(square->coords_));
            square->free_ = true;
            square->piece_ = nullptr;

            Node* slotNode{square->node_->CreateChild("Slot")};
            slotNode->SetPosition(Vector3::UP * 0.05f);
            square->slot_ = slotNode->CreateComponent<AnimatedModel>();
            square->slot_->SetModel(MC->GetModel("Slot"));
            square->slot_->SetMaterial(MC->GetMaterial("Glow")->Clone());
            square->slot_->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.0f, 0.0f, 0.0f, 0.0f));

            Node* lightNode{slotNode->CreateChild("Light")};
            lightNode->SetPosition(Vector3::UP * 0.23f);
            square->light_ = square->node_->CreateComponent<Light>();
            square->light_->SetColor(Color(0.0f, 0.8f, 0.5f));
            square->light_->SetBrightness(0.0f);
            square->light_->SetRange(2.0f);
            square->light_->SetCastShadows(true);


            squares_[square->coords_] = square;
            square->node_->SetEnabledRecursive(false);
        }

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Board, HandleSceneUpdate));
}

Vector3 Board::SquarePosition(IntVector2 coords)
{
    return Vector3(0.5f + coords.x_ - BOARD_WIDTH/2,
                   GetThickness(),
                   0.5f + coords.y_ - BOARD_HEIGHT/2);
}

void Board::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    for (Square* s: squares_.Values()){
        s->slot_->SetMorphWeight(0, MC->Sine(2.3f, 0.0f, 1.0f));
    }
}

bool Board::CheckQuatter()
{
    //Check rows
    for (int i{0}; i < BOARD_WIDTH; ++i){
        Piece::Attributes matching{};
        matching.flip();
        Piece::Attributes first{};
        for (int j{0}; j < BOARD_HEIGHT; ++j){
            IntVector2 coords(i, j);
            Piece* piece{squares_[coords].Get()->piece_};
            if (piece){
                Piece::Attributes attributes{piece->GetAttributes()};
                if (j == 0) {
                    first = attributes;
                } else {
                    for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                        if (first[a] != attributes[a])
                            matching[a] = false;
                }
            //Full row required
            } else {
                matching.reset();
                break;
            }
        }
        //Quatter!
        if (matching.any()){
            return true;
        }
    }

    //Check columns
    for (int j{0}; j < BOARD_HEIGHT; ++j){
        Piece::Attributes matching{};
        matching.flip();
        Piece::Attributes first{};
        for (int i{0}; i < BOARD_WIDTH; ++i){
            IntVector2 coords(i, j);
            Piece* piece{squares_[coords].Get()->piece_};
            if (piece){
                Piece::Attributes attributes(piece->GetAttributes());
                if (i == 0) {
                    first = attributes;
                } else {
                    for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                        if (first[a] != attributes[a])
                            matching[a] = false;
                }
            //Full column required
            } else {
                matching.reset();
                break;
            }
        }
        //Quatter!
        if (matching.any()){
            return true;
        }
    }
    //Check diagonals
    for (bool direction : {true, false}){
        Piece::Attributes matching{};
        matching.flip();
        Piece::Attributes first{};
        for (int i{0}; i < BOARD_WIDTH; ++i){
            IntVector2 coords(i, direction ? i : (BOARD_WIDTH - i - 1));
            Piece* piece{squares_[coords].Get()->piece_};
            if (piece){
                Piece::Attributes attributes(piece->GetAttributes());
                if (i == 0) {
                    first = attributes;
                } else {
                    for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                        if (first[a] != attributes[a])
                            matching[a] = false;
                }
                //Full line required
            } else {
                matching.reset();
                break;
            }
        }
        //Quatter!
        if (matching.any()){
            return true;
        }
    }
    //Check 2x2 blocks
    for (int k{0}; k < BOARD_WIDTH - 1; ++k){
        for (int l{0}; l < BOARD_HEIGHT - 1; ++l){
            Piece::Attributes matching{};
            matching.flip();
            Piece::Attributes first{};
            for (int m : {0, 1}) for (int n : {0, 1}){
                IntVector2 coords(k + m, l + n);
                Piece* piece{squares_[coords].Get()->piece_};
                if (piece){
                    Piece::Attributes attributes(piece->GetAttributes());
                    if (m == 0 && n == 0) {
                        first = attributes;
                    } else {
                        for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                            if (first[a] != attributes[a])
                                matching[a] = false;
                    }
                    //Full block required
                } else {
                    matching.reset();
                    break;
                }
            }
            //Quatter!
            if (matching.any()){
                return true;
            }
        }
    }

    //No Quatter
    return false;
}

void Board::PutPiece(Piece* piece, Square* square)
{
    square->piece_ = piece;
    square->free_ = false;

    piece->Put(square->node_->GetWorldPosition()
               + Vector3(Random(-0.05f, 0.05f),
                         0.0f,
                         Random(-0.05f, 0.05f)));
    DeselectAll();
    if (CheckQuatter())
        MC->Quatter();
}
Square* Board::GetNearestFreeSquare(Vector3 pos)
{
    Square* nearest{};
    for (Square* s : squares_.Values()){
        if (!nearest ||
            LucKey::Distance(s->node_->GetWorldPosition(), pos) <
            LucKey::Distance(nearest->node_->GetWorldPosition(), pos))
            if (s->free_)
                nearest = s;
    }
    return nearest;
}
void Board::SelectNearestFreeSquare(Vector3 pos)
{
    Square* square{GetNearestFreeSquare(pos)};
    if (square) Select(square);
}
void Board::Select(Square* square)
{
    if (selectedSquare_)
        Deselect(selectedSquare_);

    selectedSquare_ = square;

    square->node_->SetEnabledRecursive(true);

//    Material* glow{square->slot_->GetMaterial()};
//    Color glowColor{glow->GetShaderParameter("MatDiffColor").GetColor()};

//    Material* originalGlow{MC->GetMaterial("Glow")};
//    Color originalGlowColor{originalGlow->GetShaderParameter("MatDiffColor").GetColor()};

//    ValueAnimation* diffFadeIn{new ValueAnimation(context_)};
//    diffFadeIn->SetKeyFrame(0.0f, glowColor);
//    diffFadeIn->SetKeyFrame(0.23f, originalGlowColor);
//    glow->SetShaderParameterAnimation("MatDiffColor", diffFadeIn, WM_ONCE);

//    ValueAnimation* lightFadeIn{new ValueAnimation(context_)};
//    lightFadeIn->SetKeyFrame(0.0f, square->light_->GetBrightness());
//    lightFadeIn->SetKeyFrame(0.23f, 0.5f);
//    square->light_->SetAttributeAnimation("Brightness Multiplier", lightFadeIn, WM_ONCE);

    MC->effectMaster_->FadeTo(square->slot_->GetMaterial(),
                              MC->GetMaterial("Glow")->GetShaderParameter("MatDiffColor").GetColor());
    MC->effectMaster_->FadeTo(square->light_, 0.42f);
}
void Board::Deselect(Square* square)
{
    if (!square) return;

    if (selectedSquare_ == square)
        selectedSquare_ = nullptr;

    square->selected_ = false;

    MC->effectMaster_->FadeOut(square->slot_->GetMaterial());
    MC->effectMaster_->FadeOut(square->light_);
//    Material* glow{square->slot_->GetMaterial()};
//    Color glowColor{glow->GetShaderParameter("MatDiffColor").GetColor()};

//    ValueAnimation* diffFadeOut{new ValueAnimation(context_)};
//    diffFadeOut->SetKeyFrame(0.0f, glowColor);
//    diffFadeOut->SetKeyFrame(0.23f, glowColor * 0.0f);
//    glow->SetShaderParameterAnimation("MatDiffColor", diffFadeOut, WM_ONCE);

//    ValueAnimation* lightFadeOut{new ValueAnimation(context_)};
//    lightFadeOut->SetKeyFrame(0.0f, square->light_->GetBrightness());
//    lightFadeOut->SetKeyFrame(0.23f, 0.0f);
//    square->light_->SetAttributeAnimation("Brightness Multiplier", lightFadeOut, WM_ONCE);
}
void Board::DeselectAll()
{
    for (Square* s: squares_.Values()){
        Deselect(s);
    }
}
