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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Message)
	FText BeaconBonusString;

	virtual void DrawPlayerBeacon(AUTCharacter* P, UCanvas* BeaconCanvas, FVector CameraPosition, FVector CameraDir, FVector ScreenLoc);


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UFont* BeaconFont;

	// TODO: Use from UTHUD

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UT3 HUD")
	UTexture* UT3GHudTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UT3 HUD")
	FTextureUVs BeaconTextureUV;

};