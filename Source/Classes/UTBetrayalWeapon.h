#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalWeapon.generated.h"

// Betrayal Weapon is the main weapon class. The 2 related Blueprint Instagib classes BP_ShockRifle_Betrayal and BP_InstagibRifle_Betrayal
// are plain copies of the non-Betrayal classes where BP_ShockRifle_Betrayal has this class parent class.

// In order to create clean copies...
// 1. Duplicate "ShockRifle" and reparent to UTBetrayalWeapon
// 2. Duplicate BP_InstagibRifle and reparent to BP_ShockRifle_Betrayal
// 3. Change Blueprint methods in BP_ShockRifle_Betrayal to force using red color by defaukt
//    and use blue for the 2nd fire mode

UCLASS()
class AUTBetrayalWeapon : public AUTWeapon
{
	GENERATED_UCLASS_BODY()

	virtual void FireInstantHit(bool bDealDamage = true, FHitResult* OutHit = NULL);
	
	bool CanAttack_Implementation(AActor* Target, const FVector& TargetLoc, bool bDirectOnly, bool bPreferCurrentMode, UPARAM(ref) uint8& BestFireMode, UPARAM(ref) FVector& OptimalTargetLoc);

};