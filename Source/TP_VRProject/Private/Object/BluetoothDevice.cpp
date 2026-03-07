// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/BluetoothDevice.h"

#if PLATFORM_ANDROID
#include "BleUtils.h"
#include "Interface/BleManagerInterface.h"
#include "Interface/BleDeviceInterface.h"
#include "AndroidPermissionFunctionLibrary.h"
#include "AndroidPermissionCallbackProxy.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogBluetoothDevice, Log, All);

UBluetoothDevice::UBluetoothDevice()
	: BleManager(nullptr)
	, CurrentDevice(nullptr)
	, TargetDeviceName(BLE_DEFAULT_DEVICE_NAME)
	, ConnectionState(EBleConnectionState::Disconnected)
	, bIsOperationInProgress(false)
	, PendingOperationCount(0)
{
}

UBluetoothDevice::~UBluetoothDevice()
{
#if PLATFORM_ANDROID
	if (CurrentDevice && ConnectionState == EBleConnectionState::Connected)
	{
		// デストラクタでは安全のため直接切断（コールバックなし）
		CurrentDevice.GetInterface()->Disconnect({}, {});
	}
#endif
}

//===================================================
// 初期化 / スキャン
//===================================================

bool UBluetoothDevice::Initialize()
{
#if !PLATFORM_ANDROID
	UE_LOG(LogBluetoothDevice, Error, TEXT("BluetoothDevice only supports Android platform."));
	return false;
#endif

#if PLATFORM_ANDROID
	BleManager = UBleUtils::CreateBleManager();

	if (!CheckBluetooth())
		return false;

	// 状態リセット
	ConnectionState = EBleConnectionState::Disconnected;
	ClearQueue();
	ConnectedDeviceName.Empty();
	ConnectedDeviceUUID.Empty();

	RequestAndroidPermission();
	return true;
#endif
}

void UBluetoothDevice::StartScan()
{
#if PLATFORM_ANDROID
	if (!BleManager)
	{
		UE_LOG(LogBluetoothDevice, Error, TEXT("StartScan: BleManager is null. Call Initialize() first."));
		return;
	}

	if (ConnectionState != EBleConnectionState::Disconnected)
	{
		UE_LOG(LogBluetoothDevice, Warning, TEXT("StartScan: Cannot scan while in state %d."), (int32)ConnectionState);
		return;
	}

	UE_LOG(LogBluetoothDevice, Log, TEXT("StartScan: Scanning for device [%s]..."), *TargetDeviceName);
	ConnectionState = EBleConnectionState::Scanning;

	FBleOnDeviceFoundDelegate Function;
	Function.BindUFunction(this, FName("OnDeviceFound"));
	BleManager.GetInterface()->ScanForDevices(ScanServiceFilters, Function);
#endif
}

void UBluetoothDevice::StopScan()
{
#if PLATFORM_ANDROID
	if (BleManager)
	{
		BleManager.GetInterface()->StopScan();
		if (ConnectionState == EBleConnectionState::Scanning)
		{
			ConnectionState = EBleConnectionState::Disconnected;
		}
	}
#endif
}

bool UBluetoothDevice::Disconnect()
{
#if PLATFORM_ANDROID
	if (!CurrentDevice)
	{
		UE_LOG(LogBluetoothDevice, Warning, TEXT("Disconnect: No device connected."));
		return false;
	}

	// 切断時はQueueをクリアして中途半端な操作が残らないようにする
	ClearQueue();

	FBleDelegate SuccFunction;
	SuccFunction.BindUFunction(this, FName("OnDisconnectSucc"));
	FBleErrorDelegate ErrFunction;
	ErrFunction.BindUFunction(this, FName("OnDisconnectError"));
	CurrentDevice.GetInterface()->Disconnect(SuccFunction, ErrFunction);
	ConnectionState = EBleConnectionState::Disconnecting;
	return true;
#endif

	return false;
}

//===================================================
// 設定
//===================================================

void UBluetoothDevice::SetTargetDeviceName(const FString& DeviceName)
{
	TargetDeviceName = DeviceName;
	UE_LOG(LogBluetoothDevice, Log, TEXT("SetTargetDeviceName: [%s]"), *TargetDeviceName);
}

