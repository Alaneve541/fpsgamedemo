#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "StoryTypes.h"
#include "StorySubsystem.generated.h"

class SStoryOverlay;
class UFpsWeaponComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStoryEndingPresented);

/** Plain (non-UObject) snapshot consumed by the Slate overlay. Game thread only. */
struct FStoryHudState
{
	FString ChapterTitle;
	FString ChapterSubtitle;
	float ChapterTimer = 0.f;

	FString Objective;

	FString Speaker;
	FString Subtitle;
	float SubtitleTimer = 0.f;

	FString NoteTitle;
	FString NoteBody;
	float NoteTimer = 0.f;
	float NoteTotal = 1.f;

	FString WaveInfo;

	FString Ending;
	float EndingTimer = 0.f;

	FString Interaction;
	float InteractionTimer = 0.f;

	FString WeaponName;
	int32 WeaponAmmo = 0;
	int32 WeaponReserve = 0;
	bool bWeaponReloading = false;
};

/**
 * Per-world narrative director: owns the campaign database, the subtitle queue and the Slate HUD overlay.
 * Created automatically for every game world - no GameMode / GameInstance configuration needed.
 */
UCLASS()
class FPS_GAME_API UStorySubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UStorySubsystem* Get(const UObject* WorldContextObject);

	// USubsystem / UWorldSubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Story")
	void StartChapter(FName ChapterId);

	UFUNCTION(BlueprintCallable, Category = "Story")
	void PlayBeat(FName BeatId);

	UFUNCTION(BlueprintCallable, Category = "Story")
	void PlayCustomSubtitle(const FString& SpeakerName, const FString& Text, float Seconds = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Story")
	void ShowNote(const FString& Title, const FString& Body, float Seconds = 14.f);

	UFUNCTION(BlueprintCallable, Category = "Story")
	void SetObjective(const FString& NewObjective);

	UFUNCTION(BlueprintCallable, Category = "Story")
	void ShowEnding(const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "Story")
	void ShowInteractionPrompt(const FString& Text, float Seconds = 2.5f);

	UFUNCTION(BlueprintCallable, Category = "Story")
	void NotifyWaveState(int32 CurrentWave, int32 TotalWaves, int32 AliveCount);

	/** Updated by the runtime weapon component so the HUD can show ammo. */
	void SetWeaponState(const FString& InName, int32 InAmmo, int32 InReserve, bool bReloading);

	const FStoryHudState& GetHudState() const { return *Hud; }

	/** Fires whenever an ending card is displayed. */
	UPROPERTY(BlueprintAssignable, Category = "Story")
	FOnStoryEndingPresented OnEndingPresented;

private:
	void EnsureDatabase();
	void BuildWidget();
	void PushSubtitle(const FStoryBeat& Beat, float OverrideSeconds = 0.f);
	void QueueBeat(const FStoryBeat& Beat);

	void EnsureWeaponComponent();
	void StripBlueprintWeapon(APawn* Pawn);

	UPROPERTY()
	TObjectPtr<UStoryDatabase> Database = nullptr;

	UPROPERTY()
	TObjectPtr<UFpsWeaponComponent> WeaponComponent = nullptr;

	float WeaponCheckTimer = 1.5f;

	TSharedPtr<FStoryHudState> Hud;
	TArray<FStoryBeat> PendingBeats;
	TSharedPtr<SStoryOverlay> Overlay;
	bool bDatabaseTried = false;
};
