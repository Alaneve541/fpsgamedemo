#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelAmbience.generated.h"

class UAudioComponent;
class USoundBase;

/**
 * Loops one ambient sound for the whole level (wind, machinery hum, dripping pipes).
 * Placed by Content/Python/build_levels.py per chapter.
 */
UCLASS()
class FPS_GAME_API ALevelAmbience : public AActor
{
	GENERATED_BODY()

public:
	ALevelAmbience();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
	USoundBase* AmbientSound = nullptr;

	/** Linear volume multiplier, 0..1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
	float Volume = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
	float Pitch = 1.0f;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Ambience")
	TObjectPtr<UAudioComponent> Audio;
};
