#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalHUDWidget_GameClock.generated.h"

UCLASS()
class UUTBetrayalHUDWidget_GameClock : public UUTHUDWidget
{
	GENERATED_UCLASS_BODY()

	virtual void InitializeWidget(AUTHUD* Hud);

	virtual bool ShouldDraw_Implementation(bool bShowScores)
	{
		return !bShowScores;
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RenderObject")
	FHUDRenderObject_Texture BackgroundSlate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RenderObject")
	FHUDRenderObject_Texture BackgroundBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RenderObject")
	FHUDRenderObject_Text ClockText;

	// The scale factor to use on the clock when it has to show hours
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RenderObject")
	float AltClockScale;

	UFUNCTION(BlueprintNativeEvent, Category = "RenderObject")
	FText GetClockText();

private:


};