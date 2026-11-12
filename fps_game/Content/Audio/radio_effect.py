"""Apply a cheap radio-comm effect to the SAPI wavs: band-limit, add hiss, soft clip."""
import json
import os
import wave

import numpy as np

PROJECT = r"C:\Users\alane\Desktop\111\fps_game 5.7"
RAW = os.path.join(PROJECT, "Content", "Audio", "raw")
OUT = os.path.join(PROJECT, "Content", "Audio", "Voices")
MANIFEST = os.path.join(PROJECT, "Content", "Audio", "radio_manifest.json")

rng = np.random.default_rng(7)


def read_wav(path):
    with wave.open(path, "rb") as handle:
        channels = handle.getnchannels()
        width = handle.getsampwidth()
        rate = handle.getframerate()
        frames = handle.readframes(handle.getnframes())
    if width != 2:
        raise SystemExit("expected 16-bit wav: " + path)
    data = np.frombuffer(frames, dtype="<i2").astype(np.float64) / 32768.0
    if channels > 1:
        data = data.reshape(-1, channels).mean(axis=1)
    return data, rate


def write_wav(path, data, rate):
    peak = float(np.max(np.abs(data))) or 1.0
    data = data / peak * 0.92
    pcm = np.clip(data * 32767.0, -32768, 32767).astype("<i2")
    with wave.open(path, "wb") as handle:
        handle.setnchannels(1)
        handle.setsampwidth(2)
        handle.setframerate(rate)
        handle.writeframes(pcm.tobytes())


def biquad_bandpass(signal, rate, low, high):
    """Two cascaded one-pole filters per edge: high-pass at low, low-pass at high."""
    x = signal
    # one-pole high-pass (2x for steeper skirt)
    for _ in range(2):
        alpha = 1.0 / (1.0 + 2.0 * np.pi * low / rate)
        y = np.empty_like(x)
        prev_x = x[0]
        prev_y = x[0]
        for i in range(len(x)):
            prev_y = alpha * (prev_y + x[i] - prev_x)
            prev_x = x[i]
            y[i] = prev_y
        x = y
    # one-pole low-pass (2x)
    for _ in range(2):
        alpha = (2.0 * np.pi * high / rate) / (1.0 + 2.0 * np.pi * high / rate)
        y = np.empty_like(x)
        prev = x[0]
        for i in range(len(x)):
            prev = prev + alpha * (x[i] - prev)
            y[i] = prev
        x = y
    return x


def main():
    os.makedirs(OUT, exist_ok=True)
    with open(MANIFEST, encoding="utf-8") as handle:
        beats = json.load(handle)

    for beat in beats:
        src = os.path.join(RAW, beat["id"] + ".wav")
        dst = os.path.join(OUT, beat["id"] + ".wav")
        signal, rate = read_wav(src)
        if len(signal) < 64:
            print("skip (too short):", beat["id"])
            continue

        band = biquad_bandpass(signal, rate, 420.0, 3200.0)
        band = band / (np.max(np.abs(band)) or 1.0)

        # radio hiss + mains hum, then soft clipping for the compression grit
        hiss = rng.normal(0.0, 0.012, size=band.shape)
        t = np.arange(len(band)) / float(rate)
        hum = 0.006 * np.sin(2.0 * np.pi * 50.0 * t)
        mixed = band * 0.9 + hiss + hum
        mixed = np.tanh(mixed * 1.8) * 0.85

        # short click-in at the start
        click_len = int(rate * 0.05)
        mixed[:click_len] += rng.normal(0.0, 0.05, size=click_len) * np.linspace(1.0, 0.0, click_len)

        # remove DC offset (the tanh + hum leaves a bias the importer warns about) and
        # fade the edges so the one-shot does not pop.
        mixed -= float(np.mean(mixed))
        fade = int(rate * 0.01)
        if len(mixed) > 2 * fade:
            mixed[:fade] *= np.linspace(0.0, 1.0, fade)
            mixed[-fade:] *= np.linspace(1.0, 0.0, fade)

        write_wav(dst, mixed, rate)
        print(beat["id"], "->", dst, "%.2fs" % (len(signal) / rate))


if __name__ == "__main__":
    main()
