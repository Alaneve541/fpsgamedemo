#include "ZombieWaveSpawner.h"

#include "AIController.h"
#include "Components/StateTreeAIComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Story/StorySubsystem.h"
#include "TimerManager.h"

AZombieWaveSpawner::AZombieWaveSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AZombieWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!ZombieClass)
	{
		ZombieClass = LoadClass<APawn>(nullptr, TEXT("/Game/Code/Zombie/BP_Zombie.BP_Zombie_C"));
	}

	if (!ZombieClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Waves] No zombie class configured and BP_Zombie could not be loaded."));
		return;
	}

	GetWorldTimerManager().SetTimer(WaveTimer, this, &AZombieWaveSpawner::StartEncounter, FMath::Max(0.5f, StartDelay), false);
}

void AZombieWaveSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(WaveTimer);
	Super::EndPlay(EndPlayReason);
}

void AZombieWaveSpawner::StartEncounter()
{
	if (bEncounterActive)
	{
		return;
	}

	bEncounterActive = true;
	CurrentWave = 0;

	if (UStorySubsystem* Story = UStorySubsystem::Get(this))
	{
		if (!StartBeatId.IsNone())
		{
			Story->PlayBeat(StartBeatId);
		}
	}

	StartNextWave();
}

void AZombieWaveSpawner::StartNextWave()
{
	if (!bEncounterActive)
	{
		return;
	}

	bWaitingForNextWave = false;
	++CurrentWave;

	const int32 Count = FMath::Max(0, BaseCount + CountPerWave * (CurrentWave - 1));
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	UE_LOG(LogTemp, Log, TEXT("[Waves] Wave %d/%d spawning %d zombies (player=%s)"),
		CurrentWave, TotalWaves, Count, PlayerPawn ? TEXT("yes") : TEXT("NO"));

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FVector Location = PickSpawnLocation();
		FRotator Rotation = FRotator::ZeroRotator;
		if (PlayerPawn)
		{
			Rotation = (PlayerPawn->GetActorLocation() - Location).Rotation();
		}

		const FTransform SpawnTransform(Rotation, Location);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		APawn* Zombie = GetWorld()->SpawnActor<APawn>(ZombieClass, SpawnTransform, Params);
		if (Zombie)
		{
			Zombie->OnDestroyed.AddDynamic(this, &AZombieWaveSpawner::HandleZombieDestroyed);
			SpawnedZombies.Add(Zombie);
			ApplyZombieTuning(Zombie);
			ScheduleZombieAI(Zombie);
		}
	}

	const float WaveSpeed = ZombieBaseSpeed * (1.f + SpeedBonusPerWave * FMath::Max(0, CurrentWave - 1));
	UE_LOG(LogTemp, Log, TEXT("[Waves] Wave %d zombie walk speed ~%.0f cm/s."), CurrentWave, WaveSpeed);

	ReportWaveState();
	CheckWaveComplete();
}

void AZombieWaveSpawner::ApplyZombieTuning(APawn* Zombie) const
{
	if (!Zombie)
	{
		return;
	}

	UCharacterMovementComponent* Move = Zombie->FindComponentByClass<UCharacterMovementComponent>();
	if (!Move)
	{
		return;
	}

	const float WaveScale = 1.f + SpeedBonusPerWave * FMath::Max(0, CurrentWave - 1);
	const float Jitter = 1.f + FMath::FRandRange(-ZombieSpeedJitter, ZombieSpeedJitter);
	Move->MaxWalkSpeed = ZombieBaseSpeed * WaveScale * Jitter;
}

void AZombieWaveSpawner::ScheduleZombieAI(APawn* Zombie)
{
	if (!Zombie)
	{
		return;
	}

	// A spawned pawn is possessed a moment after SpawnActor; the AI controller's StateTree
	// only starts cleanly once both BeginPlay and OnPossess have run, so wait a beat.
	TWeakObjectPtr<APawn> WeakZombie(Zombie);
	FTimerHandle ScheduleHandle;
	FTimerDelegate Delegate = FTimerDelegate::CreateWeakLambda(this, [this, WeakZombie]()
	{
		TryStartZombieAI(WeakZombie, 0);
	});
	GetWorldTimerManager().SetTimer(ScheduleHandle, Delegate, 0.35f, false);
}