void UBluetoothDevice::AddScanServiceFilter(const FString& ServiceUUID)
{
	ScanServiceFilters.AddUnique(ServiceUUID);
}

void UBluetoothDevice::ClearScanServiceFilter()
{
	ScanServiceFilters.Empty();
}

//===================================================
// Operation Queue（外部API）
//===================================================

void UBluetoothDevice::EnqueueWrite(const FString& ServiceUUID, const FString& CharacteristicUUID, const TArray<uint8>& Data)
{
	EnqueueOperation(FBleOperation::MakeWrite(ServiceUUID, CharacteristicUUID, Data));
}

void UBluetoothDevice::EnqueueSubscribe(const FString& ServiceUUID, const FString& CharacteristicUUID, bool bWithResponse)
{
	EnqueueOperation(FBleOperation::MakeSubscribe(ServiceUUID, CharacteristicUUID, bWithResponse));
}

void UBluetoothDevice::EnqueueRead(const FString& ServiceUUID, const FString& CharacteristicUUID)
{
	EnqueueOperation(FBleOperation::MakeRead(ServiceUUID, CharacteristicUUID));
}

void UBluetoothDevice::ClearQueue()
{
	FBleOperation Dummy;
	while (OperationQueue.Dequeue(Dummy)) {}
	bIsOperationInProgress = false;
	PendingOperationCount = 0;
	UE_LOG(LogBluetoothDevice, Log, TEXT("ClearQueue: Queue cleared."));
}

//===================================================
// 状態取得
//===================================================

EBleConnectionState UBluetoothDevice::GetConnectionState() const
{
	return ConnectionState;
}

bool UBluetoothDevice::IsConnected() const
{
	return ConnectionState == EBleConnectionState::Connected;
}

FString UBluetoothDevice::GetConnectedDeviceName() const
{
	return ConnectedDeviceName;
}

FString UBluetoothDevice::GetConnectedDeviceUUID() const
{
	return ConnectedDeviceUUID;
}

bool UBluetoothDevice::HasPendingOperations() const
{
	return PendingOperationCount > 0 || bIsOperationInProgress;
}

//===================================================
// BLE 内部処理
//===================================================

bool UBluetoothDevice::CheckBluetooth()
{
#if PLATFORM_ANDROID
	if (!BleManager)
	{
		UE_LOG(LogBluetoothDevice, Error, TEXT("CheckBluetooth: BleManager is null."));
		return false;
	}

	if (!BleManager.GetInterface()->IsBleSupported())
	{
		UE_LOG(LogBluetoothDevice, Error, TEXT("CheckBluetooth: BLE is not supported on this device."));
		return false;
	}

	if (!BleManager.GetInterface()->IsBluetoothEnabled())
	{
		UE_LOG(LogBluetoothDevice, Warning, TEXT("CheckBluetooth: Bluetooth is off. Enabling..."));
		BleManager.GetInterface()->SetBluetoothState(true);
	}
	return true;
#endif

	return false;
}

void UBluetoothDevice::RequestAndroidPermission()
{
#if PLATFORM_ANDROID
	if (!UAndroidPermissionFunctionLibrary::CheckPermission(ANDROID_FILE_LOCATION_PERMISSION))
	{
		TArray<FString> Permissions;
		Permissions.Add(ANDROID_FILE_LOCATION_PERMISSION);
		Permissions.Add(ANDROID_BLUETOOTH_CONNECT_PERMISSION);
		Permissions.Add(ANDROID_BLUETOOTH_SCAN_PERMISSION);
		UAndroidPermissionCallbackProxy* Callback = UAndroidPermissionFunctionLibrary::AcquirePermissions(Permissions);
		Callback->OnPermissionsGrantedDynamicDelegate.AddDynamic(this, &UBluetoothDevice::OnPermissionResult);
	}
#endif
}

void UBluetoothDevice::OnPermissionResult(const TArray<FString>& Permissions, const TArray<bool>& GrantResults)
{
	// 権限の結果を必要に応じて処理する（外部へ通知したい場合はDelegate追加）
	UE_LOG(LogBluetoothDevice, Log, TEXT("OnPermissionResult: Received %d permissions."), Permissions.Num());
}

