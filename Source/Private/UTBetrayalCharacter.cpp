#include "UTBetrayal.h"
#include "UTBetrayalCharacter.h"
#include "UTBetrayalHUD.h"
#include "UTPlayerCameraManager.h"

// Betrayal Character is the main character class for Betrayal which is handling all
// the logic. The 2 Blueprint character classes BaseUTCharacter_Betrayal and DefaultCharacter_Betrayal
// are plain copies of the non-Betrayal classes where BaseUTCharacter_Betrayal has this class parent
// class.

// In order to create clean copies, first duplicate BaseUTCharacter and reparent to UTBetrayalCharacter. Then duplicate
// DefaultCharacter_Betrayal and reparent to BaseUTCharacter_Betrayal.

AUTBetrayalCharacter::AUTBetrayalCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//static ConstructorHelpers::FObjectFinder<UClass> PlayerPawnObject(TEXT("Class'/UTBetrayal/DefaultCharacter_Betrayal.DefaultCharacter_Betrayal_C'"));
	//DefaultPawnClass = PlayerPawnObject.Object;

	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UClass> PawnClass;

		FConstructorStatics()
			: PawnClass(TEXT("Class'/Game/RestrictedAssets/Blueprints/DefaultCharacter.DefaultCharacter_C'"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	UClass* Cls = ConstructorStatics.PawnClass.Object;
	if (Cls != NULL)
	{
		UObject* CDO = Cls->GetDefaultObject(false);
		if (CDO != NULL)
		{
			//UUTBetrayal::InitProperties(this, Cls, CDO, true);

			/*TArray<UObject*> SubObjects;
			Cls->GetDefaultObjectSubobjects(SubObjects);
			for (int32 Index = 0; Index < SubObjects.Num(); ++Index)
			{
				UObject* MaybeTemplate = Outer->GetArchetype()->GetClass()->GetDefaultSubobjectByName(SubobjectFName);
				if (MaybeTemplate && MaybeTemplate->IsA(ReturnType) && Template != MaybeTemplate)
				{
					ComponentInits.Add(Result, MaybeTemplate);
				}

				this->InstanceSubobjectTemplates
					InitProperties(Subobject, Template->GetClass(), Template, false);
			}*/


			//TMap<FName, int32> OldInstanceMap;

			//// Find all instanced objects of the old CDO, and save off their modified properties to be later applied to the newly instanced objects of the new CDO
			//TArray<UObject*> Components;
			//CollectDefaultSubobjects(Components, true);

			//for (int32 Index = 0; Index < Components.Num(); Index++)
			//{
			//	UObject* OldInstance = Components[Index];
			//	OldInstanceMap.Add(OldInstance->GetFName(), Index);
			//}


			//TArray<UObject*> ComponentsOnNewObject;

			//// Find all instanced objects of the old CDO, and save off their modified properties to be later applied to the newly instanced objects of the new CDO
			//CDO->CollectDefaultSubobjects(ComponentsOnNewObject, true);

			//for (int32 Index = 0; Index < ComponentsOnNewObject.Num(); Index++)
			//{
			//	UObject* NewInstance = ComponentsOnNewObject[Index];
			//	if (int32* pOldInstanceIndex = OldInstanceMap.Find(NewInstance->GetFName()))
			//	{
			//		bool bContainedInsideNewInstance = false;
			//		for (UObject* Parent = NewInstance->GetOuter(); Parent != NULL; Parent = Parent->GetOuter())
			//		{
			//			if (Parent == CDO)
			//			{
			//				bContainedInsideNewInstance = true;
			//				break;
			//			}
			//		}

			//		if (!bContainedInsideNewInstance)
			//		{
			//			// A bad thing has happened and cannot be reasonably fixed at this point
			//			UE_LOG(Betrayal, Log, TEXT("Warning: The CDO '%s' references a component that does not have the CDO in its outer chain!"), *Cls->GetFullName(), *NewInstance->GetFullName());
			//		}
			//	}

			//	ObjectInitializer.CreateDefaultSubobject(this, TEXT("Movement"));
			//}
		}
	}
}

void AUTBetrayalCharacter::PostRenderFor(APlayerController* PC, UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	// safety check
	if (PC == NULL || PC->GetPawn() == this)
		return;

	// don't render irrelevant characters
	if (Health == 0 || IsDead() || IsFeigningDeath() || !IsPawnVisible(PC, CameraPosition))
		return;

	FVector WorldPosition = GetMesh()->GetComponentLocation();
	FVector ScreenPosition = Canvas->Project(WorldPosition + FVector(0.f, 0.f, GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() * 2.25f));
	
	// make sure not clipped out
	if (FVector::DotProduct(CameraDir, (GetActorLocation() - CameraPosition)) <= 0.0f)
	{
		return;
	}

	AUTBetrayalHUD* Hud = Cast<AUTBetrayalHUD>(PC->MyHUD);
	if (Hud != NULL)
	{
		float Dist = (CameraPosition - GetActorLocation()).Size();
		float MaxDist = 2.0 * TeamPlayerIndicatorMaxDistance;
		if (Dist <= MaxDist)
		{
			Hud->DrawPlayerBeacon(this, Canvas, CameraPosition, CameraDir, ScreenPosition);
		}
	}
}

bool AUTBetrayalCharacter::IsPawnVisible(APlayerController* PC, FVector CameraPosition)
{
	if (AUTPlayerCameraManager* CamMgr = Cast<AUTPlayerCameraManager>(PC->PlayerCameraManager))
	{
		FHitResult Result(1.f);
		CamMgr->CheckCameraSweep(Result, this, CameraPosition, GetActorLocation() + FVector(0.f, 0.f, GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
		if (Result.bBlockingHit)
		{
			return false;
		}
	}

	return true;
}