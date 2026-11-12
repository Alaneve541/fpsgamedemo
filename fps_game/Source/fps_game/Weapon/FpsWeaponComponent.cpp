#include "FpsWeaponComponent.h"

#include "Camera/CameraComponent.h"
#include "InputCoreTypes.h"
#include "Components/InputComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "../Story/StorySubsystem.h"
#include "TimerManager.h"

namespace
{
	const TCHAR* CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* CylinderPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
	const TCHAR* SpherePath = TEXT("/Engine/BasicShapes/Sphere.Sphere");

	const TCHAR* MetalPath = TEXT("/Game/Levels/Materials/MI_MetalPlate.MI_MetalPlate");
	const TCHAR* RustPath = TEXT("/Game/Levels/Materials/MI_RustyMetal.MI_RustyMetal");
	const TCHAR* WoodPath = TEXT("/Game/Levels/Materials/MI_WoodPlanks.MI_WoodPlanks");
	const TCHAR* GlowPath = TEXT("/Game/Levels/Materials/M_Lamp.M_Lamp");

	const TCHAR* RifleSound = TEXT("/Game/ShotSystem/Shot_SFX/SFX_Vefects_Shots_Weapon_Rifle_Cue.SFX_Vefects_Shots_Weapon_Rifle_Cue");
	const TCHAR* SmgSound = TEXT("/Game/ShotSystem/Shot_SFX/SFX_Vefects_Shots_Weapon_SMG_Cue.SFX_Vefects_Shots_Weapon_SMG_Cue");
	const TCHAR* ShotgunSound = TEXT("/Game/ShotSystem/Shot_SFX/SFX_Vefects_Shots_Weapon_Shotgun_Cue.SFX_Vefects_Shots_Weapon_Shotgun_Cue");
	const TCHAR* HandgunSound = TEXT("/Game/ShotSystem/Shot_SFX/SFX_Vefects_Shots_Weapon_Hand_Gun_Cue.SFX_Vefects_Shots_Weapon_Hand_Gun_Cue");
	const TCHAR* ImpactCue = TEXT("/Game/XL_FPSPack/Common/Sound/Bullet_Impact_Sounds/Concrete/SW_Concrete_Bullet_Hit_1.SW_Concrete_Bullet_Hit_1");

	// First-person viewmodel framing: real-world scale is too large when the mesh origin
	// sits this close to the camera, so shrink and push it toward the lower-right of the view.
	constexpr float ViewModelScale = 0.5f;
	const FVector ViewModelOffset(36.f, 6.f, -13.f);
}

UFpsWeaponComponent::UFpsWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFpsWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	BuildLoadout();
	BindInput();
	SelectWeapon(0);
}

void UFpsWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimer);
		World->GetTimerManager().ClearTimer(ReloadTimer);
		World->GetTimerManager().ClearTimer(MuzzleTimer);
		World->GetTimerManager().ClearTimer(FlashlightBlink);
	}
	Super::EndPlay(EndPlayReason);
}

UCameraComponent* UFpsWeaponComponent::GetCamera() const
{
	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		return Pawn->FindComponentByClass<UCameraComponent>();
	}
	return nullptr;
}

UStaticMeshComponent* UFpsWeaponComponent::AddPart(USceneComponent* Parent, const TCHAR* MeshPath,
	const FVector& Location, const FRotator& Rotation, const FVector& SizeCm, UMaterialInterface* Material)
{
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
	if (!Mesh)
	{
		return nullptr;
	}

	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	if (!Part)
	{
		return nullptr;
	}

	Part->SetMobility(EComponentMobility::Movable);
	Part->SetStaticMesh(Mesh);
	if (Material)
	{
		Part->SetMaterial(0, Material);
	}
	Part->SetRelativeLocation(Location);
	Part->SetRelativeRotation(Rotation);
	Part->SetRelativeScale3D(SizeCm / 100.f);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetGenerateOverlapEvents(false);
	Part->SetCastShadow(false);
	Part->RegisterComponent();
	Part->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
	return Part;
}

