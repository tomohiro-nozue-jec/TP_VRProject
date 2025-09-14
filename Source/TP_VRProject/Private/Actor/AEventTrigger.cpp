// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AEventTrigger.h"
#include "Interface/ActivatableInterface.h"


// Sets default values
AAEventTrigger::AAEventTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAEventTrigger::BeginPlay()
{
	Super::BeginPlay();

    if (TriggerData)
    {
        BindToSwitchDelegates();
    }
}

// Called every frame
void AAEventTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// スイッチのデリゲートにバインドする関数
void AAEventTrigger::BindToSwitchDelegates()
{
    for (const FEventSwitch& EventSwitch : TriggerData->EventSwitches)
    {
        AActor* SwitchActor = EventSwitch.SwitchActor.Get();
        if (SwitchActor)
        {
            USwitchComponent* SwitchComp = Cast<USwitchComponent>(SwitchActor->GetComponentByClass(USwitchComponent::StaticClass()));
            if (SwitchComp)
            {
                SwitchComp->OnSwitchStateChanged.AddDynamic(this, &AAEventTrigger::OnSwitchStateChanged);
                // データアセットで設定されたスイッチタイプをコンポーネントに渡す
                SwitchComp->SwitchType = EventSwitch.SwitchType;
            }
        }
    }
}

// スイッチの状態が変わったときに呼ばれる関数
void AAEventTrigger::OnSwitchStateChanged(USwitchComponent* SwitchComponent, bool bIsOn)
{
    // イベントが既に発動済みであれば、以降の処理は行わない
    //if (bIsTriggered)
    //{
    //    return;
    //}

	// 条件を評価して、必要ならイベントをトリガー
    if (EvaluateCondition())
    {
        // 条件が満たされたら、発動済みフラグを立てて二度と発動しないようにする
        bIsTriggered = true;

        // TargetActors配列をループして、それぞれのアクターを起動
        for (const TSoftObjectPtr<AActor>& TargetRef : TriggerData->TargetActors)
        {
            if (AActor* TargetActor = TargetRef.Get())
            {
                IActivatableInterfac::Execute_OnActivate(TargetActor);
            }
        }
    }
}

// パズルの条件を評価する関数
bool AAEventTrigger::EvaluateCondition()
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
