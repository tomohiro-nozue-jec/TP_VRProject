// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/EventTrigger.h"
#include "Interface/ActivatableInterface.h"
#include "Interface/StatefulActivatableInterface.h"
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
	bool bCurrentConditionMet = EvaluateCondition();

	// 【Stateful制御のための条件変化フラグ】
	bool bConditionJustMet = !bLastConditionMet && bCurrentConditionMet;
	bool bConditionJustLost = bLastConditionMet && !bCurrentConditionMet;

	for (const FEventTarget& EventTarget : TriggerData->EventTargets)
	{
		if (AActor* TargetActor = EventTarget.TargetActor.Get())
		{
			switch (EventTarget.TargetType)
			{
			case ETargetType::EveryTime:
				// EveryTime: 条件が満たされた瞬間のみ実行
				if (bConditionJustMet)
				{
					IActivatableInterface::Execute_OnActivate(TargetActor, SwitchComponent->GetOwner(), true);
				}
				break;

			case ETargetType::OneShot:
				// OneShot: 条件が満たされた瞬間、かつまだ実行されていなければ実行
				if (bConditionJustMet && !OneShotActivationStates.FindRef(TargetActor))
				{
					IActivatableInterface::Execute_OnActivate(TargetActor, SwitchComponent->GetOwner(), true);
					OneShotActivationStates.Add(TargetActor, true);
				}
				break;

			case ETargetType::Stateful:
				// Stateful: 条件が変化したときのみ、新しい状態を通知
				if (bConditionJustMet || bConditionJustLost)
				{
					// SetActivationStateインターフェースを呼び出し
					IStatefulActivatableInterface::Execute_SetActivationState(TargetActor, bCurrentConditionMet, SwitchComponent->GetOwner());
				}
				break;
			}
		}
	}

	// 次回の評価のために、現在の条件を保存
	bLastConditionMet = bCurrentConditionMet;
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
