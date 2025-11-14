// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/KATMoverComponent.h"
#include "KATSDKWarpper.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"

// ログカテゴリの定義
DEFINE_LOG_CATEGORY_STATIC(LogKATMover, Log, All);

UKATMoverComponent::UKATMoverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentSpeed = 0.0f;
	KATDataHandler = nullptr;
}

UKATMoverComponent::~UKATMoverComponent()
{
	// EndPlayが呼ばれなかった場合のセーフ
	if (KATDataHandler)
	{
		delete KATDataHandler;
		KATDataHandler = nullptr;
		UE_LOG(LogKATMover, Warning, TEXT("KATDataHandler was not released in EndPlay, cleaned up in destructor"));
	}
	UE_LOG(LogKATMover, Log, TEXT("KATMoverComponent destroyed"));
}

void UKATMoverComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!InitializeDevice())
	{
		UE_LOG(LogKATMover, Error, TEXT("Failed to initialize KATVR device"));
		if (bShowDebugMessages)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to initialize KATVR device"));
		}
		return;
	}
	
	// VRカメラの検索
	VRCamera = Cast<UCameraComponent>(GetOwner()->FindComponentByClass(UCameraComponent::StaticClass()));
	if (!VRCamera)
	{
		UE_LOG(LogKATMover, Warning, TEXT("VR Camera not found. Movement direction will be based on actor forward vector"));
	}
}

void UKATMoverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ShutdownDevice();
	Super::EndPlay(EndPlayReason);
}

void UKATMoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (KATDataHandler)
	{
		HandleKATVRInput(DeltaTime);
	}
}

//=============================================================================
// 内部処理関数
//=============================================================================

bool UKATMoverComponent::InitializeDevice()
{
	// 既に初期化されている場合は何もしない
	if (KATDataHandler)
	{
		UE_LOG(LogKATMover, Warning, TEXT("Device already initialized"));
		return true;
	}

	try
	{
		// 生のポインタを使用（手動でメモリ管理）
		KATDataHandler = new KATSDKWarpper();
		
		if (!KATDataHandler)
		{
			UE_LOG(LogKATMover, Error, TEXT("Failed to create KATSDKWarpper"));
			return false;
		}
		
		// デバイス数をチェック
		int32 DeviceCount = KATDataHandler->DeviceCount();
		if (DeviceCount > 0)
		{
			UE_LOG(LogKATMover, Log, TEXT("KATVR Devices Found: %d"), DeviceCount);
			if (bShowDebugMessages)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
					FString::Printf(TEXT("KATVR Devices Found: %d"), DeviceCount));
			}
			return true;
		}
		else
		{
			UE_LOG(LogKATMover, Warning, TEXT("No KATVR Devices Found"));
			if (bShowDebugMessages)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No KATVR Devices Found"));
			}
			// デバイスが見つからなくてもエラーにはしない（後で接続される可能性）
			return true;
		}
	}
	catch (const std::exception& e)
	{
		UE_LOG(LogKATMover, Error, TEXT("Exception during device initialization: %s"), ANSI_TO_TCHAR(e.what()));
		return false;
	}
	catch (...)
	{
		UE_LOG(LogKATMover, Error, TEXT("Unknown exception during device initialization"));
		return false;
	}
}

void UKATMoverComponent::ShutdownDevice()
{
	if (KATDataHandler)
	{
		UE_LOG(LogKATMover, Log, TEXT("Shutting down KATVR device"));
		delete KATDataHandler;
		KATDataHandler = nullptr;
	}
}

void UKATMoverComponent::HandleKATVRInput(float DeltaTime)
{
	if (!KATDataHandler || !CanMove)
	{
		CurrentSpeed = 0.0f;
		return;
	}
	
	// 1回だけデータを取得
	KATTreadMillMemoryData data = KATDataHandler->GetWalkStatus(nullptr);
	
	HandleKATVRInputWalk(DeltaTime, data);
	HandleKATVRRotator(data);
}

void UKATMoverComponent::HandleKATVRInputWalk(float DeltaTime, const KATTreadMillMemoryData& data)
{
	// 前後移動速度（Z軸）を取得
	float ForwardSpeed = data.treadMillData.moveSpeed.z;
	
	// 移動速度の絶対値を計算
	float SpeedMagnitude = FMath::Abs(ForwardSpeed);
	CurrentSpeed = SpeedMagnitude;

	// 前方ベクトルを決定（VRカメラ優先、なければアクターの前方）
	FVector HorizontalForward;
	if (VRCamera)
	{
		FVector CameraForward = VRCamera->GetForwardVector();
		HorizontalForward = FVector(CameraForward.X, CameraForward.Y, 0.0f).GetSafeNormal();
	}
	else if (GetOwner())
	{
		// フォールバック(アクターの前方ベクトルを使用)
		FVector ActorForward = GetOwner()->GetActorForwardVector();
		HorizontalForward = FVector(ActorForward.X, ActorForward.Y, 0.0f).GetSafeNormal();
	}
	else
	{
		UE_LOG(LogKATMover, Error, TEXT("Owner is null, cannot determine movement direction"));
		return;
	}

	// 移動を適用
	FVector Movement = HorizontalForward * ForwardSpeed * MoveSpeed * DeltaTime;
	GetOwner()->AddActorWorldOffset(Movement, true);
}

void UKATMoverComponent::HandleKATVRRotator(const KATTreadMillMemoryData& data)
{
	// KATVRから回転データを取得
	FQuat NewQuat = Quaternion::unityToUnreal(data.treadMillData.bodyRotationRaw);
	
	RotateCharacterByFQuat(NewQuat, 0.5f);
}

