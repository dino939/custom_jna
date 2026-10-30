//
// Created by Shaori on 04.08.2026.
//

#ifndef XDANTIDUMPMEM_PATTERNS_H
#define XDANTIDUMPMEM_PATTERNS_H
static const char *kRewriterPattern = "4C 8B DC 49 89 5B ? 49 89 6B ? 56 57 41 54 41 56 41 57 48 81 EC";
static const char *kSymbolTableNewSymbolPattern = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 63 FA 48 8B F1 81 FF FF FF 00 00";
// ms-openjdk 21.0.12 jvm.dll (unique, verified 1 match each)
static const char *kSafepointBeginPattern = "40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC F8 00 00 00 80 3D";
static const char *kSafepointEndPattern = "48 89 5C 24 ? 48 89 7C 24 ? 55 48 8D 6C 24 ? 48 81 EC 50 01 00 00";
#endif //XDANTIDUMPMEM_PATTERNS_H
