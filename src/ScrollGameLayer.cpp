#include "ScrollGameLayer.h"
#include "Utils.h"

ScrollGameLayer* ScrollGameLayer::create(GJGameLevel* level, bool fade, ccColor3B bgColor, bool lowerGround) {
    auto ret = new ScrollGameLayer();
    if (ret && ret->init(level, fade, bgColor, lowerGround)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool ScrollGameLayer::init(GJGameLevel* level, bool fade, ccColor3B bgColorDefault, bool lowerGround) {

    CCSize winSize = CCDirector::get()->getWinSize();

    m_lowerGround = lowerGround;
    setZOrder(-1);
    scheduleUpdate();
    setContentSize(winSize);
    setCascadeOpacityEnabled(true);
    setID("scroll-bg"_spr);

    int bgID = Mod::get()->getSettingValue<int64_t>("background-id");
    int groundID = Mod::get()->getSettingValue<int64_t>("ground-id");
    int lineID = Mod::get()->getSettingValue<int64_t>("line-id");

    ccColor3B bgColor = Mod::get()->getSettingValue<ccColor3B>("background-color");
    ccColor3B groundColor1 = Mod::get()->getSettingValue<ccColor3B>("ground1-color");
    ccColor3B groundColor2 = Mod::get()->getSettingValue<ccColor3B>("ground2-color");

    if (Mod::get()->getSettingValue<bool>("override-color-default")) {
        bgColor = bgColorDefault;
        groundColor1 = bgColorDefault;
        groundColor2 = bgColorDefault;
    }

    float groundMod = 0;
    if (lowerGround) groundMod = -42;

    ccColor3B lineColor = Mod::get()->getSettingValue<ccColor3B>("line-color");
    bool blending = Mod::get()->getSettingValue<bool>("line-blending");

    if (!level->m_levelString.empty()) {
        auto decompressString = Utils::decodeBase64Gzip(level->m_levelString);
        std::string start = decompressString.substr(0, decompressString.find(';'));
        std::map<std::string, std::string> parts = Utils::splitToMap(start, ",");

        if (!Mod::get()->getSettingValue<bool>("always-default")) {
            std::vector<std::string> colors = utils::string::split(parts["kS38"], "|");
            std::map<std::string, std::map<std::string, std::string>> colorValues;

            if (!parts["kS1"].empty()) {
                pre19ColorToNew("1000", {parts["kS1"], parts["kS2"], parts["kS3"]}, colorValues);
                pre19ColorToNew("1001", {parts["kS4"], parts["kS5"], parts["kS6"]}, colorValues);
                pre19ColorToNew("1002", {parts["kS7"], parts["kS8"], parts["kS9"]}, colorValues);
            }
            else if (parts["kS38"].empty()) {
                pre20ColorToNew("1000", parts["kS29"], colorValues);
                pre20ColorToNew("1001", parts["kS30"], colorValues);
                pre20ColorToNew("1002", parts["kS31"], colorValues);
            }
            else {
                for (std::string color : colors) {
                    std::map<std::string, std::string> colorAttr = Utils::splitToMap(color, "_");
                    colorValues[colorAttr["6"]] = colorAttr;
                }
            }
        
            bgColor = getColor(colorValues, "1000");
            groundColor1 = getColor(colorValues, "1001");
            groundColor2 = getColor(colorValues, "1009");
            lineColor = getColor(colorValues, "1002", false);
            blending = utils::numFromString<int>(colorValues["1002"]["5"]).unwrapOr(0);
        }
        if (!Mod::get()->getSettingValue<bool>("always-default-texture")) {
            bgID = utils::numFromString<int>(parts["kA6"]).unwrapOr(1);
            if (bgID == 0) bgID = 1;

            groundID = utils::numFromString<int>(parts["kA7"]).unwrapOr(1);
            if (groundID == 0) groundID = 1;

            lineID = utils::numFromString<int>(parts["kA17"]).unwrapOr(1);
            if (lineID == 0) lineID = 1;
        }
    }

    bool scaleLineVertically = false;
    if (lineID == 3) {
        lineID = 2;
        scaleLineVertically = true;
    }

    std::string bgTextureStr = fmt::format("game_bg_{:02}_001.png", bgID);
    std::string groundTextureStr = fmt::format("groundSquare_{:02}_001.png", groundID);
    std::string groundTextureStr2 = fmt::format("groundSquare_{:02}_2_001.png", groundID);
    std::string lineTextureStr = fmt::format("floorLine_{:02}_001.png", lineID);

    m_background = createRepeatingSprite(bgTextureStr.c_str(), bgColor);
    m_ground = createRepeatingSprite(groundTextureStr.c_str(), groundColor1);
    m_ground2 = createRepeatingSprite(groundTextureStr2.c_str(), groundColor2);
    if (!m_ground2) {
        m_ground2 = CCSprite::create();
        m_ground2->setVisible(false);
    }
    m_line = createLine(lineID, lineColor, blending);

    if (scaleLineVertically) m_line->setScaleY(2);
    
    if (!Mod::get()->getSettingValue<bool>("hide-ground")) {
        m_background->setPositionY(30 + groundMod);
    }
    m_background->setID("background");
    m_background->setScale(1.185f);

    m_ground->setID("ground");
    m_ground->setPositionY(-38 + groundMod);
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

    m_ground->addChild(m_ground2);
    m_ground2->setID("ground-2");
    m_ground2->setAnchorPoint({0.f, 1.f});
    m_ground2->setPositionY(m_ground->getContentHeight());
    m_ground2->setZOrder(0);
    
    return true;
}

void ScrollGameLayer::updateBG(LevelData data) {

    if (data.backgroundID == 0) {
        data.backgroundID = 1;
        data.backgroundColor = {40, 125, 255};
        data.groundID = 1;
        data.ground1Color = {0, 102, 255};
        data.ground2Color = {0, 102, 255};
        data.lineID = 1;
        data.lineColor = {255, 255, 255};
        data.blendLine = true;
    }

    bool scaleLineVertically = false;
    if (data.lineID == 3) {
        data.lineID = 2;
        scaleLineVertically = true;
    }

    std::string bgTextureStr = fmt::format("game_bg_{:02}_001.png", data.backgroundID);
    std::string groundTextureStr = fmt::format("groundSquare_{:02}_001.png", data.groundID);
    std::string groundTextureStr2 = fmt::format("groundSquare_{:02}_2_001.png", data.groundID);
    std::string lineTextureStr = fmt::format("floorLine_{:02}_001.png", data.lineID);

    CCSprite* bgSprite = createRepeatingSprite(bgTextureStr.c_str(), data.backgroundColor);
    m_background->setTexture(bgSprite->getTexture());
    m_background->setTextureRect(bgSprite->getTextureRect());
    m_background->setColor(data.backgroundColor);

    CCSprite* groundSprite = createRepeatingSprite(groundTextureStr.c_str(), data.ground1Color);
    m_ground->setTexture(groundSprite->getTexture());
    m_ground->setTextureRect(groundSprite->getTextureRect());
    m_ground->setColor(data.ground1Color);

    CCSprite* ground2Sprite = createRepeatingSprite(groundTextureStr2.c_str(), data.ground2Color);
    if (ground2Sprite) {
        m_ground2->setVisible(true);
        m_ground2->setTexture(ground2Sprite->getTexture());
        m_ground2->setTextureRect(ground2Sprite->getTextureRect());
        m_ground2->setColor(data.ground2Color);
    }
    else {
        m_ground2->setVisible(false);
    }

    CCSprite* lineSprite = createLine(data.lineID, data.lineColor, data.blendLine);
    m_line->setTexture(lineSprite->getTexture());
    m_line->setTextureRect(lineSprite->getTextureRect());
    m_line->setColor(data.lineColor);
   
    if (data.blendLine) {
        m_line->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
    }
    else {
        m_line->setBlendFunc({GL_ONE, GL_ONE});
    }

    if (scaleLineVertically) m_line->setScaleY(2);
    else m_line->setScaleY(1);
    
    float groundMod = 0;
    if (m_lowerGround) groundMod = -42;

    m_background->setPositionY(30 + groundMod);
    m_background->setScale(1.185f);

    m_ground->setPositionY(-38 + groundMod);
    m_ground->setContentHeight(130);
}

void ScrollGameLayer::scroll(float dt) {
    m_bgPos = m_bgPos + 25 * dt;
    m_groundPos = m_groundPos + 150 * dt;

    if (m_bgPos >= static_cast<CCFloat*>(m_background->getUserObject("orig-width"_spr))->getValue()) m_bgPos = 0;
    if (m_groundPos >= static_cast<CCFloat*>(m_ground->getUserObject("orig-width"_spr))->getValue()) m_groundPos = 0;

    m_background->setTextureRect({m_bgPos, 0, m_background->getContentWidth(), m_background->getContentHeight()});
    m_ground->setTextureRect({m_groundPos, 0, m_ground->getContentWidth(), m_ground->getContentHeight()});
    if (m_ground2) {
        m_ground2->setTextureRect({m_groundPos, 0, m_ground2->getContentWidth(), m_ground2->getContentHeight()});
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
        spr->setUserObject("orig-width"_spr, CCFloat::create(spr->getContentWidth()));
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

void ScrollGameLayer::pre19ColorToNew(std::string id, std::vector<std::string> parts, std::map<std::string, std::map<std::string, std::string>>& colorValues) {
    
    if (parts[0].empty() && parts[1].empty() && parts[2].empty()) {
        parts[0] = "255";
        parts[1] = "255";
        parts[2] = "255";
    }
    
    colorValues[id]["6"] = id;
    colorValues[id]["1"] = parts[0];
    colorValues[id]["2"] = parts[1];
    colorValues[id]["3"] = parts[2];
}

void ScrollGameLayer::pre20ColorToNew(std::string id, std::string part, std::map<std::string, std::map<std::string, std::string>>& colorValues) {
    
    std::map<std::string, std::string> colorAttr = Utils::splitToMap(part, "_");

    colorValues[id]["6"] = id;
    colorValues[id]["1"] = colorAttr["1"];
    colorValues[id]["2"] = colorAttr["2"];
    colorValues[id]["3"] = colorAttr["3"];
    colorValues[id]["4"] = colorAttr["4"];
    colorValues[id]["5"] = colorAttr["5"];
    colorValues[id]["7"] = colorAttr["7"];
    colorValues[id]["8"] = colorAttr["8"];
    colorValues[id]["9"] = colorAttr["9"];
    colorValues[id]["10"] = colorAttr["10"];
}

LevelData ScrollGameLayer::getLevelData(GJGameLevel* level) {

    int bgID = 1;
    int groundID = 1;
    int lineID = 1;

    ccColor3B bgColor = {0, 0, 0};
    ccColor3B groundColor1 = {0, 0, 0};
    ccColor3B groundColor2 = {0, 0, 0};
    ccColor3B lineColor = {0, 0, 0};

    bool blending = true;

    if (!level->m_levelString.empty()) {
        auto decompressString = Utils::decodeBase64Gzip(level->m_levelString);
        std::string start = decompressString.substr(0, decompressString.find(';'));
        std::map<std::string, std::string> parts = Utils::splitToMap(start, ",");

        std::vector<std::string> colors = utils::string::split(parts["kS38"], "|");
        std::map<std::string, std::map<std::string, std::string>> colorValues;

        if (!parts["kS1"].empty()) {
            pre19ColorToNew("1000", {parts["kS1"], parts["kS2"], parts["kS3"]}, colorValues);
            pre19ColorToNew("1001", {parts["kS4"], parts["kS5"], parts["kS6"]}, colorValues);
            pre19ColorToNew("1002", {parts["kS7"], parts["kS8"], parts["kS9"]}, colorValues);
        }
        else if (parts["kS38"].empty()) {
            pre20ColorToNew("1000", parts["kS29"], colorValues);
            pre20ColorToNew("1001", parts["kS30"], colorValues);
            pre20ColorToNew("1002", parts["kS31"], colorValues);
        }
        else {
            for (std::string color : colors) {
                std::map<std::string, std::string> colorAttr = Utils::splitToMap(color, "_");
                colorValues[colorAttr["6"]] = colorAttr;
            }
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

    return LevelData {bgColor, groundColor1, groundColor2, lineColor, bgID, groundID, lineID, blending};
}