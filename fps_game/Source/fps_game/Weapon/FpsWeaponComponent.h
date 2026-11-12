#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FpsWeaponComponent.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class USceneComponent;
class USoundBase;
class UMaterialInterface;
class UPointLightComponent;
class USpotLightComponent;

UENUM()
enum class EFpsFireMode : uint8
{
	Semi,
	Auto
};

/** Pure data definition of one slot in the player's loadout. */
struct FFpsWeaponDef
{
	FString Name;
	FString Short;
	EFpsFireMode FireMode = EFpsFireMode::Auto;
	int32 MagSize = 30;
	int32 Reserve = 120;
	float Damage = 24.f;
	float RPM = 620.f;
	int32 Pellets = 1;
	float SpreadDeg = 0.9f;
	float ReloadTime = 2.0f;
	float RecoilPitch = 0.9f;
	float RecoilYaw = 0.3f;
	float AimFov = 62.f;
	float Range = 12000.f;
	FString FireSound;
	/** Optional imported static mesh (baked so the barrel points along +X, units in cm). */
	FString VisualMeshPath;
	FVector MuzzleLocal = FVector(100.f, 0.f, -9.f);
};

/**
 * Self contained weapon inventory used by the campaign levels.
 * It replaces the blueprint weapon at runtime (the BP weapon is stripped by UStorySubsystem),
 * owns hitscan firing, recoil, reloads, weapon switching and a toggleable flashlight.
 */
UCLASS(ClassGroup = (FPS), meta = (BlueprintSpawnableComponent))
class FPS_GAME_API UFpsWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFpsWeaponComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void ToggleFlashlight();

private:
	void BuildLoadout();
	void BindInput();
	void SelectWeapon(int32 Index);
	void CycleWeapon(int32 Direction);

	void StartFire();
	void StopFire();
	void FireShot();
	void Reload();
	void FinishReload();
	void SetAiming(bool bNewAiming);

	// Thin input wrappers.
	void SelectWeapon0() { SelectWeapon(0); }
	void SelectWeapon1() { SelectWeapon(1); }
	void SelectWeapon2() { SelectWeapon(2); }
	void SelectWeapon3() { SelectWeapon(3); }
	void CycleWeaponUp() { CycleWeapon(1); }
	void CycleWeaponDown() { CycleWeapon(-1); }
	void StartAim() { SetAiming(true); }
	void StopAim() { SetAiming(false); }
	void StartReload() { Reload(); }

	void SyncHud();
	UCameraComponent* GetCamera() const;

	USceneComponent* BuildWeaponVisual(int32 Index, USceneComponent* AttachParent);
	UStaticMeshComponent* AddPart(USceneComponent* Parent, const TCHAR* MeshPath, const FVector& Location,
		const FRotator& Rotation, const FVector& SizeCm, UMaterialInterface* Material);

	UPROPERTY()
	TObjectPtr<USceneComponent> LoadoutRoot = nullptr;

	UPROPERTY()
	TObjectPtr<USpotLightComponent> Flashlight = nullptr;

	UPROPERTY()
	TObjectPtr<USoundBase> FireSound = nullptr;

	UPROPERTY()
	TObjectPtr<USoundBase> ImpactSound = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> MaterialMetal = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> MaterialRust = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> MaterialWood = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> MaterialGlow = nullptr;

	TArray<FFpsWeaponDef> Weapons;
	TArray<TObjectPtr<USceneComponent>> WeaponRoots;
	TArray<TObjectPtr<UPointLightComponent>> MuzzleLights;
	TArray<TObjectPtr<USoundBase>> FireSounds;
	TArray<int32> Mags;
	TArray<int32> Reserves;

	int32 CurrentWeapon = 0;
	bool bReloading = false;
	bool bFiring = false;
	bool bAiming = false;
	float BaseFov = 90.f;

	FTimerHandle FireTimer;
	FTimerHandle ReloadTimer;
	FTimerHandle MuzzleTimer;
	FTimerHandle FlashlightBlink;

	int32 MuzzleLightIndex = 0;
};
