#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalWeapon.generated.h"

UCLASS()
class AUTBetrayalWeapon : public AUTWeapon
{
	GENERATED_UCLASS_BODY()

	virtual void FireInstantHit(bool bDealDamage = true, FHitResult* OutHit = NULL);

};