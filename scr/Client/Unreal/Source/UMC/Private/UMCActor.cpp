/***
UMC
Created by Tomasz 'kamesenin' Witczak - kamesenin@gmail.com
**/
#include "UMCActor.h"
#include "UMCReceiver.h"

AUMCActor::AUMCActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Receiver(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AUMCActor::BeginPlay()
{
	Super::BeginPlay();

	if (Receiver == nullptr) { Receiver = new FUMCReceiver(); }
}

void AUMCActor::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	delete Receiver;
	Receiver = nullptr;
}

void AUMCActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Receiver != nullptr) { Receiver->Update(DeltaTime); }

	TArray<int32> IDs;

	Receiver->GetAliveControllers().GenerateKeyArray(IDs);

	int32 Index = 0;

	for (int32 ID : IDs)
	{
		FTransform ControllerTransform = Receiver->GetControllersTransforms()[ID];
		
		const FVector LocationTo = Offsets.Contains(ID) ? ControllerTransform.GetLocation() - Offsets[ID] : ControllerTransform.GetLocation();
		const FVector NewLocation(FMath::VInterpTo(ActorsToControl[Index]->GetActorLocation(), LocationTo, DeltaTime, 10.0f));
		ActorsToControl[Index]->SetActorLocation(NewLocation);
		const FQuat NewRotation(FMath::QInterpTo(ActorsToControl[Index]->GetActorRotation().Quaternion(), ControllerTransform.GetRotation(), DeltaTime, 10.0f));
		ActorsToControl[Index]->SetActorRotation(NewRotation);
		++Index;

		if (Receiver->GetTouches() >= 5)
		{
			if (Offsets.Contains(ID))
			{
				Offsets[ID] = ControllerTransform.GetLocation();
			}
			else
			{
				Offsets.Add(ID, ControllerTransform.GetLocation());
			}
		}
	}
}

