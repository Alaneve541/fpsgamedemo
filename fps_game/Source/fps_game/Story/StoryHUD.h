#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "StorySubsystem.h"

/**
 * Full-screen narrative overlay: chapter cards, objectives, subtitles, readable notes,
 * wave state and the ending card. Purely cosmetic - never takes input.
 */
class SStoryOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStoryOverlay) {}
		SLATE_ARGUMENT(TSharedPtr<FStoryHudState>, State)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TAttribute<EVisibility> VisLambda(TFunction<bool()> Predicate) const;
	TAttribute<FText> TextLambda(TFunction<FString()> Getter) const;

	TSharedPtr<FStoryHudState> State;
};
