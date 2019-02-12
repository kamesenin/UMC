#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "Runtime/Networking/Public/Common/UdpSocketReceiver.h"
#include "UMCActor.generated.h"

UCLASS()
class UMC_API AUMCActor : public AActor
{
	GENERATED_BODY()

public:
	
	AUMCActor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> ActorsToControl;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	class FUMCReceiver* Receiver;
	TMap<int32, FVector> Offsets;
public:
	virtual void Tick(float DeltaTime) override;
};
