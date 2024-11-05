#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ScrollGameLayer : public CCNodeRGBA {

    private:
        CCSprite* m_background;
        CCSprite* m_ground;
        CCSprite* m_ground2;
        CCSprite* m_line;

    public:
        static ScrollGameLayer* create(GJGameLevel* level, bool fade);
        bool init(GJGameLevel* level, bool fade);
        CCSprite* createLine(int lineID, ccColor3B color, bool blending);
        CCSprite* createRepeatingSprite(std::string texture, ccColor3B color);

        ccColor3B getColor(std::map<std::string, std::map<std::string, std::string>> colors, std::string colorID, bool adjust = true);
        void scroll(float dt);
};