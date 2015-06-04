#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalWeapon.generated.h"

UCLASS()
class AUTBetrayalWeapon : public AUTWeapon
{
	GENERATED_UCLASS_BODY()

	virtual void FireInstantHit(bool bDealDamage = true, FHitResult* OutHit = NULL);
	
	bool CanAttack_Implementation(AActor* Target, const FVector& TargetLoc, bool bDirectOnly, bool bPreferCurrentMode, UPARAM(ref) uint8& BestFireMode, UPARAM(ref) FVector& OptimalTargetLoc);

};