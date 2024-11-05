#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace Utils {

    static std::string decodeBase64Gzip(const std::string& input) {
        return ZipUtils::decompressString(input, false, 0);
    }

    static std::map<std::string, std::string> splitToMap(const std::string& str, const std::string& split) {
        std::map<std::string, std::string> resultMap;
        if (str.empty()) return resultMap;

        std::string s = str;
        std::string key;
        size_t pos;
        bool isKey = true;

        while ((pos = s.find(split)) != std::string::npos) {
            std::string token = s.substr(0, pos);
            s.erase(0, pos + split.length());
            if (isKey) key = token;
            else resultMap[key] = token;
            isKey = !isKey;
        }

        if (!isKey) resultMap[key] = s;
        return resultMap;
    }

    static ccHSVValue rgbToHsv(ccColor3B color) {
        float fr = color.r / 255.0f;
        float fg = color.g / 255.0f;
        float fb = color.b / 255.0f;

        float max = std::max({fr, fg, fb});
        float min = std::min({fr, fg, fb});
        float delta = max - min;

        float h = 0.0f, s = 0.0f, v = max;

        if (delta > 0.0f) {
            if (max == fr) {
                h = 60.0f * std::fmod(((fg - fb) / delta), 6.0f);
            } else if (max == fg) {
                h = 60.0f * (((fb - fr) / delta) + 2.0f);
            } else if (max == fb) {
                h = 60.0f * (((fr - fg) / delta) + 4.0f);
            }
            if (h < 0.0f) h += 360.0f;
        }

        s = (max == 0.0f) ? 0.0f : (delta / max);

        return ccHSVValue{h, s, v};
    }

    static ccColor3B hsvToRgb(ccHSVValue color) {
        float r = 0.0f, g = 0.0f, b = 0.0f;

        if (color.s == 0.0f) {
            r = g = b = color.v;
        } else {
            float c = color.v * color.s;
            float x = c * (1 - std::fabs(fmod(color.h / 60.0f, 2) - 1));
            float m = color.v - c;

            if (color.h >= 0 && color.h < 60) {
                r = c, g = x, b = 0;
            } else if (color.h >= 60 && color.h < 120) {
                r = x, g = c, b = 0;
            } else if (color.h >= 120 && color.h < 180) {
                r = 0, g = c, b = x;
            } else if (color.h >= 180 && color.h < 240) {
                r = 0, g = x, b = c;
            } else if (color.h >= 240 && color.h < 300) {
                r = x, g = 0, b = c;
            } else {
                r = c, g = 0, b = x;
            }

            r += m;
            g += m;
            b += m;
        }

        uint8_t r_byte = static_cast<uint8_t>(r * 255);
        uint8_t g_byte = static_cast<uint8_t>(g * 255);
        uint8_t b_byte = static_cast<uint8_t>(b * 255);

        return ccColor3B{r_byte, g_byte, b_byte};
    }

}