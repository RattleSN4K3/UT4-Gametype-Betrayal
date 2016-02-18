#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalTeam.h"
#include "UTBetrayalPlayerState.h"

#include "Private/Slate/Dialogs/SUTPlayerInfoDialog.h"

#include "UTBetrayalGameMode.generated.h"

#if !UE_SERVER

struct TAttributeStatBetrayal : public TAttributeStat
{
	typedef float(*StatValueFuncBet)(const AUTBetrayalPlayerState*);
	typedef FText(*StatValueTextFuncBet)(const AUTBetrayalPlayerState*, const float Value);

	TAttributeStatBetrayal(AUTBetrayalPlayerState* InPlayerState, StatValueFuncBet InValueFunc = nullptr, StatValueTextFuncBet InTextFunc = nullptr)
		: TAttributeStat(InPlayerState, NAME_None, nullptr, nullptr), BetrayalPlayerState(InPlayerState), ValueFuncBet(InValueFunc), TextFuncBet(InTextFunc)
	{
		checkSlow(PlayerState.IsValid());
	}

	virtual float GetValue() const override
	{
		if (BetrayalPlayerState.IsValid() && ValueFuncBet != nullptr)
		{
			return ValueFuncBet(BetrayalPlayerState.Get());
		}
		return 0.0f;
	}
	virtual FText GetValueText() const override
	{
		if (BetrayalPlayerState.IsValid())
		{
			return (TextFuncBet != nullptr) ? TextFuncBet(BetrayalPlayerState.Get(), GetValue()) : FText::FromString(FString::FromInt((int32)GetValue()));
		}
		return FText();
	}

	TWeakObjectPtr<AUTBetrayalPlayerState> BetrayalPlayerState;
	StatValueFuncBet ValueFuncBet;
	StatValueTextFuncBet TextFuncBet;
};

#endif

UCLASS()
class AUTBetrayalGameMode : public AUTDMGameMode
{
	GENERATED_UCLASS_BODY()

protected:

	FTimerHandle TimerHandle_MaybeStartTeam;

	bool bForcePlayerIntro;

public:

	/** mutators which are disallowed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	TArray< TSubclassOf<AUTMutator> > DisallowedMutators;

	/** Inventory items which are disallowed when AllowPickups is set*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	TArray< TSubclassOf<AUTInventory> > DisallowedInventories;

	/** Pickup factories which are disallowed when AllowPickups is set*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	TArray< TSubclassOf<AUTPickupInventory> > DisallowedPickupFactories;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<AUTBetrayalTeam*> Teams;

	// TODO: Use UTWeap_InstagibRifle as field type
	UPROPERTY(EditDefaultsOnly, Category = Game)
	TSubclassOf<AUTWeapon> InstagibRifleClass;

	UPROPERTY(EditDefaultsOnly, Category = Game)
	TSubclassOf<AUTBetrayalTeam> TeamClass;

	/** Whether to allow non-timed pickup */
	UPROPERTY(EditDefaultsOnly, Category = Betrayal)
	bool bAllowPickups;

	/** Score bonus for killing Rogue that betrayed you */
	UPROPERTY(EditDefaultsOnly, Category = Betrayal)
	int32 RogueValue;

	/** Rogue time penalty **/
	UPROPERTY(EditDefaultsOnly, Category = Betrayal)
	int32 RogueTimePenalty;

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
	virtual void StartMatch() override;

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
	virtual void ConditionallyStartTeamTimer();
	virtual void MaybeStartTeam();

	virtual void Logout(AController* Exiting) override;
	virtual void ScoreKill_Implementation(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType) override;

	virtual int32 GetTotalPlayingPlayers()
	{
		return NumBots + NumPlayers;
	}

#if !UE_SERVER
	virtual void BuildPlayerInfo(AUTPlayerState* PlayerState, TSharedPtr<class SUTTabWidget> TabWidget, TArray<TSharedPtr<struct TAttributeStat> >& StatList) override;
	virtual void BuildBetrayalInfo(AUTPlayerState* PlayerState, TSharedPtr<class SUTTabWidget> TabWidget, TArray<TSharedPtr<struct TAttributeStat> >& StatList);
	// TEMP: workaround
	// TODO: remove once player mesh preview is smaller
	virtual void NewInfoHeader(TSharedPtr<SVerticalBox> VBox, FText DisplayName);
	virtual void BuildScoreInfo(AUTPlayerState* PlayerState, TSharedPtr<class SUTTabWidget> TabWidget, TArray<TSharedPtr<struct TAttributeStat> >& StatList) override;
	virtual void AddBetrayalInfo(AUTPlayerState* PlayerState, TSharedPtr<class SUTTabWidget> TabWidget, TArray<TSharedPtr<struct TAttributeStat> >& StatList, TSharedPtr<SHorizontalBox> HBox, TSharedPtr<SVerticalBox> LeftPane, TSharedPtr<SVerticalBox> RightPane);

	/** called on the default object of this class by the UI to create widgets to manipulate this game type's settings
	* you can use TAttributeProperty<> to easily implement get/set delegates that map directly to the config property address
	* add any such to the ConfigProps array so the menu maintains the shared pointer
	*/
	virtual void CreateConfigWidgets(TSharedPtr<class SVerticalBox> MenuSpace, bool bCreateReadOnly, TArray< TSharedPtr<TAttributePropertyBase> >& ConfigProps) override;

#endif

	// Allows gametypes to build their rules settings for the mid game menu.
	virtual FText BuildServerRules(AUTGameState* GameState) override;

	/**
	*	Builds a \t separated list of rules that will be sent out to clients when they request server info via the UTServerBeaconClient.
	**/
	virtual void BuildServerResponseRules(FString& OutRules) override;
	
	// used to modify list of game options in custom games menu
	virtual void CreateGameURLOptions(TArray<TSharedPtr<TAttributePropertyBase>>& MenuProps) override;

	virtual void GetGameURLOptions(const TArray<TSharedPtr<TAttributePropertyBase>>& MenuProps, TArray<FString>& OptionsList, int32& DesiredPlayerCount) override;

	// TODO: Check how to remove exec for shipped build
	UFUNCTION(Exec)
	virtual void BETKillbot(const FString& NameOrUIDStr);
	UFUNCTION(Server, Reliable, WithValidation)
	virtual void BETServerKillbot(const FString& NameOrUIDStr);

};


// TODO: Remove once TAttributePropertyBool is properly exported
struct TAttributePropertyBool_TEMP : public TAttributePropertyBool
{
	TAttributePropertyBool_TEMP(UObject* InObj, bool* InData, const TCHAR* InURLKey = NULL)
	: TAttributePropertyBool(InObj, InData, InURLKey)
	{}
	ECheckBoxState GetAsCheckBox() const
	{
		return Get() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	void SetFromCheckBox(ECheckBoxState CheckedState)
	{
		if (CheckedState == ECheckBoxState::Checked)
		{
			Set(true);
		}
		else if (CheckedState == ECheckBoxState::Unchecked)
		{
			Set(false);
		}
	}
};