void AZombieWaveSpawner::TryStartZombieAI(TWeakObjectPtr<APawn> WeakZombie, int32 Attempt)
{
	APawn* Zombie = WeakZombie.Get();
	if (!IsValid(Zombie))
	{
		return;
	}

	if (AController* Controller = Zombie->GetController())
	{
		if (UStateTreeAIComponent* StateTree = Controller->FindComponentByClass<UStateTreeAIComponent>())
		{
			if (!StateTree->IsRunning())
			{
				StateTree->RestartLogic();
				UE_LOG(LogTemp, Log, TEXT("[Waves] Started zombie StateTree AI for %s (attempt %d)"), *Zombie->GetName(), Attempt + 1);
			}
			return;
		}
	}

	if (Attempt < 8)
	{
		FTimerHandle RetryHandle;
		FTimerDelegate Delegate = FTimerDelegate::CreateWeakLambda(this, [this, WeakZombie, Attempt]()
		{
			TryStartZombieAI(WeakZombie, Attempt + 1);
		});
		GetWorldTimerManager().SetTimer(RetryHandle, Delegate, 0.35f, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Waves] Zombie %s never received a StateTree AI controller."), *Zombie->GetName());
	}
}

void AZombieWaveSpawner::HandleZombieDestroyed(AActor* DestroyedActor)
{
	SpawnedZombies.RemoveAll([DestroyedActor](const TObjectPtr<APawn>& Entry)
	{
		return Entry == nullptr || Entry.Get() == DestroyedActor;
	});

	ReportWaveState();
	CheckWaveComplete();
}

void AZombieWaveSpawner::CheckWaveComplete()
{
	if (!bEncounterActive || bWaitingForNextWave)
	{
		return;
	}

	if (AliveCount() > 0)
	{
		return;
	}

	UStorySubsystem* Story = UStorySubsystem::Get(this);

	if (CurrentWave < TotalWaves)
	{
		bWaitingForNextWave = true;
		if (Story && !WaveClearedBeatId.IsNone())
		{
			Story->PlayBeat(WaveClearedBeatId);
		}
		GetWorldTimerManager().SetTimer(WaveTimer, this, &AZombieWaveSpawner::StartNextWave, FMath::Max(0.5f, TimeBetweenWaves), false);
	}
	else
	{
		bEncounterActive = false;
		if (Story && !AllClearedBeatId.IsNone())
		{
			Story->PlayBeat(AllClearedBeatId);
		}
	}
}

FVector AZombieWaveSpawner::PickSpawnLocation() const
{
	const FVector Origin = GetActorLocation();
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	for (int32 Attempt = 0; Attempt < 24; ++Attempt)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius = FMath::FRandRange(SpawnRadiusMin, SpawnRadiusMax);
		FVector Candidate = Origin + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);

		if (NavSys)
		{
			FNavLocation Projected;
			if (NavSys->ProjectPointToNavigation(Candidate, Projected, FVector(400.f, 400.f, 1200.f)))
			{
				Candidate = Projected.Location;
				if (FVector::Dist2D(Candidate, Origin) >= SpawnRadiusMin * 0.8f)
				{
					return Candidate + FVector(0.f, 0.f, 100.f);
				}
				continue;
			}
		}

		// Fallback: drop onto whatever is below the random point.
		const FVector Start = Candidate + FVector(0.f, 0.f, 1200.f);
		const FVector End = Candidate - FVector(0.f, 0.f, 3000.f);
		FHitResult Hit;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ZombieSpawner), false, this);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, QueryParams))
		{
			return Hit.Location + FVector(0.f, 0.f, 100.f);
		}
	}

	const float FallbackAngle = FMath::FRandRange(0.f, 2.f * PI);
	return Origin + FVector(FMath::Cos(FallbackAngle) * SpawnRadiusMax, FMath::Sin(FallbackAngle) * SpawnRadiusMax, 100.f);
}

int32 AZombieWaveSpawner::AliveCount() const
{
	int32 Alive = 0;
	for (const TObjectPtr<APawn>& Zombie : SpawnedZombies)
	{
		if (IsValid(Zombie))
		{
			++Alive;
		}
	}
	return Alive;
}

void AZombieWaveSpawner::ReportWaveState() const
{
	if (UStorySubsystem* Story = UStorySubsystem::Get(this))
	{
		Story->NotifyWaveState(CurrentWave, TotalWaves, AliveCount());
	}
}
