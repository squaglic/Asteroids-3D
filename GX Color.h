#pragma once

struct Color
{
    float R, G, B, A;

    Color() {}
    Color(float r, float g, float b) { R = r; G = g; B = b; A = 1; }
    Color(float r, float g, float b, float a) { R = r; G = g; B = b; A = a; }

    const static Color Black, White, Red, Green, Blue, Magenta, Cyan, Yellow, Gray;

    static Color ColorFrom255(int r, int g, int b);
    static Color ColorFromHex(int hexCode);
};