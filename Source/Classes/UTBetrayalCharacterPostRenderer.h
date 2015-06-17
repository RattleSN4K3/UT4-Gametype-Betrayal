#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalCharacterPostRenderer.generated.h"

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