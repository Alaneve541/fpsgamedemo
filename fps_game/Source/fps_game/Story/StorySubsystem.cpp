#include "StorySubsystem.h"

#include "StoryHUD.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Stats/Stats.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "../Weapon/FpsWeaponComponent.h"

UStorySubsystem* UStorySubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UStorySubsystem>() : nullptr;
}

void UStorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Hud = MakeShared<FStoryHudState>();
}

void UStorySubsystem::Deinitialize()
{
	if (Overlay.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(Overlay.ToSharedRef());
	}
	Overlay.Reset();
	PendingBeats.Reset();
	Super::Deinitialize();
}

void UStorySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	BuildWidget();
}

TStatId UStorySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UStorySubsystem, STATGROUP_Tickables);
}

void UStorySubsystem::EnsureDatabase()
{
	if (bDatabaseTried)
	{
		return;
	}
	bDatabaseTried = true;

	Database = LoadObject<UStoryDatabase>(nullptr, TEXT("/Game/Story/DA_Campaign.DA_Campaign"));
	if (!Database)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Story] Campaign database /Game/Story/DA_Campaign was not found."));
	}
}

void UStorySubsystem::BuildWidget()
{
	if (Overlay.IsValid() || !Hud.IsValid())
	{
		return;
	}
	if (!GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}
	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	Overlay = SNew(SStoryOverlay).State(Hud);
	GEngine->GameViewport->AddViewportWidgetContent(Overlay.ToSharedRef(), 50);
}

void UStorySubsystem::StartChapter(FName ChapterId)
{
	EnsureDatabase();
	if (!Database)
	{
		return;
	}

	const FStoryChapter* Chapter = Database->FindChapter(ChapterId);
	if (!Chapter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Story] Chapter '%s' not found in the campaign database."), *ChapterId.ToString());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Story] StartChapter %s '%s' with %d beats."),
		*ChapterId.ToString(), *Chapter->Title, Chapter->Beats.Num());

	Hud->ChapterTitle = Chapter->Title;
	Hud->ChapterSubtitle = Chapter->Subtitle;
	Hud->ChapterTimer = 6.f;
	Hud->WaveInfo.Empty();
	Hud->Ending.Empty();
	Hud->EndingTimer = 0.f;
	SetObjective(Chapter->Objective);

	PendingBeats.Reset();
	for (const FStoryBeat& Beat : Chapter->Beats)
	{
		if (Beat.bAutoPlay)
		{
			QueueBeat(Beat);
		}
	}
}

void UStorySubsystem::QueueBeat(const FStoryBeat& Beat)
{
	UE_LOG(LogTemp, Log, TEXT("[Story] Beat %s (%s) speaker=%s text=%s"),
		*Beat.BeatId.ToString(), *UEnum::GetValueAsString(Beat.Kind), *Beat.Speaker, *Beat.Text);

	switch (Beat.Kind)
	{
	case EStoryBeatKind::Objective:
		SetObjective(Beat.Objective.IsEmpty() ? Beat.Text : Beat.Objective);
		break;
	case EStoryBeatKind::Ending:
		ShowEnding(Beat.Text);
		break;
	case EStoryBeatKind::Subtitle:
	default:
		PendingBeats.Add(Beat);
		break;
	}
}

void UStorySubsystem::PlayBeat(FName BeatId)
{
	EnsureDatabase();
	if (!Database)
	{
		return;
	}

	if (const FStoryBeat* Beat = Database->FindBeat(BeatId))
	{
		if (Beat->Kind == EStoryBeatKind::Subtitle && !Hud->Subtitle.IsEmpty())
		{
			QueueBeat(*Beat);
		}
		else
		{
			QueueBeat(*Beat);
			if (Beat->Kind == EStoryBeatKind::Subtitle && PendingBeats.Num() == 1 && Hud->SubtitleTimer <= 0.f)
			{
				// QueueBeat already appended; let Tick pick it up on the next frame.
			}
		}
		if (!Beat->Objective.IsEmpty())
		{
			SetObjective(Beat->Objective);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Story] Beat '%s' not found in the campaign database."), *BeatId.ToString());
	}
}

