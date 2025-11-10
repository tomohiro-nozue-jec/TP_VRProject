#pragma once

#include "Core.h"
#include <functional>

#pragma pack(push,1)
struct KATDeviceDesc
{
	char	device[63];
	bool	isBusy;
	char	serialNumber[13];
	int		pid;
	int		vid;
	short	deviceType; //0. Err 1. Tread Mill 2. Tracker 
	short	deviceSource; //0. Local. 1.Nexus
	int     hidUsage;
};

struct DeviceData
{
	bool	btnPressed;
	bool	isBatteryCharging;
	float	batteryLevel;
	char	firmwareVersion;
};

struct Quaternion
{
public:
	float x;
	float y;
	float z;
	float w;

	static FQuat toFQuat(const Quaternion& q)
	{
		return FQuat(q.x, q.y, q.z, q.w);
	}

	static FQuat unityToUnreal(const Quaternion& uq)
	{
		auto fq = toFQuat(uq);
		FVector axis;
		float angle;
		fq.ToAxisAndAngle(axis, angle);
		axis = FVector(axis.Z, -axis.X, axis.Y);

		return FQuat(FVector::UpVector, FMath::DegreesToRadians(90.0f)) * FQuat(axis, -angle);
	}
};

struct Vector3
{
public:
	float x;
	float y;
	float z;

	static FVector toFVector(const Vector3 v)
	{
		return FVector(v.x, v.y, v.z);
	}

	static FVector unityToUnreal(const Vector3 uv)
	{
		return FVector(-uv.x, uv.z, uv.y);
	}
};

struct TreadMillData
{
	char 		deviceName[64];
	bool		connected;
	double		lastUpdateTimePoint;

	Quaternion 		bodyRotationRaw;

	Vector3 	moveSpeed;
};

struct KATTreadMillMemoryData
{
	TreadMillData	treadMillData;

	DeviceData  deviceDatas[3];

	char		extraData[128];
};

struct InstallPaths
{
	char steamInstallPath[1024];
	char oculusInstallPath[1024];
	char KATLibraryPath[1024];
};

class KATSDKWarpper
{
private:
	void* _sdkHandle;

	void* handle()
	{
		//auto sdkPath = FPaths::ProjectPluginsDir() + "KATVRUniversalSDK/ThirdParty/KAT/" + "KATSDKWarpper.dll";
		auto sdkPath = PLATFORM_WINDOWS ? (FPaths::ProjectPluginsDir() + "KATVRUniversalSDK_UE5.4/ThirdParty/KAT/" + "KATSDKWarpper.dll") : "libKATSDKWarpper.so";
		if (_sdkHandle == nullptr)
		{
			auto arr = *sdkPath;

#if PLATFORM_WINDOWS
			if (FPaths::FileExists(sdkPath))
#endif
			{
				_sdkHandle = FPlatformProcess::GetDllHandle(arr);
			}

			if (_sdkHandle == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("KAT Native SDK Not Found!"));
			}
		}

		//GEngine->AddOnScreenDebugMessage(-1, 20, FColor::White, *sdkPath);
		return _sdkHandle;
	}

	std::function<int(void)>							deviceCountFunc;
	std::function<KATDeviceDesc(unsigned)>				getDevicesDescFunc;
	std::function<KATTreadMillMemoryData(const char*)>	getWalkStatusFunc;
	std::function<void(const char*)>					calibrateFunc;
	std::function<InstallPaths()>						getVRPathFunc;
	std::function<void(const char*, float)>				vibrateFunc;
	std::function<void(const char*, float)>				LEDFunc;

	std::function<void(float)>							vibrateConstFunc;
	std::function<void(float, float)>					vibrateInSecondsFunc;
	std::function<void(float)>							vibrateOnceFunc;
	std::function<void(float, float)>					vibrateForFunc;

	std::function<void(float)>							LEDConstFunc;
	std::function<void(float, float)>					LEDInSecondsFunc;
	std::function<void(float)>							LEDOnceFunc;
	std::function<void(float, float, float)>			LEDForFunc;

	std::function<double()>								GetLastCalibratedTimeEscapedFunc;

	std::function<void(const char*)>					ForceConnectFunc;

	std::function<void(void(*)(const char*, const char*, int))>			SetDeviceLostCallbackFunc;



