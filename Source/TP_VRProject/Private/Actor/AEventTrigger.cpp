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
    for (const TSoftObjectPtr<AActor>& SwitchRef : TriggerData->TriggerSwitches)
    {
        AActor* SwitchActor = SwitchRef.Get();
        if (SwitchActor)
        {
            USwitchComponent* SwitchComp = Cast<USwitchComponent>(SwitchActor->GetComponentByClass(USwitchComponent::StaticClass()));
            if (SwitchComp)
            {
                SwitchComp->OnSwitchStateChanged.AddDynamic(this, &AAEventTrigger::OnSwitchStateChanged);
            }
        }
    }
}

// スイッチの状態が変わったときに呼ばれる関数
void AAEventTrigger::OnSwitchStateChanged(USwitchComponent* SwitchComponent, bool bIsOn)
{
    if (EvaluateCondition())
    {
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
        for (const TSoftObjectPtr<AActor>& SwitchRef : TriggerData->TriggerSwitches)
        {
            AActor* SwitchActor = SwitchRef.Get();
            if (!SwitchActor) continue;
            USwitchComponent* SwitchComp = Cast<USwitchComponent>(SwitchActor->GetComponentByClass(USwitchComponent::StaticClass()));
            if (SwitchComp && !SwitchComp->bIsOn)
            {
                return false; // 1つでもOFFならAND条件は満たされない
            }
        }
        return true; // 全てON
    }
    else if (TriggerData->SwitchCondition == ESwitchCondition::OR)
    {
        for (const TSoftObjectPtr<AActor>& SwitchRef : TriggerData->TriggerSwitches)
        {
            AActor* SwitchActor = SwitchRef.Get();
            if (!SwitchActor) continue;
            USwitchComponent* SwitchComp = Cast<USwitchComponent>(SwitchActor->GetComponentByClass(USwitchComponent::StaticClass()));
            if (SwitchComp && SwitchComp->bIsOn)
            {
                return true; // 1つでもONならOR条件は満たされる
            }
        }
        return false; // 全てOFF
    }

    return false;
}
