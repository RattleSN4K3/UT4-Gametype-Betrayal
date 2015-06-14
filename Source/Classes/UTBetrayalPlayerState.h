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

	/** Player to betrayal count mapping array (for stats only). Mapped to PlayerID */
	TMap< int32, uint16 > NemesisData;
	/** Player to name mapping array (for stats only). Mapped to PlayerID */
	TMap< int32, FString > NemesisNames;

public:

	/** Current Nemesis for this player  (for stats only) */
	UPROPERTY(Replicated)
	FString CurrentNemesis;

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

	/** Number of times this player has been a betrayer */
	UPROPERTY(Replicated)
	int32 BetrayalCount;

	/** Total sum this player has gained through betrayals (for stats only) */
	UPROPERTY(Replicated)
	uint32 BetrayalPot;

	/** Number of times this player has repayed the betrayal */
	UPROPERTY(Replicated)
	int32 RetributionCount;

	/** Number of times this player has payed back*/
	UPROPERTY(Replicated)
	int32 PaybackCount;

	AUTBetrayalTeam* BetrayedTeam;

	/** Number of times being betrayed (for stats only) */
	UPROPERTY(Replicated)
	int32 BetrayedCount;

	/** Total sum of pot this player has been betrayed for (for stats only) */
	UPROPERTY(Replicated)
	uint32 BetrayedPot;

	/** Value of the highest pot this player has achieved (in team) (for stats only) */
	UPROPERTY(Replicated)
	uint32 HighestPot;

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

	virtual void UpdateNemesis(AUTBetrayalPlayerState* PRI);

};