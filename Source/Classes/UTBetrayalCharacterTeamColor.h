#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalTeam.h"
#include "UTBetrayalCharacterTeamColor.generated.h"

UCLASS()
class AUTBetrayalCharacterTeamColor : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	AUTCharacter* RefPawn;
	APlayerController* RefPC;

	bool bPawnInitialized;

	uint8 PlayerStateErrorCount;

	UFUNCTION()
	virtual void OnPawnDied(AController* Killer, const UDamageType* DamageType);
	
	virtual void InitializePawn();
	virtual void HookPawn();

public:

	virtual void Assign(AUTCharacter* Char, APlayerController *PC);
	virtual void UpdateTeamColor();
	virtual void ApplyTeamColorFor(AUTCharacter* P, bool bIsTeam);

};