void UFpsWeaponComponent::BuildLoadout()
{
	UMaterialInterface* Metal = LoadObject<UMaterialInterface>(nullptr, MetalPath);
	UMaterialInterface* Rust = LoadObject<UMaterialInterface>(nullptr, RustPath);
	UMaterialInterface* Wood = LoadObject<UMaterialInterface>(nullptr, WoodPath);
	MaterialMetal = Metal ? Metal : Rust;
	MaterialRust = Rust ? Rust : Metal;
	MaterialWood = Wood ? Wood : Metal;
	MaterialGlow = LoadObject<UMaterialInterface>(nullptr, GlowPath);
	ImpactSound = LoadObject<USoundBase>(nullptr, ImpactCue);

	FFpsWeaponDef Rifle;
	Rifle.Name = TEXT("VIRTUS 突击步枪");
	Rifle.Short = TEXT("VIRTUS");
	Rifle.FireMode = EFpsFireMode::Auto;
	Rifle.MagSize = 30;
	Rifle.Reserve = 150;
	Rifle.Damage = 24.f;
	Rifle.RPM = 640.f;
	Rifle.SpreadDeg = 0.9f;
	Rifle.ReloadTime = 2.1f;
	Rifle.RecoilPitch = 0.85f;
	Rifle.RecoilYaw = 0.35f;
	Rifle.AimFov = 60.f;
	Rifle.FireSound = RifleSound;
	Rifle.VisualMeshPath = TEXT("/Game/Models/Guns/SM_gun_virtus.SM_gun_virtus");
	Rifle.MuzzleLocal = FVector(92.f, 0.f, 2.f);
	Weapons.Add(Rifle);

	FFpsWeaponDef Smg;
	Smg.Name = TEXT("K-9 冲锋枪");
	Smg.Short = TEXT("K-9");
	Smg.FireMode = EFpsFireMode::Auto;
	Smg.MagSize = 36;
	Smg.Reserve = 180;
	Smg.Damage = 15.f;
	Smg.RPM = 950.f;
	Smg.SpreadDeg = 1.8f;
	Smg.ReloadTime = 1.6f;
	Smg.RecoilPitch = 0.5f;
	Smg.RecoilYaw = 0.45f;
	Smg.AimFov = 64.f;
	Smg.FireSound = SmgSound;
	Smg.VisualMeshPath = TEXT("/Game/Models/Guns/SM_gun_k9.SM_gun_k9");
	Smg.MuzzleLocal = FVector(79.f, 0.f, 2.f);
	Weapons.Add(Smg);

	FFpsWeaponDef Shotgun;
	Shotgun.Name = TEXT("M-90 霰弹枪");
	Shotgun.Short = TEXT("M-90");
	Shotgun.FireMode = EFpsFireMode::Semi;
	Shotgun.MagSize = 6;
	Shotgun.Reserve = 42;
	Shotgun.Damage = 11.f;
	Shotgun.Pellets = 9;
	Shotgun.RPM = 75.f;
	Shotgun.SpreadDeg = 4.5f;
	Shotgun.ReloadTime = 3.0f;
	Shotgun.RecoilPitch = 3.0f;
	Shotgun.RecoilYaw = 1.0f;
	Shotgun.AimFov = 68.f;
	Shotgun.FireSound = ShotgunSound;
	Shotgun.VisualMeshPath = TEXT("/Game/Models/Guns/SM_gun_m90.SM_gun_m90");
	Shotgun.MuzzleLocal = FVector(101.f, 0.f, 2.f);
	Weapons.Add(Shotgun);

	FFpsWeaponDef Dmr;
	Dmr.Name = TEXT("DMR-7 精确步枪");
	Dmr.Short = TEXT("DMR-7");
	Dmr.FireMode = EFpsFireMode::Semi;
	Dmr.MagSize = 10;
	Dmr.Reserve = 60;
	Dmr.Damage = 58.f;
	Dmr.RPM = 180.f;
	Dmr.SpreadDeg = 0.22f;
	Dmr.ReloadTime = 2.6f;
	Dmr.RecoilPitch = 2.2f;
	Dmr.RecoilYaw = 0.4f;
	Dmr.AimFov = 45.f;
	Dmr.FireSound = RifleSound;
	Dmr.VisualMeshPath = TEXT("/Game/Models/Guns/SM_gun_dmr.SM_gun_dmr");
	Dmr.MuzzleLocal = FVector(119.f, 0.f, 3.f);
	Weapons.Add(Dmr);

	Mags.SetNum(Weapons.Num());
	Reserves.SetNum(Weapons.Num());
	for (int32 i = 0; i < Weapons.Num(); ++i)
	{
		Mags[i] = Weapons[i].MagSize;
		Reserves[i] = Weapons[i].Reserve;
		FireSounds.Add(LoadObject<USoundBase>(nullptr, *Weapons[i].FireSound));
	}

	if (UCameraComponent* Camera = GetCamera())
	{
		LoadoutRoot = NewObject<USceneComponent>(this);
		LoadoutRoot->SetMobility(EComponentMobility::Movable);
		LoadoutRoot->SetRelativeLocation(FVector(0.f, 8.f, 0.f));
		LoadoutRoot->RegisterComponent();
		LoadoutRoot->AttachToComponent(Camera, FAttachmentTransformRules::KeepRelativeTransform);

		const float CamScale = FMath::Max(Camera->GetComponentScale().X, 0.01f);
		for (int32 i = 0; i < Weapons.Num(); ++i)
		{
			USceneComponent* Root = BuildWeaponVisual(i, LoadoutRoot);
			WeaponRoots.Add(Root);

			UPointLightComponent* Muzzle = NewObject<UPointLightComponent>(this);
			Muzzle->SetMobility(EComponentMobility::Movable);
			Muzzle->SetIntensity(0.f);
			Muzzle->SetAttenuationRadius(320.f);
			Muzzle->SetLightColor(FLinearColor(1.f, 0.85f, 0.6f));
			Muzzle->SetCastShadows(false);
			Muzzle->RegisterComponent();
			Muzzle->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
			Muzzle->SetRelativeLocation((ViewModelOffset + Weapons[i].MuzzleLocal * ViewModelScale) / CamScale);
			MuzzleLights.Add(Muzzle);
		}

		Flashlight = NewObject<USpotLightComponent>(this);
		Flashlight->SetMobility(EComponentMobility::Movable);
		Flashlight->SetIntensity(14000.f);
		Flashlight->SetAttenuationRadius(3200.f);
		Flashlight->SetInnerConeAngle(16.f);
		Flashlight->SetOuterConeAngle(30.f);
		Flashlight->SetLightColor(FLinearColor(1.f, 0.97f, 0.9f));
		Flashlight->SetVisibility(false);
		Flashlight->RegisterComponent();
		Flashlight->AttachToComponent(Camera, FAttachmentTransformRules::KeepRelativeTransform);
	}

	UE_LOG(LogTemp, Log, TEXT("[Weapon] Loadout built with %d weapons (flashlight ready)."), Weapons.Num());
}

