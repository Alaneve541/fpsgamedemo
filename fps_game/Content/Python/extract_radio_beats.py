"""Extract radio subtitle beats from build_levels.py into a JSON manifest for TTS."""
import json
import re
import sys

SRC = r"C:\Users\alane\Desktop\111\fps_game 5.7\Content\Python\build_levels.py"
OUT = r"C:\Users\alane\Desktop\111\fps_game 5.7\Content\Audio\radio_manifest.json"

pattern = re.compile(
    r"\('(?P<id>CH\d\d_[A-Z0-9_]+)',\s*'subtitle',\s*'(?P<speaker>[^']*)',\s*'(?P<text>[^']*)',\s*(?P<auto>True|False)\),"
)

with open(SRC, encoding="utf-8") as handle:
    source = handle.read()

beats = []
for match in pattern.finditer(source):
    speaker = match.group("speaker")
    if "无线电" not in speaker:
        continue
    beats.append(
        {
            "id": match.group("id"),
            "speaker": speaker,
            "text": match.group("text"),
        }
    )

with open(OUT, "w", encoding="utf-8") as handle:
    json.dump(beats, handle, ensure_ascii=False, indent=2)

print("radio beats:", len(beats))
for beat in beats:
    print(beat["id"], beat["text"])
