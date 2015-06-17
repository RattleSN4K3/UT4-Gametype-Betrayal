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
		if (PC == NULL || PC->Announcer == NULL)
			return;

		// TODO: Add support for dynamic music
		if (ClientData.MessageIndex == 1)
		{
			PC->Announcer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.OptionalObject);
		}
		else if (ClientData.MessageIndex == 0)
		{
			PC->Announcer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.OptionalObject);
			//PC->ClientMusicEvent(10);
		}
		else if ((ClientData.MessageIndex == 2) || (ClientData.MessageIndex == 3))
		{
			PC->Announcer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.OptionalObject);
			//PC->ClientMusicEvent(10);
		}
		else if (ClientData.MessageIndex == 5)
		{
			PC->Announcer->PlayAnnouncement(GetClass(), ClientData.MessageIndex, ClientData.OptionalObject);
			//PC->ClientMusicEvent(14);
		}
	}

	virtual void PrecacheAnnouncements_Implementation(class UUTAnnouncer* Announcer) const
	{
		if (Announcer == NULL) return;

		// hack/workaround.
		// Announcer is not allowing to use a direct USound, we need to  cache these first mapped to names. 
		// For convenience, using reflection to map names to sounds and add these to announcers CachedAudio mapping

		// FIXMESTEVE: Allow custom sounds (with custom path) being played by Announcer

		UObject* CDO = GetClass()->GetDefaultObject();
		for (TFieldIterator<UObjectProperty> Prop(GetClass()); Prop; ++Prop)
		{
			if (Prop->PropertyClass->IsChildOf<USoundBase>())
			{
				if (USoundBase* Sound = Cast<USoundBase>(Prop->GetObjectPropertyValue(Prop->ContainerPtrToValuePtr<UObject>(CDO))))
				{
					FName SoundName = ToAnnouncementName(Prop->GetName());
					if (!Announcer->RewardCachedAudio.Contains(SoundName))
					{
						Announcer->RewardCachedAudio.Add(SoundName, Sound);
					}

					// TODO: need to add Sound to StatusCachedAudio as well?
				}
			}
		}
	}

	// TODO: Remove once Announcer can play Sounds from custom path
	virtual FName ToAnnouncementName(FString PropName) const
	{
		return FName(*(GetClass()->GetName() + TEXT("_") + PropName));
	}

	virtual FName GetAnnouncementName_Implementation(int32 Switch, const UObject* OptionalObject) const override
	{
		// TODO: use sound instead of Names.
		// FIXMESTEVE: Allow custom sounds (with custom path) being played by Announcer

		if (Switch == 1)
			return ToAnnouncementName(TEXT("JoinTeamSound"));
		else if (Switch == 2)
			return ToAnnouncementName(TEXT("RetributionSound"));
		else if (Switch == 3)
			return ToAnnouncementName(TEXT("PaybackSound"));
		else if (Switch == 5)
			return ToAnnouncementName(TEXT("PaybackAvoidedSound"));

		return ToAnnouncementName(TEXT("BetrayalKillSound"));
	}
	
	virtual FLinearColor GetMessageColor(int32 MessageIndex) const override
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