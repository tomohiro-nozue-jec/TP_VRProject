// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/EventTrigger.h"
#include "Interface/ActivatableInterface.h"
#include "Component/SwitchComponent.h"


// Sets default values
AEventTrigger::AEventTrigger()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AEventTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerData)
	{
		// ターゲットアクターの状態管理マップを初期化
		for (const FEventTarget& EventTarget : TriggerData->EventTargets)
		{
			if (AActor* TargetActor = EventTarget.TargetActor.Get())
			{
				TargetActivationStates.Add(TargetActor, false);
			}
		}

		BindToSwitchDelegates();
	}
}

// スイッチのデリゲートにバインドする関数
void AEventTrigger::BindToSwitchDelegates()
{
	for (const FEventSwitch& EventSwitch : TriggerData->EventSwitches)
	{
		AActor* SwitchActor = EventSwitch.SwitchActor.Get();
		if (SwitchActor)
		{
			USwitchComponent* SwitchComp = Cast<USwitchComponent>(SwitchActor->GetComponentByClass(USwitchComponent::StaticClass()));
			if (SwitchComp)
			{
				SwitchComp->OnSwitchStateChanged.AddDynamic(this, &AEventTrigger::OnSwitchStateChanged);
				SwitchComp->SwitchType = EventSwitch.SwitchType;
			}
		}
	}
}

// スイッチの状態が変わったときに呼ばれる関数
void AEventTrigger::OnSwitchStateChanged(USwitchComponent* SwitchComponent, bool bIsOn)
{
	// 条件を評価
	bool bConditionMet = EvaluateCondition();


	// ターゲットアクターをループ
	for (const FEventTarget& EventTarget : TriggerData->EventTargets)
	{
		if (AActor* TargetActor = EventTarget.TargetActor.Get())
		{
			bool bCurrentState = TargetActivationStates.FindRef(TargetActor);
			bool bNextState = bCurrentState;

			switch (EventTarget.TargetType)
			{
			case ETargetType::OneShot:
				// OneShotタイプはまだ起動していなければ、条件が満たされたら起動
				if (!bCurrentState && bConditionMet)
				{
					bNextState = true;
				}
				break;
			case ETargetType::LatchOn:
				// LatchOnタイプは、条件が満たされたらON
				if (bConditionMet)
				{
					bNextState = true;
				}
				break;
			case ETargetType::Toggle:
				// Toggleタイプは、条件が変化したら状態を反転
				bNextState = bConditionMet;
				break;
			}

			// 状態が変更された場合のみアクションを実行
			if (bNextState != bCurrentState)
			{
				if (bConditionMet)
				{
					IActivatableInterface::Execute_OnActivate(TargetActor, SwitchComponent->GetOwner(), bNextState);
				}
				TargetActivationStates.Add(TargetActor, bNextState);
			}
		}
	}
}

// パズルの条件を評価する関数
bool AEventTrigger::EvaluateCondition()
{
	if (!TriggerData) return false;

	if (TriggerData->SwitchCondition == ESwitchCondition::AND)
	{
		for (const FEventSwitch& EventSwitch : TriggerData->EventSwitches)
		{
			AActor* SwitchActor = EventSwitch.SwitchActor.Get();
			if (!SwitchActor) continue;
			USwitchComponent* SwitchComp = Cast<USwitchComponent>(SwitchActor->GetComponentByClass(USwitchComponent::StaticClass()));
			if (SwitchComp && !SwitchComp->bIsOn)
			{
				return false;
			}
		}
		return true;
	}
	else if (TriggerData->SwitchCondition == ESwitchCondition::OR)
	{
		for (const FEventSwitch& EventSwitch : TriggerData->EventSwitches)
		{
			AActor* SwitchActor = EventSwitch.SwitchActor.Get();
			if (!SwitchActor) continue;
			USwitchComponent* SwitchComp = Cast<USwitchComponent>(SwitchActor->GetComponentByClass(USwitchComponent::StaticClass()));
			if (SwitchComp && SwitchComp->bIsOn)
			{
				return true;
			}
		}
		return false;
	}

	return false;
}
