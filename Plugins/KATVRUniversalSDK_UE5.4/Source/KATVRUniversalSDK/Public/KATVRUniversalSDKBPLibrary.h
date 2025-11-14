// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "KATSDKWarpper.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KATVRUniversalSDKBPLibrary.generated.h"

#pragma pack(push,1)
USTRUCT(BlueprintType)
struct FKATDeviceDesc
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FString	device;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	bool		isBusy;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FString	serialNumber;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	int		pid;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	int		vid;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	int		deviceType; //0. Err 1. Tread Mill 2. Tracker 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	int		deviceSource;//0.Local; 1.Nexus
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	int		hidUsage;
};

USTRUCT(BlueprintType)
struct FDeviceData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	bool	btnPressed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	bool	isBatteryCharging;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	float	batteryLevel;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	int		firmwareVersion;

	FDeviceData& operator=(const DeviceData& data)
	{
		memcpy(this, &data, sizeof(DeviceData));
		return *this;
	}
};

USTRUCT(BlueprintType)
struct FTreadMillData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FString 	deviceName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	bool		connected;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	float		lastUpdateTimePoint;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FQuat 		bodyRotationRaw;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FVector 	moveSpeed;


	FTreadMillData& operator=(const TreadMillData& data)
	{
		deviceName = data.deviceName;
		connected = data.connected;
		lastUpdateTimePoint = data.lastUpdateTimePoint;
		//Convert From Standard Left Handness to Unreal

		bodyRotationRaw = Quaternion::unityToUnreal(data.bodyRotationRaw);
		

		//Convert Movement to WorldSpace
		moveSpeed = Vector3::unityToUnreal(data.moveSpeed);
		return *this;
	}
};

USTRUCT(BlueprintType)
struct FKATTreadMillMemoryData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FTreadMillData	treadMillData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FDeviceData		deviceDatas0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FDeviceData		deviceDatas1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FDeviceData		deviceDatas2;

	char			extraData[128];
};

//Enviorment Install Path
USTRUCT(BlueprintType)
struct FInstallPaths
{
	GENERATED_USTRUCT_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FString steamInstallPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FString oculusInstallPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KATVRUniversalSDK")
	FString KATLibraryPath;
};

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FBPPinDeviceLost, const FString&, sn, const FString&, message, int, resason);


#pragma pack(pop)
UCLASS()
class UKATVRUniversalSDKBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	// ユーティリティ
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Quaternion Mul Vector", Keywords = "treadmill KAT"), Category = "KATVR|Utility")
	static FVector QuaterMul(const FQuat& q, const FVector& v);

	// デバイス情報
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Connected KAT Treadmill Count", Keywords = "treadmill KAT"), Category = "KATVR|Device")
	static int DeviceCount();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get KAT Treadmill Device Description", Keywords = "treadmill KAT"), Category = "KATVR|Device")
	static FKATDeviceDesc GetDevicesDesc(int index);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Force Connect", Keywords = "treadmill KAT"), Category = "KATVR|Device")
	static void ForceConnect(const FString& sn);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Device Lost Callback", Keywords = "treadmill KAT"), Category = "KATVR|Device")
	static void SetDeviceLostCallback(FBPPinDeviceLost OnDeviceLost);

	// 移動データ
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get KAT Treadmill Walk Status", Keywords = "treadmill KAT"), Category = "KATVR|Movement")
	static FKATTreadMillMemoryData GetWalkStatus(const FString& sn = "");

	// キャリブレーション
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Last Calibrated Time Escaped", Keywords = "treadmill KAT Extension"), Category = "KATVR|Calibration")
	static float GetLastCalibratedTimeEscaped();

	// 振動（Haptics）
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Raw Vibrate Interface", Keywords = "treadmill KAT"), Category = "KATVR|Haptics")
	static void Vibrate(const FString& sn, float amplitude);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Vibrate With Const Amplitude", Keywords = "treadmill KAT Extension"), Category = "KATVR|Haptics")
	static void	VibrateConst(float amplitude);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Vibrate In Seconds", Keywords = "treadmill KAT Extension"), Category = "KATVR|Haptics")
	static void	VibrateInSeconds(float amplitude, float duration);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Vibrate Once", Keywords = "treadmill KAT Extension"), Category = "KATVR|Haptics")
	static void	VibrateOnce(float amplitude);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Vibrate For", Keywords = "treadmill KAT Extension"), Category = "KATVR|Haptics")
	static void	VibrateFor(float duration, float amplitude);

	// LED制御
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Raw LED Interface", Keywords = "treadmill KAT"), Category = "KATVR|LED")
	static void LED(const FString& sn, float amplitude);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LED With Const Amplitude", Keywords = "treadmill KAT Extension"), Category = "KATVR|LED")
	static void	LEDConst(float amplitude);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LED In Seconds", Keywords = "treadmill KAT Extension"), Category = "KATVR|LED")
	static void	LEDInSeconds(float amplitude, float duration);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LED Once", Keywords = "treadmill KAT Extension"), Category = "KATVR|LED")
	static void	LEDOnce(float amplitude);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "LED For", Keywords = "treadmill KAT Extension"), Category = "KATVR|LED")
	static void	LEDFor(float duration, float frequency, float amplitude);
	
private:
	static KATSDKWarpper sdkWarpper;
	
};
