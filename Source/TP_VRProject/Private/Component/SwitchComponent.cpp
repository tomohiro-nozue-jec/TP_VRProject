// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/SwitchComponent.h"

// Sets default values for this component's properties
USwitchComponent::USwitchComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USwitchComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USwitchComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// スイッチの状態を切り替える関数
void USwitchComponent::ToggleSwitch()
{
    // SwitchTypeに応じてロジックを分岐
    if (SwitchType == ESwitchType::Toggle)
    {
        // トグル式：状態を反転
        bIsOn = !bIsOn;
        // 状態変更を通知
        OnSwitchStateChanged.Broadcast(this, bIsOn);
    }
    else if (SwitchType == ESwitchType::LatchOn)
    {
        // ラッチ式：まだオフの場合のみオンにし、それ以降は状態を維持
        if (!bIsOn)
        {
            bIsOn = true;
            // 状態変更を通知
            OnSwitchStateChanged.Broadcast(this, bIsOn);
        }
    }
}