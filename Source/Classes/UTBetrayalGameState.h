#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalGameState.generated.h"

UCLASS()
class AUTBetrayalGameState : public AUTGameState
{
	GENERATED_UCLASS_BODY()

	/** Determines if a player is on the same team */
	virtual bool OnSameTeam(const AActor* Actor1, const AActor* Actor2) override;

	/** Returns the UTBetrayalPRI (if any) associated with Actor A */
	UFUNCTION(BlueprintCallable, Category = Betrayal)
	virtual class AUTBetrayalPlayerState* GetBetrayalPRIFor(const AActor* A) const;
};