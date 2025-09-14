// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SwitchComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSwitchStateChanged, USwitchComponent*, SwitchComponent, bool, bIsOn);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TP_VRPROJECT_API USwitchComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USwitchComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Switch")
	bool bIsOn = false;

	UPROPERTY(BlueprintAssignable, Category = "Switch")
	FOnSwitchStateChanged OnSwitchStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Switch")
	void ToggleSwitch();
};
