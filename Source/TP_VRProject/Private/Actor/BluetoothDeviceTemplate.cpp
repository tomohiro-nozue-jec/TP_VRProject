// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BluetoothDeviceTemplate.h"
#include "Object/BluetoothDevice.h"

// Sets default values
ABluetoothDeviceTemplate::ABluetoothDeviceTemplate()
	: BluetoothDevice(nullptr)
	, bIsInitialized(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ABluetoothDeviceTemplate::BeginPlay()
{
	Super::BeginPlay();
	
	// ─── Step 1: UBluetoothDevice を生成する ───────────────────────────
	BluetoothDevice = NewObject<UBluetoothDevice>(this);

	// ─── Step 2: イベントをバインドする ────────────────────────────────
	// すべてのバインドは Initialize / StartScan の前に行う
	BluetoothDevice->OnConnected.AddDynamic(this, &ABluetoothDeviceTemplate::OnBleConnected);
	BluetoothDevice->OnConnectFailed.AddDynamic(this, &ABluetoothDeviceTemplate::OnBleConnectFailed);
	BluetoothDevice->OnDisconnected.AddDynamic(this, &ABluetoothDeviceTemplate::OnBleDisconnected);
	BluetoothDevice->OnWriteCompleted.AddDynamic(this, &ABluetoothDeviceTemplate::OnBleWriteCompleted);
	BluetoothDevice->OnDataReceived.AddDynamic(this, &ABluetoothDeviceTemplate::OnBleDataReceived);
	BluetoothDevice->OnQueueEmpty.AddDynamic(this, &ABluetoothDeviceTemplate::OnBleQueueEmpty);

	// ─── Step 3: 接続先デバイス名を設定する ────────────────────────────
	// デフォルトは BLE_DEFAULT_DEVICE_NAME ("DefaultDevice")
	// 別のデバイスを使う場合はここで上書きする
	BluetoothDevice->SetTargetDeviceName(TEXT("DefaultDevice"));

	// ─── Step 4: 初期化してスキャンを開始する ──────────────────────────
	if (!BluetoothDevice->Initialize())
	{
		UE_LOG(LogTemp, Error, TEXT("BeginPlay: BluetoothDevice initialization failed."));
		return;
	}

	BluetoothDevice->StartScan();
	UE_LOG(LogTemp, Log, TEXT("BeginPlay: Scanning for BLE device..."));
}

void ABluetoothDeviceTemplate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Actor 破棄時に接続が残っていれば切断する
	if (BluetoothDevice && BluetoothDevice->IsConnected())
	{
		BluetoothDevice->Disconnect();
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void ABluetoothDeviceTemplate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//===================================================
// UBluetoothDevice イベントハンドラ
//===================================================

void ABluetoothDeviceTemplate::OnBleConnected(const FString& DeviceName, const FString& DeviceUUID)
{
	UE_LOG(LogTemp, Log, TEXT("OnBleConnected: Device=[%s] UUID=[%s]"), *DeviceName, *DeviceUUID);

	// ─── 初期化シーケンス Step A ───────────────────────────────────────
	// ペアリング要求を Write する。
	// この Write が完了すると OnBleWriteCompleted が呼ばれ、
	// そこで Subscribe を Enqueue する（Step B へ続く）。
	TArray<uint8> PairData;
	PairData.Add(1);
	BluetoothDevice->EnqueueWrite(TEMPLATE_PAIR_SERVICE_UUID, TEMPLATE_PAIR_CHARACTERISTIC_UUID, PairData);

	UE_LOG(LogTemp, Log, TEXT("OnBleConnected: Enqueued pairing Write."));
}

void ABluetoothDeviceTemplate::OnBleConnectFailed(const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Error, TEXT("OnBleConnectFailed: %s"), *ErrorMessage);

	// 必要に応じて再スキャンを試みる
	// BluetoothDevice->StartScan();
}

void ABluetoothDeviceTemplate::OnBleDisconnected()
{
	UE_LOG(LogTemp, Log, TEXT("OnBleDisconnected: Connection lost."));
	bIsInitialized = false;

	// 必要に応じて再接続を試みる
	// BluetoothDevice->StartScan();
}

void ABluetoothDeviceTemplate::OnBleWriteCompleted(const FString& ServiceUUID, const FString& CharacteristicUUID)
{
	UE_LOG(LogTemp, Log, TEXT("OnBleWriteCompleted: [%s / %s]"), *ServiceUUID, *CharacteristicUUID);

	// ─── 初期化シーケンス Step B ───────────────────────────────────────
	// ペアリング Write が完了したら、データ通知の Subscribe を Enqueue する。
	// Subscribe は順番に実行されるので、2つ続けて Enqueue しても問題ない。
	if (ServiceUUID.Equals(TEMPLATE_PAIR_SERVICE_UUID) &&	CharacteristicUUID.Equals(TEMPLATE_PAIR_CHARACTERISTIC_UUID))
	{
		//BluetoothDevice->EnqueueSubscribe(TEMPLATE_BIKE_SERVICE_UUID, TEMPLATE_RPM_CHARACTERISTIC_UUID);

		UE_LOG(LogTemp, Log, TEXT("OnBleWriteCompleted: Enqueued RPM and Revolution Subscribe."));
	}
}

void ABluetoothDeviceTemplate::OnBleDataReceived(const FString& ServiceUUID, const FString& CharacteristicUUID,	const TArray<uint8>& Data)
{
	if (Data.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnBleDataReceived: Empty data from [%s / %s]"), *ServiceUUID, *CharacteristicUUID);
		return;
	}

	// ServiceUUID / CharacteristicUUID で処理を振り分ける
	
}

void ABluetoothDeviceTemplate::OnBleQueueEmpty()
{
	// 初期化シーケンス完了（Write → Subscribe がすべて終わった）
	if (!bIsInitialized)
	{
		bIsInitialized = true;
		UE_LOG(LogTemp, Log, TEXT("OnBleQueueEmpty: Initialization sequence complete. Ready to receive data."));
	}
}

//===================================================
// 受信データ処理
//===================================================


//===================================================
// ユーティリティ
//===================================================

template<typename T>
T ABluetoothDeviceTemplate::BytesToInt(const uint8* Data, int32 Size) const
{
	// ビッグエンディアン: Data[0] が最上位バイト
	T Result = 0;
	for (int32 Index = 0; Index < Size; ++Index)
	{
		Result |= static_cast<T>(Data[Index]) << (8 * (Size - 1 - Index));
	}
	return Result;
}