USceneComponent* UFpsWeaponComponent::BuildWeaponVisual(int32 Index, USceneComponent* AttachParent)
{
	USceneComponent* Root = NewObject<USceneComponent>(this);
	Root->SetMobility(EComponentMobility::Movable);
	Root->SetRelativeLocation(FVector::ZeroVector);
	Root->RegisterComponent();
	Root->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);

	// Prefer a real imported weapon mesh when one is assigned for this slot.
	if (Weapons.IsValidIndex(Index) && !Weapons[Index].VisualMeshPath.IsEmpty())
	{
		if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Weapons[Index].VisualMeshPath))
		{
			UStaticMeshComponent* Body = NewObject<UStaticMeshComponent>(this);
			Body->SetMobility(EComponentMobility::Movable);
			Body->SetStaticMesh(Mesh);
			const int32 NumSlots = Mesh->GetStaticMaterials().Num();
			for (int32 Slot = 0; Slot < NumSlots; ++Slot)
			{
				UMaterialInterface* Mat = MaterialMetal.Get();
				if (Slot % 2 == 1)
				{
					Mat = (Index == 1 || Index == 3) ? MaterialRust.Get() : MaterialWood.Get();
				}
				if (!Mat)
				{
					Mat = MaterialMetal.Get();
				}
				Body->SetMaterial(Slot, Mat);
			}
			// The camera inherits the pawn's scale; compensate so the viewmodel keeps a
			// consistent world size regardless of how the player blueprint is scaled.
			float ParentScale = AttachParent->GetComponentScale().X;
			if (ParentScale <= 0.01f)
			{
				ParentScale = 1.f;
			}
			Body->SetRelativeLocation(ViewModelOffset / ParentScale);
			Body->SetRelativeScale3D(FVector(ViewModelScale / ParentScale));
			Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Body->SetGenerateOverlapEvents(false);
			Body->SetCastShadow(false);
			Body->RegisterComponent();
			Body->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
			UE_LOG(LogTemp, Log, TEXT("[Weapon] Slot %d using static mesh %s (slots=%d, viewmodel scale=%.2f)."),
				Index, *Weapons[Index].VisualMeshPath, NumSlots, Body->GetComponentScale().X);
			return Root;
		}
	}

	UMaterialInterface* Metal = MaterialMetal;
	UMaterialInterface* Dark = MaterialRust;
	UMaterialInterface* Wood = MaterialWood;

	auto Part = [&](const TCHAR* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Size, UMaterialInterface* Mat)
	{
		AddPart(Root, Mesh, Loc, Rot, Size, Mat);
	};

	switch (Index)
	{
	case 0: // VIRTUS style carbine
		Part(CubePath, FVector(62.f, 0.f, -13.f), FRotator::ZeroRotator, FVector(36.f, 7.f, 9.f), Metal);
		Part(CylinderPath, FVector(96.f, 0.f, -13.f), FRotator(90.f, 0.f, 0.f), FVector(2.6f, 2.6f, 26.f), Dark);
		Part(CubePath, FVector(78.f, 0.f, -13.f), FRotator::ZeroRotator, FVector(20.f, 6.f, 6.f), Dark);
		Part(CubePath, FVector(44.f, 0.f, -13.f), FRotator::ZeroRotator, FVector(14.f, 5.f, 7.f), Metal);
		Part(CubePath, FVector(62.f, 0.f, -22.f), FRotator(-12.f, 0.f, 0.f), FVector(7.f, 5.f, 14.f), Dark);
		Part(CubePath, FVector(52.f, 0.f, -21.f), FRotator(-28.f, 0.f, 0.f), FVector(5.f, 5.f, 9.f), Dark);
		Part(CubePath, FVector(60.f, 0.f, -7.f), FRotator::ZeroRotator, FVector(5.f, 2.f, 2.f), Metal);
		Part(CubePath, FVector(86.f, 0.f, -7.f), FRotator::ZeroRotator, FVector(3.f, 1.5f, 3.f), Metal);
		Part(CylinderPath, FVector(110.f, 0.f, -13.f), FRotator(90.f, 0.f, 0.f), FVector(3.2f, 3.2f, 5.f), Dark);
		break;
	case 1: // compact SMG
		Part(CubePath, FVector(58.f, 0.f, -11.f), FRotator::ZeroRotator, FVector(26.f, 6.5f, 8.f), Metal);
		Part(CylinderPath, FVector(76.f, 0.f, -11.f), FRotator(90.f, 0.f, 0.f), FVector(2.2f, 2.2f, 12.f), Dark);
		Part(CubePath, FVector(70.f, 0.f, -11.f), FRotator::ZeroRotator, FVector(10.f, 5.f, 5.f), Dark);
		Part(CubePath, FVector(56.f, 0.f, -21.f), FRotator(-8.f, 0.f, 0.f), FVector(5.f, 4.f, 18.f), Dark);
		Part(CubePath, FVector(48.f, 0.f, -19.f), FRotator(-30.f, 0.f, 0.f), FVector(5.f, 5.f, 9.f), Metal);
		Part(CubePath, FVector(41.f, 0.f, -11.f), FRotator::ZeroRotator, FVector(10.f, 3.f, 4.f), Metal);
		Part(CubePath, FVector(58.f, 0.f, -6.f), FRotator::ZeroRotator, FVector(4.f, 2.f, 2.f), Metal);
		break;
	case 2: // pump shotgun
		Part(CubePath, FVector(60.f, 0.f, -12.f), FRotator::ZeroRotator, FVector(26.f, 7.f, 9.f), Metal);
		Part(CylinderPath, FVector(94.f, 0.f, -12.f), FRotator(90.f, 0.f, 0.f), FVector(3.4f, 3.4f, 36.f), Dark);
		Part(CylinderPath, FVector(90.f, 0.f, -18.f), FRotator(90.f, 0.f, 0.f), FVector(2.4f, 2.4f, 30.f), Metal);
		Part(CubePath, FVector(86.f, 0.f, -17.f), FRotator::ZeroRotator, FVector(10.f, 6.f, 6.f), Wood);
		Part(CubePath, FVector(40.f, 0.f, -12.f), FRotator::ZeroRotator, FVector(16.f, 5.f, 8.f), Wood);
		Part(CubePath, FVector(52.f, 0.f, -20.f), FRotator(-25.f, 0.f, 0.f), FVector(5.f, 5.f, 8.f), Wood);
		break;
	case 3: // marksman rifle
	default:
		Part(CubePath, FVector(68.f, 0.f, -13.f), FRotator::ZeroRotator, FVector(40.f, 7.f, 9.f), Metal);
		Part(CylinderPath, FVector(108.f, 0.f, -13.f), FRotator(90.f, 0.f, 0.f), FVector(2.2f, 2.2f, 34.f), Dark);
		Part(CubePath, FVector(86.f, 0.f, -13.f), FRotator::ZeroRotator, FVector(24.f, 6.f, 6.f), Dark);
		Part(CubePath, FVector(42.f, 0.f, -13.f), FRotator::ZeroRotator, FVector(16.f, 5.f, 8.f), Metal);
		Part(CubePath, FVector(68.f, 0.f, -21.f), FRotator(-8.f, 0.f, 0.f), FVector(6.f, 5.f, 12.f), Dark);
		Part(CylinderPath, FVector(72.f, 0.f, -2.f), FRotator(90.f, 0.f, 0.f), FVector(5.f, 5.f, 20.f), Dark);
		Part(CubePath, FVector(62.f, 0.f, -5.f), FRotator::ZeroRotator, FVector(3.f, 2.f, 6.f), Metal);
		Part(CubePath, FVector(82.f, 0.f, -5.f), FRotator::ZeroRotator, FVector(3.f, 2.f, 6.f), Metal);
		Part(CylinderPath, FVector(128.f, 0.f, -13.f), FRotator(90.f, 0.f, 0.f), FVector(3.0f, 3.0f, 5.f), Dark);
		break;
	}

	return Root;
}

