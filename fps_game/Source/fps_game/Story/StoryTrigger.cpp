#include "StoryTrigger.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "StorySubsystem.h"

AStoryTrigger::AStoryTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(400.f, 400.f, 200.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBox;
}

void AStoryTrigger::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AStoryTrigger::HandleBeginOverlap);
}

void AStoryTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasFired && bTriggerOnce)
	{
		return;
	}

	if (!Cast<APawn>(OtherActor))
	{
		return;
	}

	Fire();
}

void AStoryTrigger::Fire()
{
	bHasFired = true;

	UStorySubsystem* Story = UStorySubsystem::Get(this);
	if (!Story)
	{
		return;
	}

	if (!ChapterId.IsNone())
	{
		Story->StartChapter(ChapterId);
	}

	if (!BeatId.IsNone())
	{
		Story->PlayBeat(BeatId);
	}
}
