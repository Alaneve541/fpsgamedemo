#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoryDirector.generated.h"

/**
 * Single per-level manager. Starts the level's chapter after BeginPlay and can
 * hand the player to the next map once the finale beat has been shown.
 */
UCLASS()
class FPS_GAME_API AStoryDirector : public AActor
{
	GENERATED_BODY()

public:
	AStoryDirector();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FName ChapterId;

	/** Seconds to wait after BeginPlay before the chapter card appears. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	float StartDelay = 1.5f;

	/** If set, the ending card is shown and the level progress is written when this beat fires. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FName FinaleBeatId;

	/** Map to travel to (e.g. /Game/Maps/Ch02). Empty = stay in this level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FString NextMap = "/Game/Maps/Ch02";

	/** Seconds between the finale card and the travel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	float TravelDelay = 8.0f;

protected:
	virtual void BeginPlay() override;

private:
	void StartChapterNow();
	void FadeFromBlack(float Duration);
	void FadeToBlack(float Duration);

	UFUNCTION()
	void OnFinaleSeen();
	FTimerHandle StartTimer;
	FTimerHandle TravelTimer;
	FTimerHandle FadeTimer;
};