void UFpsWeaponComponent::BindInput()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !PC->InputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Weapon] No player controller input component found; weapon input unbound."));
		return;
	}

	UInputComponent* Input = PC->InputComponent;
	Input->BindKey(EKeys::One, IE_Pressed, this, &UFpsWeaponComponent::SelectWeapon0);
	Input->BindKey(EKeys::Two, IE_Pressed, this, &UFpsWeaponComponent::SelectWeapon1);
	Input->BindKey(EKeys::Three, IE_Pressed, this, &UFpsWeaponComponent::SelectWeapon2);
	Input->BindKey(EKeys::Four, IE_Pressed, this, &UFpsWeaponComponent::SelectWeapon3);
	Input->BindKey(EKeys::Q, IE_Pressed, this, &UFpsWeaponComponent::CycleWeaponUp);
	Input->BindKey(EKeys::E, IE_Pressed, this, &UFpsWeaponComponent::CycleWeaponDown);
	Input->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &UFpsWeaponComponent::CycleWeaponUp);
	Input->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &UFpsWeaponComponent::CycleWeaponDown);

	Input->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &UFpsWeaponComponent::StartFire);
	Input->BindKey(EKeys::LeftMouseButton, IE_Released, this, &UFpsWeaponComponent::StopFire);
	Input->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &UFpsWeaponComponent::StartAim);
	Input->BindKey(EKeys::RightMouseButton, IE_Released, this, &UFpsWeaponComponent::StopAim);
	Input->BindKey(EKeys::R, IE_Pressed, this, &UFpsWeaponComponent::StartReload);
	Input->BindKey(EKeys::F, IE_Pressed, this, &UFpsWeaponComponent::ToggleFlashlight);
}

