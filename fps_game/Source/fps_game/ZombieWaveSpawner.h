#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "ZombieWaveSpawner.generated.h"

class APawn;

/**
 * Spawns escalating waves of zombies around itself. A wave is cleared when every
 * spawned pawn is destroyed (the stock zombie blueprint destroys itself on death),
 * then the level's story beats are fired and the next wave begins.
 */
UCLASS()
class FPS_GAME_API AZombieWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	AZombieWaveSpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	TSubclassOf<APawn> ZombieClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	int32 TotalWaves = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	int32 BaseCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	int32 CountPerWave = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	float SpawnRadiusMin = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	float SpawnRadiusMax = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	float TimeBetweenWaves = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	float StartDelay = 5.f;

	/** Beat played when the encounter starts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FName StartBeatId;

	/** Beat played whenever a wave is cleared (before the last one). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FName WaveClearedBeatId;

	/** Beat played when the final wave is cleared. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FName AllClearedBeatId;

	/** Base walk speed applied to every spawned zombie (the BP itself is left untouched). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves|Tuning")
	float ZombieBaseSpeed = 290.f;

	/** Per-zombie random speed variation, e.g. 0.1 = +/-10%. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves|Tuning")
	float ZombieSpeedJitter = 0.10f;

	/** Extra walk speed fraction per wave after the first (late waves feel more urgent). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves|Tuning")
	float SpeedBonusPerWave = 0.07f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void StartEncounter();
	void StartNextWave();
	void CheckWaveComplete();
	FVector PickSpawnLocation() const;
	int32 AliveCount() const;
	void ReportWaveState() const;

	/** State trees on spawned AIs need to be restarted once the controller possesses the pawn. */
	void ScheduleZombieAI(APawn* Zombie);
	void TryStartZombieAI(TWeakObjectPtr<APawn> WeakZombie, int32 Attempt);

	/** Applies the wave-scaled walk speed to a freshly spawned zombie. */
	void ApplyZombieTuning(APawn* Zombie) const;

	UFUNCTION()
	void HandleZombieDestroyed(AActor* DestroyedActor);

	UPROPERTY()
	TArray<TObjectPtr<APawn>> SpawnedZombies;

	FTimerHandle WaveTimer;
	int32 CurrentWave = 0;
	bool bEncounterActive = false;
	bool bWaitingForNextWave = false;
};