void UBluetoothDevice::OnDeviceFound(TScriptInterface<IBleDeviceInterface> Device)
{
#if PLATFORM_ANDROID
	if (!Device)
		return;

	const FString FoundName = Device.GetInterface()->GetDeviceName();

	// ターゲット名と一致するデバイスだけを対象にする
	if (!FoundName.Equals(TargetDeviceName))
		return;

	UE_LOG(LogBluetoothDevice, Log, TEXT("OnDeviceFound: Found target device [%s]. Connecting..."), *FoundName);

	// スキャンを止めて接続へ
	StopScan();
	CurrentDevice = Device;
	ConnectionState = EBleConnectionState::Connecting;

	FBleDelegate SuccFunction;
	SuccFunction.BindUFunction(this, FName("OnConnectSucc"));
	FBleErrorDelegate ErrFunction;
	ErrFunction.BindUFunction(this, FName("OnConnectError"));
	CurrentDevice.GetInterface()->Connect(SuccFunction, ErrFunction);
#endif
}

void UBluetoothDevice::OnConnectSucc()
{
#if PLATFORM_ANDROID
	ConnectedDeviceName = CurrentDevice.GetInterface()->GetDeviceName();
	ConnectedDeviceUUID = CurrentDevice.GetInterface()->GetDeviceId();
	ConnectionState = EBleConnectionState::Connected;

	UE_LOG(LogBluetoothDevice, Log, TEXT("OnConnectSucc: Connected to [%s] (%s)"), *ConnectedDeviceName, *ConnectedDeviceUUID);

	// コールバックを一度だけ登録する
	FBleCharacteristicDelegate WriteFunction;
	WriteFunction.BindUFunction(this, FName("OnWriteData"));
	CurrentDevice.GetInterface()->BindToCharacteristicWriteEvent(WriteFunction);

	FBleCharacteristicDataDelegate ReceiveFunction;
	ReceiveFunction.BindUFunction(this, FName("OnReceiveData"));
	CurrentDevice.GetInterface()->BindToCharacteristicNotificationEvent(ReceiveFunction);

	// 接続成功を外部へ通知する
	// 外部コードはこのイベントをバインドして、必要なEnqueueWrite/Read/Subscribeを呼ぶ
	OnConnected.Broadcast(ConnectedDeviceName, ConnectedDeviceUUID);
#endif
}

void UBluetoothDevice::OnConnectError(FString ErrorMessage)
{
	UE_LOG(LogBluetoothDevice, Error, TEXT("OnConnectError: %s"), *ErrorMessage);
	ConnectionState = EBleConnectionState::Disconnected;
	CurrentDevice = nullptr;
	OnConnectFailed.Broadcast(ErrorMessage);
}

void UBluetoothDevice::OnDisconnectSucc()
{
	UE_LOG(LogBluetoothDevice, Log, TEXT("OnDisconnectSucc: Disconnected from [%s]"), *ConnectedDeviceName);
	ConnectedDeviceName.Empty();
	ConnectedDeviceUUID.Empty();
	ConnectionState = EBleConnectionState::Disconnected;
	CurrentDevice = nullptr;
	OnDisconnected.Broadcast();
}

void UBluetoothDevice::OnDisconnectError(FString ErrorMessage)
{
	UE_LOG(LogBluetoothDevice, Error, TEXT("OnDisconnectError: %s"), *ErrorMessage);
	// 切断失敗 → 接続状態に戻す
	ConnectionState = EBleConnectionState::Connected;
}

void UBluetoothDevice::OnWriteData(FString ServiceUUID, FString CharacteristicUUID)
{
	UE_LOG(LogBluetoothDevice, Log, TEXT("OnWriteData: Write completed [%s / %s]"), *ServiceUUID, *CharacteristicUUID);

	// Write完了を外部へ通知する（外部が次のEnqueueを積むタイミングに使える）
	OnWriteCompleted.Broadcast(ServiceUUID, CharacteristicUUID);

	OnOperationCompleted();
}

