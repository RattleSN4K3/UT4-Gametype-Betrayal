#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalTeam.h"
#include "UTBetrayalCharacterTeamColor.generated.h"

UCLASS()
class AUTBetrayalCharacterTeamColor : public AActor
{
	GENERATED_UCLASS_BODY()

protected:

	bool bRefPawnInitialized;

	uint8 PlayerStateErrorCount;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_RefPawn)
	AUTCharacter* RefPawn;
	UFUNCTION()
	virtual void OnRep_RefPawn();
	virtual void InitializePawn(AUTCharacter* Pawn);

	UFUNCTION()
	virtual void OnRefPawnDied(AController* Killer, const UDamageType* DamageType);
	virtual void UpdateTeamColor();

	virtual void BeginPlay() override;


};