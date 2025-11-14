// Copyright Epic Games, Inc. All Rights Reserved.

#include "ASerialComBPLibrary.h"
#include "Device/WiredDevice.h"
#include "ASerialCom.h"

UASerialComBPLibrary::UASerialComBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

UWiredDevice* UASerialComBPLibrary::CreateDevice(int DeviceID, int DeviceVersion, const FString& Name)
{
	UWiredDevice* Device = NewObject<UWiredDevice>();
	if (Device)
	{
		Device->InitCommunicator(DeviceID, DeviceVersion, Name);
		return Device;
	}
	return nullptr;
}

