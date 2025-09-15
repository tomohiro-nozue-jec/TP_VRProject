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
		// OneShotタイプのターゲットの状態を管理するマップを初期化
		for (const FEventTarget& EventTarget : TriggerData->EventTargets)
		{
			if (AActor* TargetActor = EventTarget.TargetActor.Get())
			{
				// OneShotタイプのみマップに追加
				if (EventTarget.TargetType == ETargetType::OneShot)
				{
					OneShotActivationStates.Add(TargetActor, false);
				}
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
	// 現在のパズル条件を評価
	bool bConditionMet = EvaluateCondition();

	// 条件が満たされている場合のみ、ターゲットアクターのアクションを実行
	if (bConditionMet)
	{
		for (const FEventTarget& EventTarget : TriggerData->EventTargets)
		{
			if (AActor* TargetActor = EventTarget.TargetActor.Get())
			{
				switch (EventTarget.TargetType)
				{
				case ETargetType::EveryTime:
					// EveryTimeタイプは、条件が満たされるたびに実行
					IActivatableInterface::Execute_OnActivate(TargetActor, SwitchComponent->GetOwner(), true);
					break;
				case ETargetType::OneShot:
					// OneShotタイプは、まだ実行されていなければ実行
					if (!OneShotActivationStates.FindRef(TargetActor))
					{
						IActivatableInterface::Execute_OnActivate(TargetActor, SwitchComponent->GetOwner(), true);
						// 実行済みとしてマーク
						OneShotActivationStates.Add(TargetActor, true);
					}
					break;
				}
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
