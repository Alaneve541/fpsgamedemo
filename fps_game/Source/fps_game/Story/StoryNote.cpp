#include "StoryNote.h"

#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "StorySubsystem.h"

AStoryNote::AStoryNote()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(220.f, 220.f, 160.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBox;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeScale3D(FVector(0.12f, 0.3f, 0.45f));
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -60.f));

	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(RootComponent);
	Glow->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	Glow->SetIntensity(2200.f);
	Glow->SetAttenuationRadius(420.f);
	Glow->SetLightColor(FLinearColor(0.45f, 0.95f, 0.75f));
}

void AStoryNote::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AStoryNote::HandleBeginOverlap);
}

void AStoryNote::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<APawn>(OtherActor))
	{
		return;
	}

	if (!GetWorld())
	{
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastShownTime < ReopenCooldown)
	{
		return;
	}

	LastShownTime = Now;

	if (UStorySubsystem* Story = UStorySubsystem::Get(this))
	{
		Story->ShowNote(NoteTitle, NoteBody, DisplaySeconds);
		Story->ShowInteractionPrompt(TEXT("E   阅读档案"), 3.0f);
	}
}
