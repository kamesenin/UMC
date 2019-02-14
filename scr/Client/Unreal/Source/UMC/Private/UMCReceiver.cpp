/***
UMC
Created by Tomasz 'kamesenin' Witczak - kamesenin@gmail.com
**/
#include "UMCReceiver.h"
#include "Networking.h"

FUMCReceiver::FUMCReceiver()
	: ListenerSocket(nullptr)
	, UDPReceiver(nullptr)
	, SenderSocket(nullptr)
	, CurrentTime(0.0f)
	, MotionMultiply(1000.0f)
	, MaximumIdleTime(10.0f)
	, CurrentTouches(0)
{
	OpenConnection();
}

void FUMCReceiver::Update(float DeltaTime)
{
	CurrentTime += DeltaTime;

	if (SenderSocket == nullptr) { return; }

	int32 BytesSent(0);
	FString Message = "IamAlive";

	FArrayWriter DataWriter;
	DataWriter << Message;
	SenderSocket->SendTo(DataWriter.GetData(), DataWriter.Num(), BytesSent, *SenderRemoteAddress);
}

void FUMCReceiver::OpenConnection()
{
	SenderRemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool IsValid(false);
	SenderRemoteAddress->SetIp(TEXT("255.255.255.255"), IsValid);
	SenderRemoteAddress->SetPort(2492);

	if (!IsValid)
	{
		return;
	}

	const FString SenderSocketName = "SenderSocket" + FDateTime::Now().ToString();
	SenderSocket = FUdpSocketBuilder(SenderSocketName).WithSendBufferSize(2048);

	if (SenderSocket == nullptr)
	{
		return;
	}

	const FIPv4Endpoint Endpoint(FIPv4Address::Any, 2492);
	const FString ListenerSocketName = "ListenerSocket" + FDateTime::Now().ToString();
	ListenerSocket = FUdpSocketBuilder(ListenerSocketName).BoundToEndpoint(Endpoint).WithReceiveBufferSize(2048);

	if (ListenerSocket == nullptr)
	{
		CloseConnection();
		return;
	}

	const FTimespan ThreadWaitTime(FTimespan::FromMilliseconds(1000));
	const FString UDPReceiverName = "UDP RECEIVER" + FDateTime::Now().ToString();
	UDPReceiver = new FUdpSocketReceiver(ListenerSocket, ThreadWaitTime, *UDPReceiverName);

	if (UDPReceiver == nullptr)
	{
		CloseConnection();
		return;
	}

	UDPReceiver->OnDataReceived().BindRaw(this, &FUMCReceiver::ReceiveData);
	UDPReceiver->Start();
}

void FUMCReceiver::ReceiveData(const FArrayReaderPtr& ArrayReader, const FIPv4Endpoint& EndPoint)
{
	int32 BytesRead(0);

	if (ArrayReader->Num() == 0 || ArrayReader->GetData() == nullptr)
	{
		return;
	}

	if (ListenerSocket == nullptr)
	{
		return;
	}
	ListenerSocket->Recv(ArrayReader->GetData(), ArrayReader->Num(), BytesRead);
	{
		if (ArrayReader->GetData() == nullptr)
		{
			return;
		}

		FString ServerMessage(UTF8_TO_TCHAR((char*)ArrayReader->GetData()));

		TArray<FString> Splited;
		ServerMessage.ParseIntoArray(Splited, TEXT("#"), false);

		if (Splited.Num() < 7) { return; }

		if (Splited[0] != "PC")
		{
			int ID = FCString::Atoi(*Splited[2]);
			if (Splited[1] == "ID")
			{
				if (AliveControllers.Contains(ID) == false)
				{
					AliveControllers.Add(ID, CurrentTime);
					ControllersLocation.Add(ID, FVector::ZeroVector);
					ControllersRotation.Add(ID, FQuat::Identity);
				}
				else
				{
					AliveControllers[ID] = CurrentTime;
				}
			}

			if (Splited[3] == "Location")
			{
				TArray<FString> SplitedLocation;
				Splited[4].ParseIntoArray(SplitedLocation, TEXT(","), true);

				FVector LocationFromPhone(FCString::Atof(*SplitedLocation[0]) * MotionMultiply, FCString::Atof(*SplitedLocation[2]) * -MotionMultiply, FCString::Atof(*SplitedLocation[1]) * MotionMultiply);
				ControllersLocation[ID] = LocationFromPhone;
			}

			if (Splited[5] == "Rotation")
			{
				TArray<FString> SplitedRotation;
				Splited[6].ParseIntoArray(SplitedRotation, TEXT(","), true);

				FQuat RotationFromPhone(FCString::Atof(*SplitedRotation[0]), FCString::Atof(*SplitedRotation[1]), FCString::Atof(*SplitedRotation[2]), FCString::Atof(*SplitedRotation[3]));
				ControllersRotation[ID] = RotationFromPhone;
			}

			if (Splited.Num() > 8 && Splited[7] == "Touches")
			{
				CurrentTouches = FCString::Atoi(*Splited[8]);
			}
		}
	}
}

void FUMCReceiver::CloseConnection(const bool CalledOnDestroy)
{
	if (SenderSocket)
	{
		SenderSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(SenderSocket);
		SenderSocket = nullptr;
	}

	if (ListenerSocket)
	{
		delete UDPReceiver;
		UDPReceiver = nullptr;

		ListenerSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenerSocket);
		ListenerSocket = nullptr;
	}
	AliveControllers.Empty();
	ControllersLocation.Empty();
	ControllersRotation.Empty();
}

void FUMCReceiver::CheckConnectedControllers()
{
	bool ShouldRemoveID = false;
	int ToRemoveIndex = 0;
	int CurrentIndex = 0;

	TArray<int32> IDs;
	AliveControllers.GenerateKeyArray(IDs);

	for (int32 ID : IDs)
	{
		float LastTimeUpdated = CurrentTime - AliveControllers[ID];
		if (LastTimeUpdated > MaximumIdleTime)
		{
			ShouldRemoveID = true;
			break;
		}
		++ToRemoveIndex;
	}

	if (ShouldRemoveID == false) { return; }

	TArray<float> AliveTimes;
	AliveControllers.GenerateValueArray(AliveTimes);

	for (int32 ID : IDs)
	{
		if (CurrentIndex == ToRemoveIndex)
		{
			AliveControllers.Remove(ID);
			ControllersLocation.Remove(ID);
			ControllersRotation.Remove(ID);
		}

		if (CurrentIndex > ToRemoveIndex)
		{
			int NewKey = ID - 1;

			float AliveValue = AliveTimes[ID];
			AliveControllers.Remove(ID);
			AliveControllers.Add(NewKey, AliveValue);

			FVector Location(ControllersLocation[ID]);
			ControllersLocation.Remove(ID);
			ControllersLocation.Add(NewKey, Location);

			FQuat Rotation(ControllersRotation[ID]);
			ControllersRotation.Remove(ID);
			ControllersRotation.Add(NewKey, Rotation);
		}
	}
}

FUMCReceiver::~FUMCReceiver()
{
	CloseConnection(true);
}