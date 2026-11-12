#include "LevelAmbience.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

ALevelAmbience::ALevelAmbience()
{
	PrimaryActorTick.bCanEverTick = false;

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudio"));
	RootComponent = Audio;
	Audio->bAutoActivate = false;
	Audio->bAllowSpatialization = false;
}

void ALevelAmbience::BeginPlay()
{
	Super::BeginPlay();

	if (!Audio || !AmbientSound)
	{
		return;
	}

	Audio->SetSound(AmbientSound);
	Audio->SetVolumeMultiplier(FMath::Clamp(Volume, 0.f, 1.f));
	Audio->SetPitchMultiplier(FMath::Max(0.1f, Pitch));
	Audio->Play();

	UE_LOG(LogTemp, Log, TEXT("[Audio] Ambience '%s' playing at volume %.2f."),
		*AmbientSound->GetName(), Volume);
}