void UFpsWeaponComponent::SelectWeapon(int32 Index)
{
	if (!Weapons.IsValidIndex(Index) || !WeaponRoots.IsValidIndex(Index))
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimer);
		World->GetTimerManager().ClearTimer(ReloadTimer);
	}
	bReloading = false;
	bFiring = false;
	CurrentWeapon = Index;

	for (int32 i = 0; i < WeaponRoots.Num(); ++i)
	{
		if (WeaponRoots[i])
		{
			WeaponRoots[i]->SetVisibility(i == CurrentWeapon, true);
		}
	}

	if (LoadoutRoot)
	{
		LoadoutRoot->SetRelativeLocation(bAiming ? FVector(0.f, 3.f, 0.f) : FVector(0.f, 8.f, 0.f));
	}

	UE_LOG(LogTemp, Log, TEXT("[Weapon] Equipped %s (%d/%d)."),
		*Weapons[CurrentWeapon].Name, Mags[CurrentWeapon], Reserves[CurrentWeapon]);
	SyncHud();
}

void UFpsWeaponComponent::CycleWeapon(int32 Direction)
{
	if (Weapons.Num() == 0)
	{
		return;
	}
	const int32 Next = (CurrentWeapon + Direction + Weapons.Num()) % Weapons.Num();
	SelectWeapon(Next);
}

