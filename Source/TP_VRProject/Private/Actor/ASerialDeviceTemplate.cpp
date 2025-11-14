// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ASerialDeviceTemplate.h"
#include "ASerialComBPLibrary.h"
#include "Device/WiredDevice.h"

// Sets default values
AASerialDeviceTemplate::AASerialDeviceTemplate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AASerialDeviceTemplate::BeginPlay()
{
	Super::BeginPlay();

	// デバイスの初期化
	InitializeDevice();

	// シリアルデバイスの作成, ID, バージョン, 名前を指定する
	SerialDevice = UASerialComBPLibrary::CreateDevice(WiredDeviceID, WiredDeviceVer, "Test");
	if (SerialDevice)
	{
		// デバイスからデータを受け取ったときのイベントを登録
		SerialDevice->OnRecieveDataEvent.AddDynamic(this, &AASerialDeviceTemplate::OnDataReceived);
	}
}

// Called every frame
void AASerialDeviceTemplate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AASerialDeviceTemplate::SendCmd()
{
	if (SerialDevice && SerialDevice->IsConnecting())
	{
		SerialDevice->RequestData(RPMCmd);
	}
}

void AASerialDeviceTemplate::InitializeDevice_Implementation()
{
	// これはエルゴメーターのデータ、実際どうなるかは電子科から貰った資料を見てください
	// 有線デバイスのIDとバージョンを設定
	WiredDeviceID = 0x03;
	WiredDeviceVer = 0x03;

	// 取得したいデータのコマンドを設定
	RPMCmd = 0x21;
}

void AASerialDeviceTemplate::OnDataReceived(int Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Received Data: %d"), Value));
	// ここで受け取ったデータを処理

	// ずっとデータを要求し続ける場合、前のデータを受け取った後に再度コマンドを送信
	UE_LOG(LogTemp, Display, TEXT("Send Command Again"));
	SendCmd();
}

