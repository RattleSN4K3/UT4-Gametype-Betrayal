#include "UTBetrayal.h"
#include "UTBetrayalHUDWidget_Leaderboard.h"
//#include "BlueprintGeneratedClass.h"

UUTBetrayalHUDWidget_Leaderboard::UUTBetrayalHUDWidget_Leaderboard(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UClass> HudObject(TEXT("Class'/Game/RestrictedAssets/UI/HUDWidgets/bpHW_DMLeaderboard.bpHW_DMLeaderboard_C'"));

	UClass* Cls = HudObject.Object;
	if (HudObject.Object != NULL && Cls != NULL)
	{
		UObject* CDO = Cls->GetDefaultObject(false);
		if (CDO != NULL)
		{
			UUTBetrayal::InitProperties(this, Cls, CDO, false);
		}
	}

	ScreenPosition = FVector2D(1.0f, 0.07f);
}
