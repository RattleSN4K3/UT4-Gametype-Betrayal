#include "UTBetrayal.h"
#include "UTBetrayalHUDWidget_Leaderboard.h"
//#include "BlueprintGeneratedClass.h"

UUTBetrayalHUDWidget_Leaderboard::UUTBetrayalHUDWidget_Leaderboard(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UClass> HudObject;

		FConstructorStatics()
			: HudObject(TEXT("Class'/Game/RestrictedAssets/UI/HUDWidgets/bpHW_DMLeaderboard.bpHW_DMLeaderboard_C'"))
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

	ScreenPosition = FVector2D(1.0f, 0.07f);
}
