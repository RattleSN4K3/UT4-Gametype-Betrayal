#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalPlayerState.generated.h"

class AUTBetrayalTeam;

UCLASS()
class AUTBetrayalPlayerState : public AUTPlayerState
{
	GENERATED_UCLASS_BODY()

protected:

	FTimerHandle TimerHandle_RogueTimer;

public:

	/** Remaining rogue time */
	UPROPERTY(Replicated)
	int32 RemainingRogueTime;

	/** Rogue time penalty **/
	UPROPERTY(EditDefaultsOnly)
	int32 RogueTimePenalty;

	/** sound played when rogue time is running out */
	UPROPERTY(EditDefaultsOnly)
	USoundCue* RogueFadingSound;

	/** A rogue is someone who committed a betrayal less than 60 seconds ago, and has suffered retribution yet.
	* A rogue is a target for his betrayal victim(s), and cannot rejoin a team.
	*/
	UPROPERTY(Replicated)
	bool bIsRogue;

	/** Current alliance */
	UPROPERTY(Replicated)
	AUTBetrayalTeam* CurrentTeam;

	/** Last player to betray you */
	UPROPERTY(Replicated)
	AUTBetrayalPlayerState* Betrayer;

	/** FIXME show the number of times this player has been a betrayer on the scoreboard */
	UPROPERTY(Replicated)
	int32 BetrayalCount;

	AUTBetrayalTeam* BetrayedTeam;

	/** How likely bot associated with this PRI is to betray teammates */
	float TrustWorthiness;

	bool bHasSetTrust;

	virtual void Reset() override;

	virtual void SetRogueTimer();
	virtual void RogueTimer();
	virtual void RogueExpired();

	virtual int32 ScoreValueFor(AUTBetrayalPlayerState* OtherPRI);
	virtual float GetTrustWorthiness();

	virtual void UpdateTeam(class AUTBetrayalTeam* Team);
	virtual void ApplyTeamColorFor(AUTCharacter* P, bool bIsTeam);

};