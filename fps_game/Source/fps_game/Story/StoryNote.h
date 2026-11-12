#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoryNote.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UPointLightComponent;

/**
 * A readable note/terminal lying in the level. Overlapping it pops the archive
 * panel; a cooldown prevents the same note from re-opening constantly.
 */
UCLASS()
class FPS_GAME_API AStoryNote : public AActor
{
	GENERATED_BODY()

public:
	AStoryNote();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FString NoteTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FString NoteBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	float DisplaySeconds = 14.f;

	/** Seconds before the same note can re-open automatically. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	float ReopenCooldown = 12.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
	TObjectPtr<UPointLightComponent> Glow;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

private:
	double LastShownTime = -1000.0;
};
