#include "StoryHUD.h"

#include "Brushes/SlateColorBrush.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	FLinearColor Dim(float Alpha)
	{
		return FLinearColor(0.0f, 0.0f, 0.0f, Alpha);
	}
}

TAttribute<EVisibility> SStoryOverlay::VisLambda(TFunction<bool()> Predicate) const
{
	return TAttribute<EVisibility>::CreateLambda([Predicate]()
	{
		return Predicate() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
	});
}

TAttribute<FText> SStoryOverlay::TextLambda(TFunction<FString()> Getter) const
{
	return TAttribute<FText>::CreateLambda([Getter]()
	{
		return FText::FromString(Getter());
	});
}

void SStoryOverlay::Construct(const FArguments& InArgs)
{
	State = InArgs._State;

	SetVisibility(EVisibility::SelfHitTestInvisible);

	static FSlateColorBrush WhiteBrush(FLinearColor::White);
	static FSlateColorBrush SubtleBrush(FLinearColor(0.02f, 0.03f, 0.04f, 1.0f));

	const FSlateFontInfo SmallFont = FCoreStyle::GetDefaultFontStyle("Regular", 16);
	const FSlateFontInfo BodyFont = FCoreStyle::GetDefaultFontStyle("Regular", 26);
	const FSlateFontInfo BodyBoldFont = FCoreStyle::GetDefaultFontStyle("Bold", 26);
	const FSlateFontInfo SpeakerFont = FCoreStyle::GetDefaultFontStyle("Bold", 20);
	const FSlateFontInfo ChapterFont = FCoreStyle::GetDefaultFontStyle("Bold", 54);
	const FSlateFontInfo ChapterSubFont = FCoreStyle::GetDefaultFontStyle("Regular", 22);
	const FSlateFontInfo PanelTitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 28);
	const FSlateFontInfo EndingFont = FCoreStyle::GetDefaultFontStyle("Bold", 44);

	auto IsLive = [this]()
	{
		return State.IsValid();
	};

	ChildSlot
	[
		SNew(SOverlay)

		// ---------------------------------------------------------------- objective (top left)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(FMargin(48.f, 44.f, 0.f, 0.f))
		[
			SNew(SVerticalBox)
			.Visibility(VisLambda([this, IsLive]()
			{
				return IsLive() && !State->Objective.IsEmpty();
			}))

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Font(SmallFont)
				.ColorAndOpacity(FLinearColor(0.65f, 0.72f, 0.66f))
				.Text(FText::FromString(TEXT("当 前 目 标")))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 6.f, 0.f, 0.f))
			[
				SNew(SBorder)
				.BorderImage(&WhiteBrush)
				.BorderBackgroundColor(Dim(0.38f))
				.Padding(FMargin(14.f, 8.f))
				[
					SNew(SBox)
					.MaxDesiredWidth(560.f)
					[
						SNew(STextBlock)
						.Font(BodyBoldFont)
						.AutoWrapText(true)
						.ColorAndOpacity(FLinearColor(1.0f, 0.9f, 0.66f))
						.ShadowOffset(FVector2D(1.f, 1.f))
						.ShadowColorAndOpacity(Dim(0.8f))
						.Text(TextLambda([this, IsLive]()
						{
							return IsLive() ? State->Objective : FString();
						}))
					]
				]
			]
		]

		// ---------------------------------------------------------------- wave state (top right)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.f, 44.f, 48.f, 0.f))
		[
			SNew(STextBlock)
			.Font(SmallFont)
			.ColorAndOpacity(FLinearColor(0.86f, 0.4f, 0.34f))
			.Visibility(VisLambda([this, IsLive]()
			{
				return IsLive() && !State->WaveInfo.IsEmpty();
			}))
			.Text(TextLambda([this, IsLive]()
			{
				return IsLive() ? State->WaveInfo : FString();
			}))
		]

		// ---------------------------------------------------------------- chapter title card (center)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.f, -140.f, 0.f, 0.f))
		[
			SNew(SVerticalBox)
			.Visibility(VisLambda([this, IsLive]()
			{
				return IsLive() && !State->ChapterTitle.IsEmpty();
			}))

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(ChapterFont)
				.ColorAndOpacity(FLinearColor(0.94f, 0.9f, 0.82f))
				.ShadowOffset(FVector2D(3.f, 3.f))
				.ShadowColorAndOpacity(Dim(0.9f))
				.Text(TextLambda([this, IsLive]()
				{
					return IsLive() ? State->ChapterTitle : FString();
				}))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(FMargin(0.f, 14.f, 0.f, 0.f))
			[
				SNew(STextBlock)
				.Font(ChapterSubFont)
				.ColorAndOpacity(FLinearColor(0.7f, 0.74f, 0.7f))
				.Text(TextLambda([this, IsLive]()
				{
					return IsLive() ? State->ChapterSubtitle : FString();
				}))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(FMargin(0.f, 18.f, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(360.f)
				.HeightOverride(2.f)
				[
					SNew(SImage)
					.Image(&WhiteBrush)
					.ColorAndOpacity(FLinearColor(0.8f, 0.62f, 0.3f, 0.75f))
				]
			]
		]

		// ---------------------------------------------------------------- subtitle (bottom center)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.f, 0.f, 0.f, 104.f))
		[
			SNew(SBox)
			.MaxDesiredWidth(1100.f)
			.Visibility(VisLambda([this, IsLive]()
			{
				return IsLive() && !State->Subtitle.IsEmpty();
			}))
			[
				SNew(SBorder)
				.BorderImage(&WhiteBrush)
				.BorderBackgroundColor(Dim(0.5f))
				.Padding(FMargin(26.f, 14.f))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Font(SpeakerFont)
						.ColorAndOpacity(FLinearColor(0.95f, 0.72f, 0.32f))
						.Visibility(VisLambda([this, IsLive]()
						{
							return IsLive() && !State->Speaker.IsEmpty();
						}))
						.Text(TextLambda([this, IsLive]()
						{
							return IsLive() ? State->Speaker : FString();
						}))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(FMargin(0.f, 4.f, 0.f, 0.f))
					[
						SNew(STextBlock)
						.Font(BodyFont)
						.AutoWrapText(true)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FLinearColor(0.95f, 0.95f, 0.93f))
						.ShadowOffset(FVector2D(1.f, 1.f))
						.ShadowColorAndOpacity(Dim(0.85f))
						.Text(TextLambda([this, IsLive]()
						{
							return IsLive() ? State->Subtitle : FString();
						}))
					]
				]
			]
		]

		// ---------------------------------------------------------------- interaction prompt
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.f, 0.f, 0.f, 210.f))
		[
			SNew(SBorder)
			.BorderImage(&WhiteBrush)
			.BorderBackgroundColor(Dim(0.42f))
			.Padding(FMargin(16.f, 6.f))
			.Visibility(VisLambda([this, IsLive]()
			{
				return IsLive() && !State->Interaction.IsEmpty();
			}))
			[
				SNew(STextBlock)
				.Font(SpeakerFont)
				.ColorAndOpacity(FLinearColor(0.9f, 0.92f, 0.88f))
				.Text(TextLambda([this, IsLive]()
				{
					return IsLive() ? State->Interaction : FString();
				}))
			]
		]

		// ---------------------------------------------------------------- readable note (center)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(820.f)
			.Visibility(VisLambda([this, IsLive]()
			{
				return IsLive() && !State->NoteTitle.IsEmpty();
			}))
			[
				SNew(SBorder)
				.BorderImage(&SubtleBrush)
				.BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.03f, 0.95f))
				.Padding(FMargin(34.f, 26.f))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Font(SmallFont)
						.ColorAndOpacity(FLinearColor(0.55f, 0.62f, 0.56f))
						.Text(FText::FromString(TEXT("档 案 记 录")))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.f, 8.f, 0.f, 0.f))
					[
						SNew(STextBlock)
						.Font(PanelTitleFont)
						.ColorAndOpacity(FLinearColor(0.95f, 0.82f, 0.55f))
						.Text(TextLambda([this, IsLive]()
						{
							return IsLive() ? State->NoteTitle : FString();
						}))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.f, 14.f, 0.f, 0.f))
					[
						SNew(STextBlock)
						.Font(BodyFont)
						.AutoWrapText(true)
						.ColorAndOpacity(FLinearColor(0.86f, 0.86f, 0.83f))
						.Text(TextLambda([this, IsLive]()
						{
							return IsLive() ? State->NoteBody : FString();
						}))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.f, 20.f, 0.f, 0.f))
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.Font(SmallFont)
						.ColorAndOpacity(FLinearColor(0.5f, 0.55f, 0.5f))
						.Text(FText::FromString(TEXT("（稍后自动收起）")))
					]
				]
			]
		]

		// ---------------------------------------------------------------- ending card
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			.Visibility(VisLambda([this, IsLive]()
			{
				return IsLive() && !State->Ending.IsEmpty();
			}))

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(EndingFont)
				.ColorAndOpacity(FLinearColor(0.95f, 0.86f, 0.62f))
				.ShadowOffset(FVector2D(3.f, 3.f))
				.ShadowColorAndOpacity(Dim(0.9f))
				.Text(TextLambda([this, IsLive]()
				{
					return IsLive() ? State->Ending : FString();
				}))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(FMargin(0.f, 20.f, 0.f, 0.f))
			[
				SNew(STextBlock)
				.Font(ChapterSubFont)
				.ColorAndOpacity(FLinearColor(0.66f, 0.68f, 0.66f))
				.Text(FText::FromString(TEXT("— 感谢游玩 —")))
			]
		]

		// ---------------------------------------------------------------- weapon panel (bottom right)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.f, 0.f, 42.f, 40.f))
		[
			SNew(SBorder)
			.BorderImage(&WhiteBrush)
			.BorderBackgroundColor(Dim(0.45f))
			.Padding(FMargin(18.f, 10.f, 22.f, 12.f))
			.Visibility(VisLambda([this, IsLive]()
			{
				return IsLive() && !State->WeaponName.IsEmpty();
			}))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Font(SmallFont)
					.ColorAndOpacity(FLinearColor(0.55f, 0.60f, 0.55f))
					.Text(FText::FromString(TEXT("武 器")))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.Padding(FMargin(0.f, 2.f, 0.f, 2.f))
				[
					SNew(STextBlock)
					.Font(BodyBoldFont)
					.ColorAndOpacity(FLinearColor(0.95f, 0.86f, 0.62f))
					.Text(TextLambda([this, IsLive]()
					{
						return IsLive() ? State->WeaponName : FString();
					}))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Font(BodyBoldFont)
					.ColorAndOpacity(FLinearColor(0.92f, 0.92f, 0.90f))
					.Text(TextLambda([this, IsLive]()
					{
						return IsLive() ? FString::Printf(TEXT("%d / %d"), State->WeaponAmmo, State->WeaponReserve) : FString();
					}))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.Padding(FMargin(0.f, 2.f, 0.f, 0.f))
				[
					SNew(STextBlock)
					.Font(SmallFont)
					.ColorAndOpacity(FLinearColor(0.90f, 0.50f, 0.35f))
					.Text(FText::FromString(TEXT("换弹中…")))
					.Visibility(VisLambda([this, IsLive]()
					{
						return IsLive() && State->bWeaponReloading;
					}))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.Padding(FMargin(0.f, 8.f, 0.f, 0.f))
				[
					SNew(STextBlock)
					.Font(SmallFont)
					.ColorAndOpacity(FLinearColor(0.48f, 0.52f, 0.48f))
					.Text(FText::FromString(TEXT("1-4 切换武器 · 滚轮/Q E · R 换弹 · RMB 瞄准 · F 手电")))
				]
			]
		]

	];
}