void UFpsWeaponComponent::StartFire()
{
	if (bReloading || !Weapons.IsValidIndex(CurrentWeapon))
	{
		return;
	}
	bFiring = true;
	FireShot();

	if (Weapons[CurrentWeapon].FireMode == EFpsFireMode::Auto && Weapons[CurrentWeapon].RPM > 1.f)
	{
		if (UWorld* World = GetWorld())
		{
			const float Interval = 60.f / Weapons[CurrentWeapon].RPM;
			World->GetTimerManager().SetTimer(FireTimer, this, &UFpsWeaponComponent::FireShot, Interval, true);
		}
	}
}

void UFpsWeaponComponent::StopFire()
{
	bFiring = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimer);
	}
}

void UFpsWeaponComponent::FireShot()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	UCameraComponent* Camera = GetCamera();
	UWorld* World = GetWorld();
	if (!Pawn || !Camera || !World || !Weapons.IsValidIndex(CurrentWeapon))
	{
		return;
	}
	if (bReloading)
	{
		return;
	}
	if (Mags[CurrentWeapon] <= 0)
	{
		Reload();
		return;
	}

	--Mags[CurrentWeapon];
	const FFpsWeaponDef& Def = Weapons[CurrentWeapon];

	const FVector Start = Camera->GetComponentLocation();
	const FVector BaseDir = Camera->GetForwardVector();
	const float ConeRad = FMath::DegreesToRadians(Def.SpreadDeg);
	FCollisionQueryParams Params(TEXT("FpsWeaponShot"), true, Pawn);

	for (int32 Pellet = 0; Pellet < FMath::Max(1, Def.Pellets); ++Pellet)
	{
		const FVector Dir = Def.SpreadDeg > 0.f ? FMath::VRandCone(BaseDir, ConeRad) : BaseDir;
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Start, Start + Dir * Def.Range, ECC_Visibility, Params))
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				float Damage = Def.Damage;
				if (Hit.BoneName.ToString().Contains(TEXT("head"), ESearchCase::IgnoreCase))
				{
					Damage *= 2.2f;
				}
				UGameplayStatics::ApplyPointDamage(HitActor, Damage, Dir, Hit, Pawn->GetController(), Pawn, nullptr);
			}
			if (ImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Hit.ImpactPoint, 0.55f, FMath::FRandRange(0.92f, 1.08f));
			}
		}
	}

	if (FireSounds.IsValidIndex(CurrentWeapon) && FireSounds[CurrentWeapon])
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSounds[CurrentWeapon], Camera->GetComponentLocation() + BaseDir * 80.f, 0.9f);
	}

	if (MuzzleLights.IsValidIndex(CurrentWeapon) && MuzzleLights[CurrentWeapon])
	{
		MuzzleLights[CurrentWeapon]->SetIntensity(70000.f);
		MuzzleLightIndex = CurrentWeapon;
		World->GetTimerManager().SetTimer(MuzzleTimer, [this]()
		{
			if (MuzzleLights.IsValidIndex(MuzzleLightIndex) && MuzzleLights[MuzzleLightIndex])
			{
				MuzzleLights[MuzzleLightIndex]->SetIntensity(0.f);
			}
		}, 0.045f, false);
	}

	if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
	{
		PC->AddPitchInput(-Def.RecoilPitch);
		PC->AddYawInput(FMath::FRandRange(-Def.RecoilYaw, Def.RecoilYaw));
	}

	SyncHud();
}

