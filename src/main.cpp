#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "ScrollGameLayer.h"

using namespace geode::prelude;

class $modify(MyLevelInfoLayer, LevelInfoLayer) {

	static void onModify(auto& self) {
        (void) self.setHookPriority("LevelInfoLayer::init", -100);
        (void) self.setHookPriority("LevelInfoLayer::levelDownloadFinished", -100);
    }

	struct Fields {
		ScrollGameLayer* m_scrollGameLayer;
	};

    bool init(GJGameLevel* level, bool challenge) {
		if (!LevelInfoLayer::init(level, challenge)) return false;
		
		if ((level && !level->m_levelString.empty()) || Mod::get()->getSettingValue<bool>("always-default")) {
			ccColor3B color = {0, 102, 255};
			if (CCSprite* bg = typeinfo_cast<CCSprite*>(getChildByID("background"))) {
				color = bg->getColor();
			}

			m_fields->m_scrollGameLayer = ScrollGameLayer::create(level, false, color);
			addChild(m_fields->m_scrollGameLayer);

			if (CCLayerGradient* gradient = getChildByType<CCLayerGradient>(0)) {
				gradient->setZOrder(-1);
				gradient->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
				gradient->setOpacity(80);
			}
		}
		return true;
	}

	void levelDownloadFinished(GJGameLevel* level) {
		LevelInfoLayer::levelDownloadFinished(level);
		if (!m_fields->m_scrollGameLayer) {
			ccColor3B color = {0, 102, 255};
			if (CCSprite* bg = typeinfo_cast<CCSprite*>(getChildByID("background"))) {
				color = bg->getColor();
			}
			
			m_fields->m_scrollGameLayer = ScrollGameLayer::create(level, true, color);
			addChild(m_fields->m_scrollGameLayer);

			if (CCLayerGradient* gradient = getChildByType<CCLayerGradient>(0)) {
				gradient->setZOrder(-1);
				gradient->setBlendFunc({GL_SRC_ALPHA, GL_ONE});
				gradient->setOpacity(80);
			}
		}
	}
};