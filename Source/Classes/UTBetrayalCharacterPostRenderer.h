#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalCharacterPostRenderer.generated.h"

UCLASS()
class AUTBetrayalCharacterPostRenderer : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	bool bRenderPawnInitialized;

	virtual void BeginPlay() override;
	virtual void PostRenderFor(APlayerController *PC, UCanvas *Canvas, FVector CameraPosition, FVector CameraDir) override;


	virtual bool IsPawnVisible(APlayerController* PC, FVector CameraPosition, ACharacter* P);

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_RenderPawn)
	AUTCharacter* RenderPawn;
	UFUNCTION()
	virtual void OnRep_RenderPawn();

	virtual void HookRender();

	UFUNCTION()
	virtual void OnPawnDied(AController* Killer, const UDamageType* DamageType);

};