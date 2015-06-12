#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalCharacterPostRenderer.generated.h"

UCLASS()
class AUTBetrayalCharacterPostRenderer : public AActor
{
	GENERATED_UCLASS_BODY()

	AUTCharacter* RenderPawn;

	virtual void BeginPlay() override;
	virtual void PostRenderFor(APlayerController *PC, UCanvas *Canvas, FVector CameraPosition, FVector CameraDir) override;

	virtual bool IsPawnVisible(APlayerController* PC, FVector CameraPosition, ACharacter* P);

	virtual void HookRender();

};