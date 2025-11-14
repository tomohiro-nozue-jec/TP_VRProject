// Fill out your copyright notice in the Description page of Project Settings.

#include "Device/WiredDevice.h"
#include "Device/DeviceCommunicator.h"

UWiredDevice::UWiredDevice()
	: Communicator(nullptr)
{
}

UWiredDevice::~UWiredDevice()
{
	UE_LOG(LogTemp, Warning, TEXT("WiredDevice Destructed"));
	if (Communicator)
	{
		delete Communicator;
		Communicator = nullptr;
	}
}

void UWiredDevice::Tick(float DeltaTime)
{
	// キューにデータがあれば処理する
	if (!DataQueue.IsEmpty())
	{
		ASerialDataStruct::ASerialData RPMData;
		DataQueue.Dequeue(RPMData);
		HandleData(RPMData);
	}
}

TStatId UWiredDevice::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWiredDevice, STATGROUP_Tickables);
}

bool UWiredDevice::IsTickableInEditor() const
{
	return false;
}

void UWiredDevice::InitCommunicator(int DeviceID, int DeviceVersion, const FString& DeviceName)
{
	// コンストラクタを呼ぶとき、自動にコネクトする
	Communicator = new DeviceCommunicator(DeviceID, DeviceVersion, *DeviceName, &DataQueue);
	Name = DeviceName;
}

void UWiredDevice::PauseThread()
{
	if (!Communicator)
		return;

	Communicator->Pause();
}

void UWiredDevice::ReStartThread()
{
	if (!Communicator)
		return;

	Communicator->ReStart();
}

void UWiredDevice::ChangeFrequency(float NewFrequency)
{
	if (!Communicator)
		return;

	Communicator->ChangeFrequency(NewFrequency);
}

FString UWiredDevice::GetName()
{
	return Name;
}

void UWiredDevice::Connect()
{
	if (!Communicator)
		return;

	if (Communicator->Connect() == ExecutionResult::Succ)
	{
		UE_LOG(LogTemp, Display, TEXT("Connect Success"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Connect Fail"));
}

void UWiredDevice::Disconnect()
{
	if (!Communicator)
		return;

	Communicator->Disconnect();
}

bool UWiredDevice::IsConnecting() const
{
	if (!Communicator)
		return false;

	return Communicator->IsConnecting();
}

void UWiredDevice::RequestData(uint8 Cmd)
{
	if (!Communicator)
		return;

	if(Communicator->IsConnecting())
		Communicator->ReceiveCmd(Cmd);
}

void UWiredDevice::HandleData(ASerialDataStruct::ASerialData Data)
{
	int ActualData = ConvertData<uint16>(Data.data, Data.data_num);
	NotifyRecieveData(ActualData);
}

void UWiredDevice::NotifyRecieveData(int Data)
{
	if (OnRecieveDataEvent.IsBound())
	{
		OnRecieveDataEvent.Broadcast(Data);
	}
}

template<typename T>
inline T UWiredDevice::ConvertData(const uint8_t* Data, int Size)
{
	// 貰ったデータは二進数の配列なので、必要な型に変換する
	// Dataはデータ本体、Sizeはバイト数(電子科から貰った資料を参考)
	//Data[0]が上位バイト, Data[1]が下位バイト
	T Result = 0;
	for (int i = 0; i < Size; ++i)
	{
		Result |= (Data[i] << (8 * (Size - 1 - i)));
	}
	return Result;
}