void UBluetoothDevice::OnReceiveData(FString ServiceUUID, FString CharacteristicUUID, TArray<uint8>& Data)
{
	UE_LOG(LogBluetoothDevice, Log, TEXT("OnReceiveData: Received data from [%s / %s], Length=%d"),
		*ServiceUUID, *CharacteristicUUID, Data.Num());

	// 受信データを外部へ通知する（データの解釈は外部で行う）
	OnDataReceived.Broadcast(ServiceUUID, CharacteristicUUID, Data);

	// ReadCharacteristic の応答は Queue の完了扱い
	// Subscribe の継続通知は Queue 管理外のため OnOperationCompleted は呼ばない
	// → 現在 Queue に Read が積まれており実行中の場合のみ完了とみなす
	if (bIsOperationInProgress)
	{
		OnOperationCompleted();
	}
}

//===================================================
// Operation Queue 内部処理
//===================================================

void UBluetoothDevice::EnqueueOperation(const FBleOperation& Operation)
{
	if (!IsConnected())
	{
		UE_LOG(LogBluetoothDevice, Warning,
			TEXT("EnqueueOperation: Device is not connected. Operation [Type=%d, %s / %s] will be queued but won't execute until connected."),
			(int32)Operation.Type, *Operation.ServiceUUID, *Operation.CharacteristicUUID);
	}

	OperationQueue.Enqueue(Operation);
	PendingOperationCount++;

	UE_LOG(LogBluetoothDevice, Log, TEXT("EnqueueOperation: Type=%d, Service=%s, Characteristic=%s (Pending=%d)"),
		(int32)Operation.Type, *Operation.ServiceUUID, *Operation.CharacteristicUUID, PendingOperationCount);

	// 実行中でなければすぐに処理を開始する
	if (!bIsOperationInProgress)
	{
		ProcessNextOperation();
	}
}

void UBluetoothDevice::ProcessNextOperation()
{
#if PLATFORM_ANDROID
	if (OperationQueue.IsEmpty())
	{
		UE_LOG(LogBluetoothDevice, Log, TEXT("ProcessNextOperation: Queue is empty."));
		OnQueueEmpty.Broadcast();
		return;
	}

	if (!IsConnected())
	{
		UE_LOG(LogBluetoothDevice, Warning, TEXT("ProcessNextOperation: Device is not connected. Waiting..."));
		return;
	}

	FBleOperation NextOp;
	if (!OperationQueue.Dequeue(NextOp))
		return;

	PendingOperationCount = FMath::Max(0, PendingOperationCount - 1);
	bIsOperationInProgress = true;

	UE_LOG(LogBluetoothDevice, Log, TEXT("ProcessNextOperation: Executing Type=%d, Service=%s, Characteristic=%s"),
		(int32)NextOp.Type, *NextOp.ServiceUUID, *NextOp.CharacteristicUUID);

	switch (NextOp.Type)
	{
	case EBleOperationType::WriteCharacteristic:
	{
		CurrentDevice.GetInterface()->WriteCharacteristic(
			NextOp.ServiceUUID,
			NextOp.CharacteristicUUID,
			NextOp.Data
		);
		// 完了は OnWriteData コールバック → OnOperationCompleted()
		break;
	}

	case EBleOperationType::SubscribeCharacteristic:
	{
		CurrentDevice.GetInterface()->SubscribeToCharacteristic(
			NextOp.ServiceUUID,
			NextOp.CharacteristicUUID,
			NextOp.bWithResponse
		);
		// SubscribeToCharacteristic は完了コールバックがないため即完了とみなす
		OnOperationCompleted();
		break;
	}

	case EBleOperationType::ReadCharacteristic:
	{
		CurrentDevice.GetInterface()->ReadCharacteristic(
			NextOp.ServiceUUID,
			NextOp.CharacteristicUUID
		);
		// 完了は OnReceiveData コールバック → OnOperationCompleted()
		break;
	}

	default:
	{
		UE_LOG(LogBluetoothDevice, Error, TEXT("ProcessNextOperation: Unknown operation type %d."), (int32)NextOp.Type);
		OnOperationCompleted();
		break;
	}
	}
#endif
}

void UBluetoothDevice::OnOperationCompleted()
{
	bIsOperationInProgress = false;
	UE_LOG(LogBluetoothDevice, Log, TEXT("OnOperationCompleted: Moving to next. (Remaining=%d)"), PendingOperationCount);
	ProcessNextOperation();
}