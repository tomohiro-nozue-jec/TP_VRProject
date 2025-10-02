#include "Component/KATMoverComponent.h"
#include "Camera/CameraComponent.h"
#include "Component/KATHub.h"
#include "GameFramework/Actor.h"


UKATMoverComponent::UKATMoverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UKATMoverComponent::BeginPlay()
{
	Super::BeginPlay();

	KATDataHandler = new KATHub(); // KATHubのインスタンスを生成
	VRCamera = GetOwner()->FindComponentByClass<UCameraComponent>();
}


// Called every frame
void UKATMoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (KATDataHandler)
	{
		HandleKATVRInput(DeltaTime);
	}
}

void UKATMoverComponent::StartMove()
{
	CanMove = true;
}

void UKATMoverComponent::StopMove()
{
	CanMove = false;
}

void UKATMoverComponent::Calibrate()
{
	if (KATDataHandler)
	{
		KATDataHandler->Calibrate(nullptr);
		UE_LOG(LogTemp, Log, TEXT("KATVR Calibration Requested."));
	}
}

void UKATMoverComponent::HandleKATVRInput(float DeltaTime)
{
	HandleKATVRInputWalk(DeltaTime);
	HandleKATVRRotator();
}

void UKATMoverComponent::HandleKATVRInputWalk(float DeltaTime)
{
	if (!CanMove || !VRCamera) return;

	auto walkStatus = KATDataHandler->GetWalkStatus(nullptr);
	if (!walkStatus.treadMillData.connected) return;

	Vector3 MoveSpeed = walkStatus.treadMillData.moveSpeed;

	float MoveAmount = MoveSpeed.z;
	FVector CameraForward = VRCamera->GetForwardVector();
	CameraForward.Z = 0; // XY平面での移動に限定
	CameraForward.Normalize();

	// DeltaTimeを乗算してフレームレートに依存しない移動を実現
	FVector Offset = CameraForward * MoveAmount * NowSpeed * DeltaTime;

	GetOwner()->AddActorWorldOffset(Offset, true);
}

void UKATMoverComponent::HandleKATVRRotator()
{
	if (!VRCamera) return;

	auto walkStatus = KATDataHandler->GetWalkStatus(nullptr);
	if (!walkStatus.treadMillData.connected) return;

	FQuat newQuat = Quaternion::unityToUnreal(walkStatus.treadMillData.bodyRotationRaw);

	CurrentRotator = newQuat.Rotator();
	FRotator DeltaRotation = CurrentRotator - PreRotator;

	// Yawの回転方向を調整
	DeltaRotation.Yaw = -DeltaRotation.Yaw;

	// Actorの現在の回転に差分を加える
	FRotator CharacterTargetRotator = GetOwner()->GetActorRotation() + DeltaRotation;
	GetOwner()->SetActorRotation(CharacterTargetRotator);

	PreRotator = CurrentRotator;
}