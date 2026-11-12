#include "StoryDirector.h"

#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "StorySubsystem.h"
#include "TimerManager.h"

AStoryDirector::AStoryDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStoryDirector::BeginPlay()
{
	Super::BeginPlay();

	// A short fade-in sells the chapter transition without a full cutscene rig.
	FadeFromBlack(2.4f);


	if (StartDelay <= 0.f)
	{
		StartChapterNow();
	}
	else
	{
		GetWorldTimerManager().SetTimer(StartTimer, this, &AStoryDirector::StartChapterNow, StartDelay, false);
	}

	if (!FinaleBeatId.IsNone())
	{
		UStorySubsystem* Story = UStorySubsystem::Get(this);
		if (Story)
		{
			Story->OnEndingPresented.AddDynamic(this, &AStoryDirector::OnFinaleSeen);
		}
	}
}

void AStoryDirector::FadeFromBlack(float Duration)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, Duration, FLinearColor::Black, false, false);
		}
	}
}

void AStoryDirector::FadeToBlack(float Duration)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, Duration, FLinearColor::Black, false, false);
		}
	}
}

void AStoryDirector::StartChapterNow()
{
	if (!ChapterId.IsNone())
	{
		if (UStorySubsystem* Story = UStorySubsystem::Get(this))
		{
			Story->StartChapter(ChapterId);
		}
	}
}

void AStoryDirector::OnFinaleSeen()
{
	if (NextMap.IsEmpty())
	{
		return;
	}

	// Fade to black just before travelling, then hand the player to the next map.
	const float FadeLead = 1.1f;
	if (TravelDelay > FadeLead + 0.1f)
	{
		GetWorldTimerManager().SetTimer(FadeTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			FadeToBlack(FadeLead);
		}), TravelDelay - FadeLead, false);
	}
	else
	{
		FadeToBlack(FMath::Max(0.3f, TravelDelay));
	}

	GetWorldTimerManager().SetTimer(TravelTimer, [this]()
	{
		UGameplayStatics::OpenLevel(this, FName(*NextMap));
	}, TravelDelay, false);
}
