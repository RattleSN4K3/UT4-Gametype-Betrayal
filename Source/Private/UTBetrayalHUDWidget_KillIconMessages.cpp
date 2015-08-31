#include "UTBetrayal.h"
#include "UTBetrayalHUDWidget_KillIconMessages.h"
//#include "BlueprintGeneratedClass.h"

UUTBetrayalHUDWidget_KillIconMessages::UUTBetrayalHUDWidget_KillIconMessages(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UClass> HudObject;

		FConstructorStatics()
			: HudObject(TEXT("Class'/Game/RestrictedAssets/UI/HUDWidgets/bpWH_KillIconMessages.bpWH_KillIconMessages_C'"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	UClass* Cls = ConstructorStatics.HudObject.Object;
	if (ConstructorStatics.HudObject.Object != NULL && Cls != NULL)
	{
		UObject* CDO = Cls->GetDefaultObject(false);
		if (CDO != NULL)
		{
			UUTBetrayal::InitProperties(this, Cls, CDO, false);
		}
	}

	ScreenPosition = FVector2D(0.0f, 0.035f);
}

void UUTBetrayalHUDWidget_KillIconMessages::DrawMessages(float DeltaTime)
{
	Canvas->Reset();

	// top location;
	float Y = Canvas->ClipY * ScreenPosition.Y;

	//Draw in reverse order from top to bottom
	int32 MessageIndex = FMath::Min(CurrentIndex, MessageQueue.Num() - 1);
	while (MessageIndex >= 0)
	{
		if (MessageQueue[MessageIndex].MessageClass != nullptr)
		{
			Y += MessageHeight + MessagePadding;
			DrawMessage(MessageIndex, 0, Y);
		}
		MessageIndex--;
	}
}