void UStorySubsystem::PlayCustomSubtitle(const FString& SpeakerName, const FString& Text, float Seconds)
{
	FStoryBeat Beat;
	Beat.Speaker = SpeakerName;
	Beat.Text = Text;
	Beat.Duration = Seconds;
	PushSubtitle(Beat, Seconds);
}

void UStorySubsystem::PushSubtitle(const FStoryBeat& Beat, float OverrideSeconds)
{
	Hud->Speaker = Beat.Speaker;
	Hud->Subtitle = Beat.Text;

	// Radio chatter: play the recorded voice line when the subtitle appears.
	if (!Beat.Voice.IsNull())
	{
		if (USoundBase* VoiceSound = Beat.Voice.LoadSynchronous())
		{
			UGameplayStatics::PlaySound2D(this, VoiceSound, 1.f);
			UE_LOG(LogTemp, Log, TEXT("[Story] Voice for %s playing '%s'."),
				*Beat.BeatId.ToString(), *VoiceSound->GetName());
		}
	}

	float Seconds = OverrideSeconds > 0.f ? OverrideSeconds : Beat.Duration;
	if (Seconds <= 0.f)
	{
		Seconds = FMath::Clamp(Beat.Text.Len() / 7.f + 2.0f, 3.0f, 10.0f);
	}
	Hud->SubtitleTimer = FMath::Max(1.5f, Seconds);
}

void UStorySubsystem::ShowNote(const FString& Title, const FString& Body, float Seconds)
{
	Hud->NoteTitle = Title;
	Hud->NoteBody = Body;
	Hud->NoteTotal = FMath::Max(3.f, Seconds);
	Hud->NoteTimer = Hud->NoteTotal;
}

void UStorySubsystem::SetObjective(const FString& NewObjective)
{
	Hud->Objective = NewObjective;
}

void UStorySubsystem::ShowEnding(const FString& Text)
{
	Hud->Ending = Text;
	Hud->EndingTimer = 16.f;
	Hud->WaveInfo.Empty();
	OnEndingPresented.Broadcast();
}

void UStorySubsystem::ShowInteractionPrompt(const FString& Text, float Seconds)
{
	Hud->Interaction = Text;
	Hud->InteractionTimer = FMath::Max(0.5f, Seconds);
}

void UStorySubsystem::NotifyWaveState(int32 CurrentWave, int32 TotalWaves, int32 AliveCount)
{
	if (AliveCount > 0)
	{
		Hud->WaveInfo = FString::Printf(TEXT("遭遇战  第 %d / %d 波      剩余 %d"), CurrentWave, TotalWaves, AliveCount);
	}
	else if (CurrentWave >= TotalWaves)
	{
		Hud->WaveInfo = FString::Printf(TEXT("遭遇战  已清除 %d / %d 波"), CurrentWave, TotalWaves);
	}
	else
	{
		Hud->WaveInfo = FString::Printf(TEXT("遭遇战  第 %d / %d 波      下一波来袭..."), CurrentWave, TotalWaves);
	}
}

void UStorySubsystem::SetWeaponState(const FString& InName, int32 InAmmo, int32 InReserve, bool bReloading)
{
	if (!Hud.IsValid())
	{
		return;
	}
	Hud->WeaponName = InName;
	Hud->WeaponAmmo = InAmmo;
	Hud->WeaponReserve = InReserve;
	Hud->bWeaponReloading = bReloading;
}