void UKATMoverComponent::RotateCharacterByFQuat(FQuat TargetQuat, float Duration)
{
	if (!GetOwner())
	{
		return;
	}

	// 現在の回転を計算
	CurrentRotator = TargetQuat.Rotator();
	FRotator DeltaRotation = CurrentRotator - PreRotator;

	// Yaw軸を反転（KATVRとUnrealの座標系の違いを補正）
	DeltaRotation.Yaw = -DeltaRotation.Yaw;

	// キャラクターの目標回転を計算
	FRotator CharacterTargetRotator = GetOwner()->GetActorRotation() + DeltaRotation;

	// VRオフセット回転を計算
	FRotator VROffsetRotator = GetRelativeRotation() - DeltaRotation;

	// 回転を適用
	GetOwner()->SetActorRotation(CharacterTargetRotator);
	SetRelativeRotation(VROffsetRotator);

	// 次回の計算のために保存
	PreRotator = CurrentRotator;
}

//=============================================================================
// 移動制御
//=============================================================================

void UKATMoverComponent::StartMove()
{
	CanMove = true;
	UE_LOG(LogKATMover, Log, TEXT("Movement enabled"));
}

void UKATMoverComponent::StopMove()
{
	CanMove = false;
	CurrentSpeed = 0.0f;
	UE_LOG(LogKATMover, Log, TEXT("Movement disabled"));
}

//=============================================================================
// キャリブレーション
//=============================================================================

void UKATMoverComponent::DoCalibration()
{
	if (!KATDataHandler)
	{
		UE_LOG(LogKATMover, Error, TEXT("KATDataHandler is not initialized"));
		if (bShowDebugMessages)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("KATDataHandler is not initialized"));
		}
		return;
	}

	KATDataHandler->Calibrate(nullptr);
	UE_LOG(LogKATMover, Log, TEXT("KATVR Calibration executed"));
	
	if (bShowDebugMessages)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("KATVR Calibration"));
	}
}

float UKATMoverComponent::GetLastCalibratedTimeEscaped()
{
	if (KATDataHandler)
	{
		return KATDataHandler->GetLastCalibratedTimeEscaped();
	}
	return -1.0f;
}

//=============================================================================
// デバイス情報
//=============================================================================

int32 UKATMoverComponent::GetDeviceCount()
{
	if (KATDataHandler)
	{
		return KATDataHandler->DeviceCount();
	}
	return 0;
}

void UKATMoverComponent::ForceConnectDevice()
{
	if (!KATDataHandler)
	{
		UE_LOG(LogKATMover, Error, TEXT("KATDataHandler is not initialized"));
		return;
	}

	KATDataHandler->ForceConnect(nullptr);
	UE_LOG(LogKATMover, Log, TEXT("Force connecting to KATVR device..."));
	
	if (bShowDebugMessages)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Force connecting to KATVR device..."));
	}
}

bool UKATMoverComponent::IsDeviceConnected()
{
	if (KATDataHandler)
	{
		KATTreadMillMemoryData data = KATDataHandler->GetWalkStatus(nullptr);
		return data.treadMillData.connected;
	}
	return false;
}

//=============================================================================
// ハプティックフィードバック（振動）
//=============================================================================

void UKATMoverComponent::VibrateConst(float Amplitude)
{
	if (KATDataHandler)
	{
		KATDataHandler->VibrateConst(Amplitude);
	}
}

void UKATMoverComponent::VibrateInSeconds(float Amplitude, float Duration)
{
	if (KATDataHandler)
	{
		KATDataHandler->VibrateInSeconds(Amplitude, Duration);
	}
}

void UKATMoverComponent::VibrateOnce(float Amplitude)
{
	if (KATDataHandler)
	{
		KATDataHandler->VibrateOnce(Amplitude);
	}
}

void UKATMoverComponent::VibrateFor(float Duration, float Amplitude)
{
	if (KATDataHandler)
	{
		KATDataHandler->VibrateFor(Duration, Amplitude);
	}
}

//=============================================================================
// LED制御
//=============================================================================

void UKATMoverComponent::LEDConst(float Amplitude)
{
	if (KATDataHandler)
	{
		KATDataHandler->LEDConst(Amplitude);
	}
}

void UKATMoverComponent::LEDInSeconds(float Amplitude, float Duration)
{
	if (KATDataHandler)
	{
		KATDataHandler->LEDInSeconds(Amplitude, Duration);
	}
}

void UKATMoverComponent::LEDOnce(float Amplitude)
{
	if (KATDataHandler)
	{
		KATDataHandler->LEDOnce(Amplitude);
	}
}

void UKATMoverComponent::LEDFor(float Duration, float Frequency, float Amplitude)
{
	if (KATDataHandler)
	{
		KATDataHandler->LEDFor(Duration, Frequency, Amplitude);
	}
}

//=============================================================================
// 静的ユーティリティ関数
//=============================================================================

UKATMoverComponent* UKATMoverComponent::FindKATMoverComponent(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// ワールド内の全てのアクターを検索
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor)
		{
			UKATMoverComponent* Component = Actor->FindComponentByClass<UKATMoverComponent>();
			if (Component)
			{
				return Component;
			}
		}
	}

	UE_LOG(LogKATMover, Warning, TEXT("KATMoverComponent not found in world"));
	return nullptr;
}

UKATMoverComponent* UKATMoverComponent::GetKATMoverComponentFromActor(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass<UKATMoverComponent>();
}
