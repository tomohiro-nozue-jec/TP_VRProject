// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/KATMoverComponent.h"
#include "KATSDKWarpper.h"
#include "Camera/CameraComponent.h"

// Sets default values for this component's properties
UKATMoverComponent::UKATMoverComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UKATMoverComponent::BeginPlay()
{
	Super::BeginPlay();
	
	KATDataHandler = new KATSDKWarpper();
	VRCamera = Cast<UCameraComponent>(GetOwner()->FindComponentByClass(UCameraComponent::StaticClass()));

	// デバイス接続確認
	if (KATDataHandler != nullptr)
	{
		int32 DeviceCount = KATDataHandler->DeviceCount();
		if (DeviceCount > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("KATVR Devices Found: %d"), DeviceCount));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No KATVR Devices Found"));
		}
	}
}

// Called every frame
void UKATMoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	HandleKATVRInput(DeltaTime);
}

void UKATMoverComponent::StartMove()
{
	CanMove = true;
}

void UKATMoverComponent::StopMove()
{
	CanMove = false;
}

void UKATMoverComponent::HandleKATVRInput(float DeltaTime)
{
	if (KATDataHandler == nullptr) return;
	
	HandleKATVRInputWalk(DeltaTime);
	HandleKATVRRotator();
}

void UKATMoverComponent::HandleKATVRInputWalk(float DeltaTime)
{
	if (!CanMove) return;
	if (VRCamera == nullptr) return;
	if (KATDataHandler == nullptr) return;

	Vector3 MoveSpeed = KATDataHandler->GetWalkStatus(nullptr).treadMillData.moveSpeed;

	float MoveAmount = MoveSpeed.z;
	auto cameraForwardVector = FVector(VRCamera->GetForwardVector().X, VRCamera->GetForwardVector().Y, 0).GetSafeNormal();

	GetOwner()->AddActorWorldOffset(cameraForwardVector * MoveAmount * NowSpeed, true);
}

void UKATMoverComponent::HandleKATVRRotator()
{
	if (KATDataHandler == nullptr) return;

	FQuat newQuat = Quaternion::unityToUnreal(KATDataHandler->GetWalkStatus(nullptr).treadMillData.bodyRotationRaw);
	RotateCharacterByFQuat(newQuat, 0.5f);
}

void UKATMoverComponent::RotateCharacterByFQuat(FQuat targetQuat, float duration)
{
	CurrentRotator = targetQuat.Rotator();
	FRotator DeltaRotation = CurrentRotator - PreRotator;

	DeltaRotation.Yaw = -DeltaRotation.Yaw;

	FRotator CharacterTargetRotator = GetOwner()->GetActorRotation() + DeltaRotation;
	FRotator VROffsetRotator = GetRelativeRotation() - DeltaRotation;

	GetOwner()->SetActorRotation(CharacterTargetRotator);
	SetRelativeRotation(VROffsetRotator);

	PreRotator = CurrentRotator;
}

void UKATMoverComponent::DoCalibration()
{
	if (KATDataHandler != nullptr)
	{
		KATDataHandler->Calibrate(nullptr);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("KATVR Calibration"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("KATDataHandler is not initialized"));
	}
}

int32 UKATMoverComponent::GetDeviceCount()
{
	if (KATDataHandler != nullptr)
	{
		return KATDataHandler->DeviceCount();
	}
	return 0;
}

float UKATMoverComponent::GetLastCalibratedTimeEscaped()
{
	if (KATDataHandler != nullptr)
	{
		return KATDataHandler->GetLastCalibratedTimeEscaped();
	}
	return -1.0f;
}

void UKATMoverComponent::VibrateConst(float Amplitude)
{
	if (KATDataHandler != nullptr)
	{
		KATDataHandler->VibrateConst(Amplitude);
	}
}

void UKATMoverComponent::VibrateInSeconds(float Amplitude, float Duration)
{
	if (KATDataHandler != nullptr)
	{
		KATDataHandler->VibrateInSeconds(Amplitude, Duration);
	}
}

void UKATMoverComponent::VibrateOnce(float Amplitude)
{
	if (KATDataHandler != nullptr)
	{
		KATDataHandler->VibrateOnce(Amplitude);
	}
}

void UKATMoverComponent::VibrateFor(float Duration, float Amplitude)
{
	if (KATDataHandler != nullptr)
	{
		KATDataHandler->VibrateFor(Duration, Amplitude);
	}
}

void UKATMoverComponent::LEDConst(float Amplitude)
{
	if (KATDataHandler != nullptr)
	{
		KATDataHandler->LEDConst(Amplitude);
	}
}

void UKATMoverComponent::LEDInSeconds(float Amplitude, float Duration)
{
	if (KATDataHandler != nullptr)
	{
		KATDataHandler->LEDInSeconds(Amplitude, Duration);
	}
}

void UKATMoverComponent::LEDOnce(float Amplitude)
{
	if (KATDataHandler != nullptr)
	{
		KATDataHandler->LEDOnce(Amplitude);
	}
}

void UKATMoverComponent::LEDFor(float Duration, float Frequency, float Amplitude)
{
	if (KATDataHandler != nullptr)
	{
		KATDataHandler->LEDFor(Duration, Frequency, Amplitude);
	}
}

void UKATMoverComponent::ForceConnectDevice()
{
	if (KATDataHandler != nullptr)
	{
		KATDataHandler->ForceConnect(nullptr);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Force connecting to KATVR device..."));
	}
}

bool UKATMoverComponent::IsDeviceConnected()
{
	if (KATDataHandler != nullptr)
	{
		KATTreadMillMemoryData data = KATDataHandler->GetWalkStatus(nullptr);
		return data.treadMillData.connected;
	}
	return false;
}
