// Fill out your copyright notice in the Description page of Project Settings.


#include "Device/DeviceCommunicator.h"
#include "ASerialLibControllerWin.h"

DeviceCommunicator::DeviceCommunicator(const TCHAR* Name)
	: UpdateFrequency(60.0f)
	, bIsRunning(true)
	, bIsPause(false)
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
	// 
	return false;
}

uint32 DeviceCommunicator::Run()
{
	while (bIsRunning)
	{
		if (bIsPause)
			continue;


	}
	return 0;
}

void DeviceCommunicator::Stop()
{
	UE_LOG(LogTemp, Warning, TEXT("Device thread stopped."));
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
