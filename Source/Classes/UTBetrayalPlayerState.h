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

	/** Set when TrustWorthiness is already cached */
	bool bHasSetTrust;

	virtual void Reset() override;

	/** Starts the rogue timer to update the roque remaining time and expired at the given time */
	virtual void SetRogueTimer();
	/** Callback for the roque timer which is called every seconds. Used to also play warning sounds */
	virtual void RogueTimer();
	/** Called once the roque time expires in order to reset specific variables */
	virtual void RogueExpired();

	/** Returns the total score for this player in relation to the passed player */
	virtual int32 ScoreValueFor(AUTBetrayalPlayerState* OtherPRI);
	/** Returns the trust worthiness for this player based on the bot personality */
	virtual float GetTrustWorthiness();

	/** Updates player colors for the current team mates and all other players according to their relation */
	virtual void UpdateTeam(class AUTBetrayalTeam* Team);
	/** Apply player color for the given player */
	virtual void ApplyTeamColorFor(AUTCharacter* P, bool bIsTeam);

	/** Updates the Nemesis propertly stating who is the player who kill this player the mode.
	 *  Using NemesisData and NemesisNames to check for the best player.
	 *
	 * @param PRI The last kill player's PlayerState to update the Nemesis data for
	 * @see NemesisData
	 * @see NemesisNames
	 */
	virtual void UpdateNemesis(AUTBetrayalPlayerState* PRI);

};