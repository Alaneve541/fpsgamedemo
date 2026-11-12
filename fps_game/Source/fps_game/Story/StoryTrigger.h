#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoryTrigger.generated.h"

class UBoxComponent;

/**
 * Invisible volume. Walking into it starts a chapter, plays a single beat, or
 * (optionally) both.
 */
UCLASS()
class FPS_GAME_API AStoryTrigger : public AActor
{
	GENERATED_BODY()

public:
	AStoryTrigger();

	/** Chapter to start when the player enters (leave empty to only play a beat). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FName ChapterId;

	/** Single beat to play when the player enters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FName BeatId;

	/** Only fire once per session. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	bool bTriggerOnce = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
	TObjectPtr<UBoxComponent> TriggerBox;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

private:
	bool bHasFired = false;

	void Fire();
};
