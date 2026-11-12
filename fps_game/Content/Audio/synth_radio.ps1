$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Speech

$project = 'C:\Users\alane\Desktop\111\fps_game 5.7'
$manifestPath = Join-Path $project 'Content\Audio\radio_manifest.json'
$rawDir = Join-Path $project 'Content\Audio\raw'
$beats = Get-Content -LiteralPath $manifestPath -Raw -Encoding UTF8 | ConvertFrom-Json

$synth = New-Object System.Speech.Synthesis.SpeechSynthesizer
$synth.SelectVoice('Microsoft Huihui Desktop')
$synth.Rate = -1

foreach ($beat in $beats) {
    $out = Join-Path $rawDir ($beat.id + '.wav')
    if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Force }

    $ssml = '<speak version="1.0" xmlns="http://www.w3.org/2001/10/synthesis" xml:lang="zh-CN"><prosody rate="-6%" pitch="-4%">' + $beat.text + '</prosody></speak>'
    $synth.SetOutputToWaveFile($out)
    $synth.SpeakSsml($ssml)
    $synth.SetOutputToNull()
    Write-Output ($beat.id + ' -> ' + $out)
}
$synth.Dispose()
Write-Output 'TTS done.'
