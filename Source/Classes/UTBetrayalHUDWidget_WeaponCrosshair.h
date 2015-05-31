#pragma once

#include "UTBetrayal.h"
#include "UTHUDWidget_WeaponCrosshair.h"
#include "UTBetrayalHUDWidget_WeaponCrosshair.generated.h"

UCLASS()
class UUTBetrayalHUDWidget_WeaponCrosshair : public UUTHUDWidget_WeaponCrosshair
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Draw_Implementation(float DeltaTime) override;

};