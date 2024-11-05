#include "ScrollGameLayer.h"
#include "Utils.h"

ScrollGameLayer* ScrollGameLayer::create(GJGameLevel* level, bool fade) {
    auto ret = new ScrollGameLayer();
    if (ret && ret->init(level, fade)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool ScrollGameLayer::init(GJGameLevel* level, bool fade) {

    CCSize winSize = CCDirector::get()->getWinSize();

    setZOrder(-1);
    scheduleUpdate();
    setContentSize(winSize);
    setCascadeOpacityEnabled(true);
    setID("scroll-bg"_spr);

    int bgID = 1;
    int groundID = 1;
    int lineID = 1;

    ccColor3B bgColor = Mod::get()->getSettingValue<ccColor3B>("background-color");
    ccColor3B groundColor1 = Mod::get()->getSettingValue<ccColor3B>("ground1-color");
    ccColor3B groundColor2 = Mod::get()->getSettingValue<ccColor3B>("ground2-color");
    ccColor3B lineColor = Mod::get()->getSettingValue<ccColor3B>("line-color");
    bool blending = Mod::get()->getSettingValue<bool>("line-blending");

    if (!level->m_levelString.empty()) {
        if (!Mod::get()->getSettingValue<bool>("always-default")) {
            auto decompressString = Utils::decodeBase64Gzip(level->m_levelString);

            std::string start = decompressString.substr(0, decompressString.find(';'));
            std::map<std::string, std::string> parts = Utils::splitToMap(start, ",");
            std::vector<std::string> colors = utils::string::split(parts["kS38"], "|");
            std::map<std::string, std::map<std::string, std::string>> colorValues;

            for (std::string color : colors) {
                std::map<std::string, std::string> colorAttr = Utils::splitToMap(color, "_");
                colorValues[colorAttr["6"]] = colorAttr;
            }
        
            bgColor = getColor(colorValues, "1000");
            groundColor1 = getColor(colorValues, "1001");
            groundColor2 = getColor(colorValues, "1009");
            lineColor = getColor(colorValues, "1002", false);
            blending = utils::numFromString<int>(colorValues["1002"]["5"]).unwrapOr(0);

            bgID = utils::numFromString<int>(parts["kA6"]).unwrapOr(1);
            if (bgID == 0) bgID = 1;

            groundID = utils::numFromString<int>(parts["kA7"]).unwrapOr(1);
            if (groundID == 0) groundID = 1;

            lineID = utils::numFromString<int>(parts["kA17"]).unwrapOr(1);
            if (lineID == 0) lineID = 1;
        }
    }

    std::string bgTextureStr = fmt::format("game_bg_{:02}_001.png", bgID);
    std::string groundTextureStr = fmt::format("groundSquare_{:02}_001.png", groundID);
    std::string groundTextureStr2 = fmt::format("groundSquare_{:02}_2_001.png", groundID);
    std::string lineTextureStr = fmt::format("floorLine_{:02}_001.png", lineID);

    m_background = createRepeatingSprite(bgTextureStr.c_str(), bgColor);
    m_ground = createRepeatingSprite(groundTextureStr.c_str(), groundColor1);
    m_ground2 = createRepeatingSprite(groundTextureStr2.c_str(), groundColor2);
    m_line = createLine(lineID, lineColor, blending);
    
    if (!Mod::get()->getSettingValue<bool>("hide-ground")) {
        m_background->setPositionY(30);
    }
    m_background->setID("background");
    m_background->setScale(1.185f);

    m_ground->setID("ground");
    m_ground->setPositionY(-38);
    m_ground->setContentHeight(130);

    m_line->setID("line");

    if (fade) {
        setOpacity(0);
        runAction(CCFadeIn::create(0.25));
    }

    if (!((level->isPlatformer() && Mod::get()->getSettingValue<bool>("stationary-platformer")) || Mod::get()->getSettingValue<bool>("always-stationary"))) {
        schedule(schedule_selector(ScrollGameLayer::scroll));
    }

    addChild(m_background);
    addChild(m_ground);
    
    if (Mod::get()->getSettingValue<bool>("hide-ground")) {
        m_ground->setVisible(false);
    }
    
    CCSprite* shadowLeft = CCSprite::createWithSpriteFrameName("groundSquareShadow_001.png");
    CCSprite* shadowRight = CCSprite::createWithSpriteFrameName("groundSquareShadow_001.png");
    shadowRight->setFlipX(true);

    shadowLeft->setScaleX(0.7);
    shadowLeft->setID("shadow-left");
    shadowLeft->setAnchorPoint({0.f, 1.f});
    shadowLeft->setOpacity(100);
    shadowLeft->setZOrder(2);
    shadowLeft->setPositionY(m_ground->getContentHeight());

    shadowRight->setScaleX(0.7);
    shadowRight->setID("shadow-right");
    shadowRight->setAnchorPoint({1.f, 1.f});
    shadowRight->setPositionX(winSize.width);
    shadowRight->setPositionY(m_ground->getContentHeight());

    shadowRight->setOpacity(100);
    shadowRight->setZOrder(2);

    m_ground->addChild(shadowLeft);
    m_ground->addChild(shadowRight);
    m_ground->addChild(m_line);

    m_line->setPositionY(m_ground->getContentHeight());

    if (m_ground2) {
        m_ground->addChild(m_ground2);
        m_ground2->setID("ground-2");
        m_ground2->setAnchorPoint({0.f, 1.f});
        m_ground2->setPositionY(m_ground->getContentHeight());
        m_ground2->setZOrder(0);
    }
    return true;
}

void ScrollGameLayer::scroll(float dt) {
    float bgPos = m_background->getTextureRect().origin.x + 25 * dt;
    float groundPos = m_ground->getTextureRect().origin.x + 150 * dt;

    m_background->setTextureRect({bgPos, 0, m_background->getContentWidth(), m_background->getContentHeight()});
    m_ground->setTextureRect({groundPos, 0, m_ground->getContentWidth(), m_ground->getContentHeight()});
    if (m_ground2) {
        m_ground2->setTextureRect({groundPos, 0, m_ground2->getContentWidth(), m_ground2->getContentHeight()});
    }
}

CCSprite* ScrollGameLayer::createLine(int lineID, ccColor3B color, bool blending) {
    CCSize winSize = CCDirector::get()->getWinSize();
    std::string texture = fmt::format("floorLine_{:02}_001.png", lineID);

    CCSprite* spr = CCSprite::createWithSpriteFrameName(texture.c_str());
    if (spr && !spr->getUserObject("geode.texture-loader/fallback")) {
        spr->setAnchorPoint({0.5, 1});
        spr->setPositionX(winSize.width/2);
        spr->setZOrder(1);
        spr->setColor(color);
        if (blending) spr->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
        if (lineID != 1) spr->setScaleX(winSize.width/ spr->getContentWidth());
        return spr;
    }

    return nullptr;
}

CCSprite* ScrollGameLayer::createRepeatingSprite(std::string texture, ccColor3B color) {
    CCSize winSize = CCDirector::get()->getWinSize();

    CCSprite* spr = CCSprite::create(texture.c_str());
    if (spr && !spr->getUserObject("geode.texture-loader/fallback")) {
        spr->setAnchorPoint({0, 0});
        spr->setContentWidth(winSize.width);
        spr->setZOrder(-1);
        spr->setColor(color);

        ccTexParams params = {GL_LINEAR, GL_LINEAR, GL_REPEAT, 0x2900};
        spr->getTexture()->setTexParameters(&params);
        spr->setTextureRect({0, 0, winSize.width, spr->getContentHeight()});
        return spr;
    }

    return nullptr;
}

ccColor3B ScrollGameLayer::getColor(std::map<std::string, std::map<std::string, std::string>> colors, std::string colorID, bool adjust) {

    std::map<std::string, std::string> colorAttr = colors[colorID];
    std::map<std::string, std::string> colorAttrOrig = colors[colorID];

    std::string copyColorIdx = colorAttr["9"];

    if (!copyColorIdx.empty()) {
        colorAttr = colors[copyColorIdx];
    }

    unsigned char r = utils::numFromString<unsigned char>(colorAttr["1"]).unwrapOr(0);
    unsigned char g = utils::numFromString<unsigned char>(colorAttr["2"]).unwrapOr(0);
    unsigned char b = utils::numFromString<unsigned char>(colorAttr["3"]).unwrapOr(0);

    ccColor3B color = ccColor3B{r, g, b};

    if (!copyColorIdx.empty()) {
        std::string hsvModifierStr = colorAttrOrig["10"];

        if (!hsvModifierStr.empty()) {
            std::vector<std::string> hsvModifiers = utils::string::split(hsvModifierStr, "a");
            
            float hue = utils::numFromString<float>(hsvModifiers.at(0)).unwrapOr(0);
            float saturation = utils::numFromString<float>(hsvModifiers.at(1)).unwrapOr(0);
            float value = utils::numFromString<float>(hsvModifiers.at(2)).unwrapOr(0);
            bool addSat = utils::numFromString<int>(hsvModifiers.at(3)).unwrapOr(0);
            bool addVal = utils::numFromString<int>(hsvModifiers.at(4)).unwrapOr(0);

            ccHSVValue hsv = Utils::rgbToHsv(color);

            hsv.h += hue;
            if (hsv.h >= 360) hsv.h -= 360;

            if (addSat) hsv.s += saturation;
            else hsv.s *= saturation;
            if (hsv.s >= 1) hsv.h = 1;

            if (addVal) hsv.v += value;
            else hsv.v *= value;
            if (hsv.v >= 1) hsv.v = 1;

            color = Utils::hsvToRgb(hsv);
        }
    }

    ccHSVValue hsv = Utils::rgbToHsv(color);

    if (adjust) {
        
        double brightnessMax = Mod::get()->getSettingValue<double>("brightness-max");
        double brightnessMin = Mod::get()->getSettingValue<double>("brightness-min");

        if (hsv.v > brightnessMax) hsv.v = brightnessMax;
        if (hsv.v < brightnessMin) hsv.v = brightnessMin;
    }

    return Utils::hsvToRgb(hsv);
}