public:
	KATSDKWarpper() :_sdkHandle(nullptr)
	{

	}

	~KATSDKWarpper()
	{
		FPlatformProcess::FreeDllHandle(_sdkHandle);
	}

	//KAT Device Count
	int	DeviceCount()
	{
		if (handle() != nullptr)
		{
			if (deviceCountFunc == nullptr)
			{
				using funcType = int(void);
				deviceCountFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("DeviceCount")));
			}

			return deviceCountFunc();
		}

		return -1;
	}

	//KAT Device Description
	KATDeviceDesc GetDevicesDesc(unsigned index)
	{
		if (handle() != nullptr)
		{
			if (getDevicesDescFunc == nullptr)
			{
				using funcType = KATDeviceDesc(unsigned);
				getDevicesDescFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("GetDevicesDesc")));
			}

			return getDevicesDescFunc(index);
		}

		return KATDeviceDesc();
	}

	//Main Function to Get Treadmill output
	KATTreadMillMemoryData GetWalkStatus(const char* sn /* = nullptr*/)
	{
		if (handle() != nullptr)
		{
			if (getWalkStatusFunc == nullptr)
			{
				using funcType = KATTreadMillMemoryData(const char*);
				getWalkStatusFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("GetWalkStatus")));
			}

			return getWalkStatusFunc(sn);
		}

		return KATTreadMillMemoryData();
	}

	//Get Enviorment Install Path
	InstallPaths GetVRPath()
	{
		if (handle() != nullptr)
		{
			if (getVRPathFunc == nullptr)
			{
				using funcType = InstallPaths();
				getVRPathFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("GetVRPath")));
			}
			
			return getVRPathFunc();
		}

		return InstallPaths();
	}

	//Raw Vibrate Interface
	void Vibrate(const char* sn, float amplitude)
	{
		if (handle() != nullptr)
		{
			if (vibrateFunc == nullptr)
			{
				using funcType = void(const char*, float);
				vibrateFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("Vibrate")));
			}

			vibrateFunc(sn, amplitude);
		}
	}

	//Raw LED Interface
	void LED(const char* sn, float amplitude)
	{
		if (handle() != nullptr)
		{
			if (LEDFunc == nullptr)
			{
				using funcType = void(const char*, float);
				LEDFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("LED")));
			}

			LEDFunc(sn, amplitude);
		}
	}

	//KAT PWM Functions Only Support Prime Devices

	//Start Vibrate with a const amplitude,0 for stop
	void VibrateConst(float amplitude)
	{
		if (handle() != nullptr)
		{
			if (vibrateConstFunc == nullptr)
			{
				using funcType = void(float);
				vibrateConstFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("VibrateConst")));
			}

			vibrateConstFunc(amplitude);
		}
	}

	//Start LED with a const amplitude,0 for stop
	void LEDConst(float amplitude)
	{
		if (handle() != nullptr)
		{
			if (LEDConstFunc == nullptr)
			{
				using funcType = void(float);
				LEDConstFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("LEDConst")));
			}

			LEDConstFunc(amplitude);
		}
	}

	//Start Vibrate In a duration
	void VibrateInSeconds(float amplitude, float duration)
	{
		if (handle() != nullptr)
		{
			if (vibrateInSecondsFunc == nullptr)
			{
				using funcType = void(float, float);
				vibrateInSecondsFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("VibrateInSeconds")));
			}

			vibrateInSecondsFunc(amplitude, duration);
		}
	}

	//Start Vibrate Once
	void VibrateOnce(float amplitude)
	{
		if (handle() != nullptr)
		{
			if (vibrateOnceFunc == nullptr)
			{
				using funcType = void(float);
				vibrateOnceFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("VibrateOnce")));
			}

			vibrateOnceFunc(amplitude);
		}
	}

	//Start Vibrate with duration and amplitude
	void VibrateFor(float duration, float amplitude)
	{
		if (handle() != nullptr)
		{
			if (vibrateForFunc == nullptr)
			{
				using funcType = void(float, float);
				vibrateForFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("VibrateFor")));
			}

			vibrateForFunc(duration, amplitude);
		}
	}

	//Start LED In a duration
	void LEDInSeconds(float amplitude, float duration)
	{
		if (handle() != nullptr)
		{
			if (LEDInSecondsFunc == nullptr)
			{
				using funcType = void(float, float);
				LEDInSecondsFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("LEDInSeconds")));
			}

			LEDInSecondsFunc(amplitude, duration);
		}
	}

	//Start LED Once
	void LEDOnce(float amplitude)
	{
		if (handle() != nullptr)
		{
			if (LEDOnceFunc == nullptr)
			{
				using funcType = void(float);
				LEDOnceFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("LEDOnce")));
			}

			LEDOnceFunc(amplitude);
		}
	}

	//Start LED with duration and frequency, amplitude
	void LEDFor(float duration, float frequency, float amplitude)
	{
		if (handle() != nullptr)
		{
			if (LEDForFunc == nullptr)
			{
				using funcType = void(float, float, float);
				LEDForFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("LEDFor")));
			}

			LEDForFunc(duration, frequency, amplitude);
		}
	}

	//Get time in seconds since last calibration
	float GetLastCalibratedTimeEscaped()
	{
		if (handle() != nullptr)
		{
			if (GetLastCalibratedTimeEscapedFunc == nullptr)
			{
				using funcType = double();
				GetLastCalibratedTimeEscapedFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("GetLastCalibratedTimeEscaped")));
			}

			return (float)GetLastCalibratedTimeEscapedFunc();
		}
		return 100;
	}

	//Force connect to device
	void ForceConnect(const char* sn)
	{
		if (handle() != nullptr)
		{
			if (ForceConnectFunc == nullptr)
			{
				using funcType = void(const char*);
				ForceConnectFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("ForceConnect")));
			}

			ForceConnectFunc(sn);
		}
	}

	//Set device lost callback
	void SetDeviceLostCallback(std::function<void(const char*, const char*, int)> callback)
	{
		if (handle() != nullptr)
		{
			static std::function<void(const char*, const char*, int)> g_deviceLostCallback = nullptr;
			g_deviceLostCallback = callback;
			
			if (SetDeviceLostCallbackFunc == nullptr)
			{
				using funcType = void(void(*)(const char*, const char*, int));
				SetDeviceLostCallbackFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("SetDeviceLostCallback")));
			}
			
			SetDeviceLostCallbackFunc([](const char* sn, const char* message, int reason) {
				if (g_deviceLostCallback)
				{
					g_deviceLostCallback(sn, message, reason);
				}
			});
		}
	}

	// KAT Device Calibrate
	void Calibrate(const char* sn = nullptr)
	{
		if (handle() != nullptr)
		{
			if (calibrateFunc == nullptr)
			{
				using funcType = void(const char*);
				calibrateFunc = reinterpret_cast<funcType*>(FPlatformProcess::GetDllExport(handle(), TEXT("Calibrate")));
			}

			if (calibrateFunc != nullptr)
			{
				calibrateFunc(sn);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Cannot Calibrate - function not found in DLL"));
			}
		}
	}
};

#pragma pack(pop)