#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalHUD.generated.h"

UCLASS()
class AUTBetrayalHUD : public AUTHUD
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor TextDefaultColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor BackgroundDefaultColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor BackgroundTeamColor;

	virtual void DrawPlayerBeacon(AUTCharacter* P, UCanvas* BeaconCanvas, FVector CameraPosition, FVector CameraDir, FVector ScreenLoc);

};