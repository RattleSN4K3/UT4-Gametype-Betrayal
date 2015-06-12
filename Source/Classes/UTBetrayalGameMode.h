#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalTeam.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalGameMode.generated.h"

UCLASS()
class AUTBetrayalGameMode : public AUTDMGameMode
{
	GENERATED_UCLASS_BODY()

protected:

	FTimerHandle TimerHandle_MaybeStartTeam;

public:

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<AUTBetrayalTeam*> Teams;

	// TODO: Use UTWeap_InstagibRifle as field type
	UPROPERTY(EditDefaultsOnly, Category = Game)
	TSubclassOf<AUTWeapon> InstagibRifleClass;

	UPROPERTY(EditDefaultsOnly, Category = Game)
	TSubclassOf<AUTBetrayalTeam> TeamClass;

	/** Score bonus for killing Rogue that betrayed you */
	UPROPERTY(EditDefaultsOnly, Category = Betrayal)
	int32 RogueValue;

	/** Class for announcement messages related to Betrayal */
	UPROPERTY(EditDefaultsOnly, Category = Game)
	TSubclassOf<class UUTLocalMessage> AnnouncerMessageClass;

	/** Sounds for Betrayal events */
	UPROPERTY(EditDefaultsOnly, Category = Game)
	USoundCue* BetrayingSound;
	UPROPERTY(EditDefaultsOnly, Category = Game)
	USoundCue* BetrayedSound;
	UPROPERTY(EditDefaultsOnly, Category = Game)
	USoundCue* JoinTeamSound;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginGame() override;

	/** checks whether the mutator is allowed in this gametype and doesn't conflict with any existing mutators */
	virtual bool AllowMutator(TSubclassOf<AUTMutator> MutClass) override;

	/** used to modify, remove, and replace Actors. Return false to destroy the passed in Actor. Default implementation queries mutators.
	* note that certain critical Actors such as PlayerControllers can't be destroyed, but we'll still call this code path to allow mutators
	* to change properties on them
	*/
	virtual bool CheckRelevance_Implementation(AActor* Other) override;

	virtual void ShotTeammate(AUTBetrayalPlayerState* InstigatorPRI, AUTBetrayalPlayerState* HitPRI, APawn* ShotInstigator, APawn* HitPawn);
	virtual void RemoveFromTeam(AUTBetrayalPlayerState* PRI);
	virtual void RemoveTeam(AUTBetrayalTeam* Team);
	virtual void MaybeStartTeam();

	virtual void Logout(AController* Exiting) override;
	virtual void ScoreKill(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType) override;

	// TODO: TEMP. Remove once Pawn::PostRender is routed to HUD for PlayerBeacon
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	// TODO: TEMP. workaround for JumpBoots fix (callign SetPlayerDefaults)
	TArray<APawn*> AlreadySpawnedPlayers;

#if !UE_SERVER
	virtual void BuildPlayerInfo(TSharedPtr<SVerticalBox> Panel, AUTPlayerState* PlayerState) override;
#endif

};