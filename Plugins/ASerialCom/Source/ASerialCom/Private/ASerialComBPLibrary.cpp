// Copyright Epic Games, Inc. All Rights Reserved.

#include "ASerialComBPLibrary.h"
#include "Device/WiredDevice.h"
#include "ASerialCom.h"

UASerialComBPLibrary::UASerialComBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

UWiredDevice* UASerialComBPLibrary::CreateDevice(int ID, int Version, const FString& Name)
{
	return NewObject<UWiredDevice>();
}