void UFpsWeaponComponent::Reload()
{
	if (bReloading || !Weapons.IsValidIndex(CurrentWeapon))
	{
		return;
	}
	if (Mags[CurrentWeapon] >= Weapons[CurrentWeapon].MagSize || Reserves[CurrentWeapon] <= 0)
	{
		return;
	}
	StopFire();
	bReloading = true;
	SyncHud();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ReloadTimer, this, &UFpsWeaponComponent::FinishReload,
			Weapons[CurrentWeapon].ReloadTime, false);
	}
	UE_LOG(LogTemp, Log, TEXT("[Weapon] Reloading %s..."), *Weapons[CurrentWeapon].Short);
}

void UFpsWeaponComponent::FinishReload()
{
	if (!Weapons.IsValidIndex(CurrentWeapon))
	{
		return;
	}
	const int32 Need = Weapons[CurrentWeapon].MagSize - Mags[CurrentWeapon];
	const int32 Take = FMath::Min(Need, Reserves[CurrentWeapon]);
	Mags[CurrentWeapon] += Take;
	Reserves[CurrentWeapon] -= Take;
	bReloading = false;
	SyncHud();
	UE_LOG(LogTemp, Log, TEXT("[Weapon] Reloaded %s (%d/%d)."),
		*Weapons[CurrentWeapon].Short, Mags[CurrentWeapon], Reserves[CurrentWeapon]);
}

void UFpsWeaponComponent::SetAiming(bool bNewAiming)
{
	if (bAiming == bNewAiming)
	{
		return;
	}
	bAiming = bNewAiming;
	if (UCameraComponent* Camera = GetCamera())
	{
		if (bAiming)
		{
			BaseFov = Camera->FieldOfView;
		}
		Camera->SetFieldOfView(bAiming ? Weapons[CurrentWeapon].AimFov : BaseFov);
	}
	if (LoadoutRoot)
	{
		LoadoutRoot->SetRelativeLocation(bAiming ? FVector(0.f, 3.f, 0.f) : FVector(0.f, 8.f, 0.f));
	}
}

void UFpsWeaponComponent::ToggleFlashlight()
{
	if (!Flashlight)
	{
		return;
	}
	const bool bVisible = !Flashlight->IsVisible();
	Flashlight->SetVisibility(bVisible);
	UE_LOG(LogTemp, Log, TEXT("[Weapon] Flashlight %s."), bVisible ? TEXT("on") : TEXT("off"));
}

void UFpsWeaponComponent::SyncHud()
{
	if (!Weapons.IsValidIndex(CurrentWeapon))
	{
		return;
	}
	if (UStorySubsystem* Story = UStorySubsystem::Get(this))
	{
		Story->SetWeaponState(Weapons[CurrentWeapon].Name, Mags[CurrentWeapon],
			Reserves[CurrentWeapon], bReloading);
	}
}
