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

#if !UE_SERVER
	virtual void BuildPlayerInfo(TSharedPtr<SVerticalBox> Panel, AUTPlayerState* PlayerState) override;

	static FText RoundPerc(uint32 dividend, uint32 divisor)
	{
		float val = divisor > 0.0 ? (float)dividend / (float)divisor : 0.0;
		FNumberFormattingOptions NumberOpts;
		NumberOpts.MinimumFractionalDigits = 2;
		NumberOpts.MaximumFractionalDigits = 2;
		return FText::AsNumber(val, &NumberOpts);
	}

	/** called on the default object of this class by the UI to create widgets to manipulate this game type's settings
	* you can use TAttributeProperty<> to easily implement get/set delegates that map directly to the config property address
	* add any such to the ConfigProps array so the menu maintains the shared pointer
	*/
	virtual void CreateConfigWidgets(TSharedPtr<class SVerticalBox> MenuSpace, bool bCreateReadOnly, TArray< TSharedPtr<TAttributePropertyBase> >& ConfigProps) override;
	//virtual void CreateConfigWidgets(bool bCreateReadOnly, TArray< FGameOptionWidgetInfo >& GameProps) override;

#endif

	// Allows gametypes to build their rules settings for the mid game menu.
	virtual FText BuildServerRules(AUTGameState* GameState) override;

	/**
	*	Builds a \t separated list of rules that will be sent out to clients when they request server info via the UTServerBeaconClient.
	**/
	virtual void BuildServerResponseRules(FString& OutRules) override;
	
	virtual void GetGameURLOptions(TArray<FString>& OptionsList, int32& DesiredPlayerCount) override;

};