/***
UMC
Created by Tomasz 'kamesenin' Witczak - kamesenin@gmail.com
**/
#pragma once
#include "CoreMinimal.h"
#include "Runtime/Networking/Public/Common/UdpSocketBuilder.h"
#include "Runtime/Networking/Public/Common/UdpSocketReceiver.h"

class UMC_API FUMCReceiver
{
public:
	FUMCReceiver();
	~FUMCReceiver();

	void Update(float DeltaTime);

	TMap<int32, float> GetAliveControllers() { return AliveControllers; }
	TMap<int32, FTransform> GetControllersTransforms() { return ControllersTransform; }

	int32 GetTouches() { return CurrentTouches; }

private:
	void OpenConnection();
	void CloseConnection(const bool CalledOnDestroy = false);

	void ReceiveData(const FArrayReaderPtr& ArrayReader, const FIPv4Endpoint& EndPoint);

	void CheckConnectedControllers();

	FSocket* ListenerSocket;
	FUdpSocketReceiver* UDPReceiver;

	TSharedPtr<FInternetAddr> SenderRemoteAddress;
	FSocket* SenderSocket;

	TMap<int32, float> AliveControllers;
	TMap<int32, FTransform> ControllersTransform;

	float CurrentTime;
	float MotionMultiply;
	float MaximumIdleTime;
	int32 CurrentTouches;

	//ARCore
	const FMatrix ARCoreToUnrealTransform = FMatrix(
		FPlane(0.0f, 0.0f, -1.0f, 0.0f),
		FPlane(1.0f, 0.0f, 0.0f, 0.0f),
		FPlane(0.0f, 1.0f, 0.0f, 0.0f),
		FPlane(0.0f, 0.0f, 0.0f, 1.0f));

	const FMatrix ARCoreToUnrealTransformInverse = ARCoreToUnrealTransform.InverseFast();
};
