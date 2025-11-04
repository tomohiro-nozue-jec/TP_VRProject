// Fill out your copyright notice in the Description page of Project Settings.


#include "Device/DeviceCommunicator.h"

DeviceCommunicator::DeviceCommunicator(int ID, int Version, const TCHAR* Name)
	: UpdateFrequency(60.0f)
	, bIsRunning(true)
	, bIsPause(false)
	, Device(nullptr)
	, DeviceID(ID)
	, DeviceVer(Version)
	, ProcessState(EProcessState::Idle)
{
	Thread = FRunnableThread::Create(this, Name, 0, TPri_Normal);
}

DeviceCommunicator::~DeviceCommunicator()
{
	if (Thread)
	{
		//Thread->Kill(true);
		EnsureCompletion();
		delete Thread;
		Thread = nullptr;
	}
}

bool DeviceCommunicator::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("Device thread initialized."));
	CreateDevice();
	if (Connect() == ExecutionResult::Fail)
	{
		UE_LOG(LogTemp, Error, TEXT("Device connect failed!"));
	}
	return false;
}

uint32 DeviceCommunicator::Run()
{
	while (bIsRunning)
	{
		if (bIsPause)
			continue;

		double StartTime = FPlatformTime::Seconds();

		// やること

		double Elapsed = FPlatformTime::Seconds() - StartTime;
		double SleepTime = 1.f / UpdateFrequency - Elapsed;

		if (SleepTime > 0)
		{
			// 更新頻度に応じてスリープ
			FPlatformProcess::Sleep(SleepTime);
		}
	}
	return 0;
}

void DeviceCommunicator::Stop()
{
	UE_LOG(LogTemp, Warning, TEXT("Device thread stopped."));

	// デバイスが接続しているなら,切断する
	if (Disconnect() == ExecutionResult::Succ)
	{
		delete Device;
		Device = nullptr;
	}
	// スレッドを止める
	bIsRunning = false;
}

void DeviceCommunicator::Exit()
{

}

void DeviceCommunicator::EnsureCompletion()
{
	Stop();

	// スレッドが存在する場合、完了を待つ
	if (Thread)
	{
		Thread->WaitForCompletion();
	}
}

void DeviceCommunicator::Pause()
{
	bIsPause = true;
}

void DeviceCommunicator::ReStart()
{
	bIsPause = false;
}

ExecutionResult DeviceCommunicator::Disconnect()
{
	if (!Device)
		return ExecutionResult::Fail;

	// デバイスが接続しているなら,切断する
	if (Device->GetConnectionState())
		return Device->DisConnectDevice();

	return ExecutionResult::Fail;
}

void DeviceCommunicator::ReceiveCmd(int Cmd)
{
	CommandQueue.Enqueue(Cmd);
}

void DeviceCommunicator::CreateDevice()
{
	Device = new UASerialLibControllerWin(DeviceID, DeviceVer);
	Device->SetInterfacePt(new WindowsSerial());
}

ExecutionResult DeviceCommunicator::Connect()
{
	if (!Device)
		return ExecutionResult::Fail;

	// デバイスが接続していないなら、接続する
	if(!Device->GetConnectionState())
		return Device->AutoConnectDevice();

	return ExecutionResult::Fail;
}

bool DeviceCommunicator::IsConnected()
{
	if (!Device || !Device->GetConnectionState())
		return false;
	return true;
}

void DeviceCommunicator::HandleCmd()
{
	if (!IsConnected())
		return;

	switch (ProcessState)
	{
		case EProcessState::Idle:
			if (!CommandQueue.IsEmpty())
				ProcessState = EProcessState::Sending;
			break;
		case EProcessState::Sending:
		{
			// 処理するコマンドをゲット
			// ここでコマンドを送ったかどうかを確認するのみ、データを貰う際にもう一度同じコマンドが必要。そのためDequeueではなくPeekを使う
			uint8_t Command;
			CommandQueue.Peek(Command);
			// デバイスにコマンドを送ったら
			if (SendCommand(Command))
			{
				ProcessState = EProcessState::WaitingResponse;
			}
			break;
		}	
		case EProcessState::WaitingResponse:
		{
			// デバイスの方がデータを更新したかを確認
			uint8_t CheckCmd = 0x20;
			if (SendCommand(CheckCmd))
			{
				ASerialDataStruct::ASerialData RecievedData;
				if (RecieveData(RecievedData))
				{
					// 更新された
					if (RecievedData.data[0] == 1)
					{
						ProcessState = EProcessState::Receiving;
					}
				}
			}
			break;
		}
		case EProcessState::Receiving:
		{
			// RecieveDataが失敗した場合、もう一回やるので、確実にデータが貰った後にコマンドを消す
			uint8_t Command;
			CommandQueue.Peek(Command);
			if (SendCommand(Command))
			{
				ASerialDataStruct::ASerialData RecievedData;
				if (RecieveData(RecievedData))
				{
					// 貰ったデータをキューに収納
					DataQueue.Enqueue(RecievedData);
					// コマンドを削除
					CommandQueue.Pop();
					ProcessState = EProcessState::Idle;
				}
			}
			break;
		}
	}
}

bool DeviceCommunicator::SendCommand(uint8_t Command)
{
	if (!IsConnected())
		return false;

	int WriteResult = Device->WriteData(Command);
	if (WriteResult != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to write %d command to device, result: %d"), Command, WriteResult);
		return false;
	}
	return true;
}

bool DeviceCommunicator::RecieveData(ASerialDataStruct::ASerialData& oData)
{
	if (!IsConnected())
		return false;

	int ReadResult = Device->ReadData(&oData);

	if (ReadResult == 0)
	{
		return true;
	}
	else if (ReadResult == -2)
	{
		UE_LOG(LogTemp, Error, TEXT("Read Timeout"));
		return false;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read data from device"));
		return false;
	}
}
