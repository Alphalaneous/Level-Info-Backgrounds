#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct LevelData {
	ccColor3B backgroundColor;
    ccColor3B ground1Color;
    ccColor3B ground2Color;
    ccColor3B lineColor;
    int backgroundID;
    int groundID;
    int lineID;
    bool blendLine;
};

class ScrollGameLayer : public CCNodeRGBA {

    private:
        CCSprite* m_background;
        CCSprite* m_ground;
        CCSprite* m_ground2;
        CCSprite* m_line;
        bool m_lowerGround = false;
        float m_bgPos = 0;
        float m_groundPos = 0;

    public:
        static ScrollGameLayer* create(GJGameLevel* level, bool fade, ccColor3B bgColor, bool lowerGround = false);
        static LevelData getLevelData(GJGameLevel* level);
        static ccColor3B getColor(std::map<std::string, std::map<std::string, std::string>> colors, std::string colorID, bool adjust = true);
        static void pre20ColorToNew(std::string id, std::string part, std::map<std::string, std::map<std::string, std::string>>& colorValues);
        static void pre19ColorToNew(std::string id, std::vector<std::string> parts, std::map<std::string, std::map<std::string, std::string>>& colorValues);
       
        bool init(GJGameLevel* level, bool fade, ccColor3B bgColor, bool lowerGround);
        void updateBG(LevelData data);
        CCSprite* createLine(int lineID, ccColor3B color, bool blending);
        CCSprite* createRepeatingSprite(std::string texture, ccColor3B color);
         void scroll(float dt);
};