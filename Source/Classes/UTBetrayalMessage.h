#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalMessage.generated.h"

UCLASS(CustomConstructor)
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
		Importance = 0.8f; //AnnouncementPriority = 8

		static ConstructorHelpers::FObjectFinder<USoundBase> BetrayalKillSoundFinder(TEXT("SoundWave'/UTBetrayal/Sounds/Announcer/A_RewardAnnouncer_Assassin.A_RewardAnnouncer_Assassin'"));
		BetrayalKillSound = BetrayalKillSoundFinder.Object;
		static ConstructorHelpers::FObjectFinder<USoundBase> RetributionSoundFinder(TEXT("SoundWave'/UTBetrayal/Sounds/Announcer/A_RewardAnnouncer_Retribution.A_RewardAnnouncer_Retribution'"));
		RetributionSound = RetributionSoundFinder.Object;
		static ConstructorHelpers::FObjectFinder<USoundBase> PaybackSoundFinder(TEXT("SoundWave'/UTBetrayal/Sounds/Announcer/A_RewardAnnouncer_Payback.A_RewardAnnouncer_Payback'"));
		PaybackSound = PaybackSoundFinder.Object;
		static ConstructorHelpers::FObjectFinder<USoundBase> JoinTeamSoundFinder(TEXT("SoundWave'/UTBetrayal/Sounds/A_StatusAnnouncer_YouAreOnBlue.A_StatusAnnouncer_YouAreOnBlue'"));
		JoinTeamSound = JoinTeamSoundFinder.Object;
		static ConstructorHelpers::FObjectFinder<USoundBase> PaybackAvoidedSoundFinder(TEXT("SoundWave'/UTBetrayal/Sounds/Announcer/A_RewardAnnouncer_Excellent.A_RewardAnnouncer_Excellent'"));
		PaybackAvoidedSound = PaybackAvoidedSoundFinder.Object;

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
		if (PC == NULL || PC->RewardAnnouncer == NULL)
			return;

		// TODO: Add support for dynamic music
		if (ClientData.MessageIndex == 1)
		{
			PC->RewardAnnouncer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.OptionalObject);
		}
		else if (ClientData.MessageIndex == 0)
		{
			PC->RewardAnnouncer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.OptionalObject);
			//PC->ClientMusicEvent(10);
		}
		else if ((ClientData.MessageIndex == 2) || (ClientData.MessageIndex == 3))
		{
			PC->RewardAnnouncer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.OptionalObject);
			//PC->ClientMusicEvent(10);
		}
		else if (ClientData.MessageIndex == 5)
		{
			PC->RewardAnnouncer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.OptionalObject);
			//PC->ClientMusicEvent(14);
		}
	}

	//AnnouncementSound
	
	virtual FLinearColor GetMessageColor(int32 MessageIndex) const override
	{
		if ((MessageIndex == 1) || (MessageIndex == 5))
		{
			return GetDefault<UUTBetrayalMessage>(GetClass())->BlueColor;
		}

		return GetDefault<UUTBetrayalMessage>(GetClass())->DrawColor;
	}

	//GetFontSize
};