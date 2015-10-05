#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalMessage.generated.h"

UCLASS(CustomConstructor, Abstract)
class UUTBetrayalMessage : public UUTLocalMessage
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Message)
	FText BetrayalKill;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Message)
	FText BetrayalJoinTeam;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Message)
	FText RetributionString;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Message)
	FText PaybackString;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Message)
	FText RogueTimerExpiredString;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* BetrayalKillSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* RetributionSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* PaybackSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* JoinTeamSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* PaybackAvoidedSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor DrawColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor BlueColor;

	UUTBetrayalMessage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	{
		bIsUnique = true;
		Lifetime = 6.0f;
		
		DrawColor = FLinearColor::Red;
		BlueColor = FLinearColor(FColor(0, 160, 255, 255));
		//FontSize = 3
		//bBeep = False

		MessageArea = FName(TEXT("GameMessages")); //MessageArea = 3

		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinder<USoundBase> BetrayalKillSound;
			ConstructorHelpers::FObjectFinder<USoundBase> RetributionSound;
			ConstructorHelpers::FObjectFinder<USoundBase> PaybackSound;
			ConstructorHelpers::FObjectFinder<USoundBase> JoinTeamSound;
			ConstructorHelpers::FObjectFinder<USoundBase> PaybackAvoidedSound;

			FConstructorStatics()
				: BetrayalKillSound(TEXT("SoundWave'/UTBetrayal/Sounds/Announcer/A_RewardAnnouncer_Assassin.A_RewardAnnouncer_Assassin'"))
				, RetributionSound(TEXT("SoundWave'/UTBetrayal/Sounds/Announcer/A_RewardAnnouncer_Retribution.A_RewardAnnouncer_Retribution'"))
				, PaybackSound(TEXT("SoundWave'/UTBetrayal/Sounds/Announcer/A_RewardAnnouncer_Payback.A_RewardAnnouncer_Payback'"))
				, JoinTeamSound(TEXT("SoundWave'/UTBetrayal/Sounds/A_StatusAnnouncer_YouAreOnBlue.A_StatusAnnouncer_YouAreOnBlue'"))
				, PaybackAvoidedSound(TEXT("SoundWave'/UTBetrayal/Sounds/Announcer/A_RewardAnnouncer_Excellent.A_RewardAnnouncer_Excellent'"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics; 
		
		BetrayalKillSound = ConstructorStatics.BetrayalKillSound.Object;
		RetributionSound = ConstructorStatics.RetributionSound.Object;
		PaybackSound = ConstructorStatics.PaybackSound.Object;
		JoinTeamSound = ConstructorStatics.JoinTeamSound.Object;
		PaybackAvoidedSound = ConstructorStatics.PaybackAvoidedSound.Object;

		BetrayalKill = NSLOCTEXT("UTBetrayalMessage", "BetrayalKill", "{Killer} BETRAYED {Killed}!");
		BetrayalJoinTeam = NSLOCTEXT("UTBetrayalMessage", "BetrayalJoinTeam", "JOINING NEW TEAM");
		RetributionString = NSLOCTEXT("UTBetrayalMessage", "RetributionString", "RETRIBUTION!");
		PaybackString = NSLOCTEXT("UTBetrayalMessage", "PaybackString", "PAYBACK!");
		RogueTimerExpiredString = NSLOCTEXT("UTBetrayalMessage", "RogueTimerExpiredString", "Payback Avoided");
	}

	virtual FText GetText(int32 Switch = 0, bool bTargetsPlayerState1 = false, class APlayerState* RelatedPlayerState_1 = NULL, class APlayerState* RelatedPlayerState_2 = NULL, class UObject* OptionalObject = NULL) const override
	{
		if (Switch == 1)
			return GetDefault<UUTBetrayalMessage>(GetClass())->BetrayalJoinTeam;
		else if (Switch == 2)
			return GetDefault<UUTBetrayalMessage>(GetClass())->RetributionString;
		else if (Switch == 3)
			return GetDefault<UUTBetrayalMessage>(GetClass())->PaybackString;
		else if ((Switch == 4) || (Switch == 0))
		{
			FFormatNamedArguments Args;
			Args.Add("Killer", FText::FromString(RelatedPlayerState_1->PlayerName));
			Args.Add("Killed", FText::FromString(RelatedPlayerState_2->PlayerName));
			return FText::Format(BetrayalKill, Args);
		}
		else
			return GetDefault<UUTBetrayalMessage>(GetClass())->RogueTimerExpiredString;
	}

	virtual void ClientReceive(const FClientReceiveData& ClientData) const override
	{
		Super::ClientReceive(ClientData);

		AUTPlayerController* PC = Cast<AUTPlayerController>(ClientData.LocalPC);
		if (PC == NULL || PC->Announcer == NULL)
			return;

		// TODO: Add support for dynamic music
		if (ClientData.MessageIndex == 1)
		{
			PC->Announcer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.RelatedPlayerState_1, ClientData.RelatedPlayerState_2, ClientData.OptionalObject);
		}
		else if (ClientData.MessageIndex == 0)
		{
			PC->Announcer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.RelatedPlayerState_1, ClientData.RelatedPlayerState_2, ClientData.OptionalObject);
			//PC->ClientMusicEvent(10);
		}
		else if ((ClientData.MessageIndex == 2) || (ClientData.MessageIndex == 3))
		{
			PC->Announcer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.RelatedPlayerState_1, ClientData.RelatedPlayerState_2, ClientData.OptionalObject);
			//PC->ClientMusicEvent(10);
		}
		else if (ClientData.MessageIndex == 5)
		{
			PC->Announcer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.RelatedPlayerState_1, ClientData.RelatedPlayerState_2, ClientData.OptionalObject);
			//PC->ClientMusicEvent(14);
		}
	}

	virtual FName GetAnnouncementName_Implementation(int32 Switch, const UObject* OptionalObject) const override
	{
		return NAME_Custom;
	}

	virtual USoundBase* GetAnnouncementSound_Implementation(int32 Switch, const UObject* OptionalObject) const override
	{
		auto default = GetDefault<UUTBetrayalMessage>(GetClass());

		if (Switch == 1)
			return default->JoinTeamSound;
		else if (Switch == 2)
			return default->RetributionSound;
		else if (Switch == 3)
			return default->PaybackSound;
		else if (Switch == 5)
			return default->PaybackAvoidedSound;

		return default->BetrayalKillSound;
	}
	
	virtual FLinearColor GetMessageColor_Implementation(int32 MessageIndex) const override
	{
		if ((MessageIndex == 1) || (MessageIndex == 5))
		{
			return GetDefault<UUTBetrayalMessage>(GetClass())->BlueColor;
		}

		return GetDefault<UUTBetrayalMessage>(GetClass())->DrawColor;
	}

	// TODO: Port GetFontSize
	virtual bool UseLargeFont(int32 MessageIndex) const override
	{
		return (MessageIndex != 4);
	}

	/** Range of 0 to 1, affects where announcement is inserted into pending announcements queue. */
	virtual float GetAnnouncementPriority(int32 Switch) const override
	{
		return 0.8f; //AnnouncementPriority = 8
	}

};