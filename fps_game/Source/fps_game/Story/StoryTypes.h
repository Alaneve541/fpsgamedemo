#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StoryTypes.generated.h"

class USoundBase;

/** A single narrative beat fired by a chapter start, a trigger volume, a wave spawner or a note. */
UENUM(BlueprintType)
enum class EStoryBeatKind : uint8
{
	Subtitle	UMETA(DisplayName = "Subtitle"),
	Objective	UMETA(DisplayName = "Objective"),
	Ending		UMETA(DisplayName = "Ending")
};

USTRUCT(BlueprintType)
struct FStoryBeat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FName BeatId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	EStoryBeatKind Kind = EStoryBeatKind::Subtitle;

	/** Optional speaker name, e.g. "无线电 · 林默" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FString Speaker;

	/** The line itself (or the new objective text when Kind == Objective). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story", meta = (MultiLine = true))
	FString Text;

	/** Optional objective set by this beat, regardless of Kind. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FString Objective;

	/** 0 means auto (text length based). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	float Duration = 0.0f;

	/** Play automatically when its chapter starts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	bool bAutoPlay = false;

	/** Optional voice-over (radio chatter lines) played when this beat is shown. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	TSoftObjectPtr<USoundBase> Voice;
};

USTRUCT(BlueprintType)
struct FStoryChapter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FName ChapterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FString Subtitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	FString Objective;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
	TArray<FStoryBeat> Beats;
};

/**
 * Campaign database: chapters + beats. Created as /Game/Story/DA_Campaign and consumed by UStorySubsystem.
 */
UCLASS(BlueprintType)
class FPS_GAME_API UStoryDatabase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	TArray<FStoryChapter> Chapters;

	const FStoryChapter* FindChapter(FName ChapterId) const
	{
		return Chapters.FindByPredicate([ChapterId](const FStoryChapter& Chapter)
		{
			return Chapter.ChapterId == ChapterId;
		});
	}

	const FStoryBeat* FindBeat(FName BeatId) const
	{
		for (const FStoryChapter& Chapter : Chapters)
		{
			for (const FStoryBeat& Beat : Chapter.Beats)
			{
				if (Beat.BeatId == BeatId)
				{
					return &Beat;
				}
			}
		}
		return nullptr;
	}
};
