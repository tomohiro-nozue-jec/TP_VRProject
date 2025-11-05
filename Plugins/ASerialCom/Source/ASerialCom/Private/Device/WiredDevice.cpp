// Fill out your copyright notice in the Description page of Project Settings.

#include "Device/WiredDevice.h"
#include "Device/DeviceCommunicator.h"

UWiredDevice::UWiredDevice()
	: Communicator(nullptr)
{
}

UWiredDevice::~UWiredDevice()
{
	if (Communicator)
	{
		delete Communicator;
		Communicator = nullptr;
	}
}

void UWiredDevice::Tick(float DeltaTime)
{
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

	Communicator->Connect();
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

	Communicator->ReceiveCmd(Cmd);
}

void UWiredDevice::HandleData(ASerialDataStruct::ASerialData Data)
{
	int ActualData = ConvertData<int>(Data.data, Data.data_num);
	NotifyRecieveData(ActualData);
}

void UWiredDevice::NotifyRecieveData(int Data)
{
	if (OnRecieveDataEvent.IsBound())
	{
		OnRecieveDataEvent.Broadcast(Data);
	}
}
