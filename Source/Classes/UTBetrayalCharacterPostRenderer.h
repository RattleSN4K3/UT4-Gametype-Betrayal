#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalCharacterPostRenderer.generated.h"

// TODO: Remove class if PostRender works fully reliable
// Class is not directly used/needed if the character class is a subclass of UTBetrayalCharacter

UCLASS()
class AUTBetrayalCharacterPostRenderer : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	AUTCharacter* RenderPawn;

	bool bPawnInitialized;


	UFUNCTION()
	virtual void OnPawnDied(AController* Killer, const UDamageType* DamageType);

	virtual void InitializePawn();

public:

	virtual void PostRenderFor(APlayerController *PC, UCanvas *Canvas, FVector CameraPosition, FVector CameraDir) override;

	virtual void Assign(AUTCharacter* Char);
	virtual bool IsPawnVisible(APlayerController* PC, FVector CameraPosition, ACharacter* P);

};