void UStorySubsystem::StripBlueprintWeapon(APawn* Pawn)
{
	UWorld* World = GetWorld();
	if (!World || !Pawn)
	{
		return;
	}

	TArray<AActor*> ToRemove;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor == Pawn)
		{
			continue;
		}
		const FString ClassName = Actor->GetClass()->GetName();
		if (!ClassName.Contains(TEXT("BP_VIRTUS")) && !ClassName.Contains(TEXT("BP_WeaponBase")))
		{
			continue;
		}
		const bool bAttachedToPawn = Actor->GetAttachParentActor() == Pawn || Actor->GetOwner() == Pawn;
		const bool bNearPawn = FVector::DistSquared(Actor->GetActorLocation(), Pawn->GetActorLocation()) < (250.f * 250.f);
		if (bAttachedToPawn || bNearPawn)
		{
			ToRemove.Add(Actor);
		}
	}

	for (AActor* Actor : ToRemove)
	{
		// Keep the blueprint weapon alive (BP_FPSPlayer.StopFire/Reload read CurrentWeapon every
		// time the player presses a key) but make it inert: hidden, no collision, no tick, no ammo.
		Actor->SetActorHiddenInGame(true);
		Actor->SetActorEnableCollision(false);
		Actor->SetActorTickEnabled(false);

		TArray<UPrimitiveComponent*> Primitives;
		Actor->GetComponents<UPrimitiveComponent>(Primitives);
		for (UPrimitiveComponent* Primitive : Primitives)
		{
			Primitive->SetVisibility(false);
		}

		for (const TCHAR* AmmoProp : { TEXT("CurrentAmmo"), TEXT("TotalAmmo") })
		{
			if (FIntProperty* Prop = FindFProperty<FIntProperty>(Actor->GetClass(), AmmoProp))
			{
				Prop->SetPropertyValue_InContainer(Actor, 0);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[Weapon] Neutralized blueprint weapon %s (hidden, ammo zeroed)."), *Actor->GetName());
	}
}

void UStorySubsystem::EnsureWeaponComponent()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!Pawn)
	{
		return;
	}

	if (UFpsWeaponComponent* Existing = Pawn->FindComponentByClass<UFpsWeaponComponent>())
	{
		WeaponComponent = Existing;
		return;
	}

	// The blueprint weapon (XL_FPSPack MCX-VIRTUS with proper hands/animation) is the only
	// visual that currently looks right, so keep it and let the blueprint own firing. The
	// runtime weapon experiment stays disabled until it can replace this cleanly.
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor == Pawn)
		{
			continue;
		}
		const FString ClassName = Actor->GetClass()->GetName();
		if ((ClassName.Contains(TEXT("BP_VIRTUS")) || ClassName.Contains(TEXT("BP_WeaponBase")))
			&& (Actor->GetAttachParentActor() == Pawn || Actor->GetOwner() == Pawn))
		{
			UE_LOG(LogTemp, Verbose, TEXT("[Weapon] Keeping original blueprint weapon %s."), *Actor->GetName());
			return;
		}
	}

	// No blueprint weapon found yet (player may still be initialising); check again next tick.
	// The runtime weapon component is disabled for now because the blueprint MCX-VIRTUS
	// (with proper hands and animations) is the only weapon visual that looks right.
	UE_LOG(LogTemp, Verbose, TEXT("[Weapon] No blueprint weapon attached yet; waiting."));
}

void UStorySubsystem::Tick(float DeltaTime)
{
	WeaponCheckTimer -= DeltaTime;
	if (WeaponCheckTimer <= 0.f)
	{
		WeaponCheckTimer = 2.f;
		EnsureWeaponComponent();
	}

	if (!Hud.IsValid())
	{
		return;
	}

	if (Hud->ChapterTimer > 0.f)
	{
		Hud->ChapterTimer = FMath::Max(0.f, Hud->ChapterTimer - DeltaTime);
	}

	if (Hud->InteractionTimer > 0.f)
	{
		Hud->InteractionTimer = FMath::Max(0.f, Hud->InteractionTimer - DeltaTime);
		if (Hud->InteractionTimer <= 0.f)
		{
			Hud->Interaction.Empty();
		}
	}

	if (Hud->NoteTimer > 0.f)
	{
		Hud->NoteTimer = FMath::Max(0.f, Hud->NoteTimer - DeltaTime);
		if (Hud->NoteTimer <= 0.f)
		{
			Hud->NoteTitle.Empty();
			Hud->NoteBody.Empty();
		}
	}

	if (Hud->EndingTimer > 0.f)
	{
		Hud->EndingTimer = FMath::Max(0.f, Hud->EndingTimer - DeltaTime);
		if (Hud->EndingTimer <= 0.f)
		{
			Hud->Ending.Empty();
		}
	}

	if (Hud->SubtitleTimer > 0.f)
	{
		Hud->SubtitleTimer -= DeltaTime;
		if (Hud->SubtitleTimer <= 0.f)
		{
			Hud->SubtitleTimer = 0.f;
			Hud->Speaker.Empty();
			Hud->Subtitle.Empty();
		}
	}
	else if (PendingBeats.Num() > 0)
	{
		FStoryBeat Next = PendingBeats[0];
		PendingBeats.RemoveAt(0);
		PushSubtitle(Next);
	}
}
