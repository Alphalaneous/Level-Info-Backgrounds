#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "ScrollGameLayer.h"

using namespace geode::prelude;

class $modify(MyLevelInfoLayer, LevelInfoLayer) {

	struct Fields {
		ScrollGameLayer* m_scrollGameLayer;
	};

    bool init(GJGameLevel* level, bool challenge) {
		if (!LevelInfoLayer::init(level, challenge)) return false;
		
		if ((level && !level->m_levelString.empty()) || Mod::get()->getSettingValue<bool>("always-default")) {
			m_fields->m_scrollGameLayer = ScrollGameLayer::create(level, false);
			addChild(m_fields->m_scrollGameLayer);
		}
		return true;
	}

	void levelDownloadFinished(GJGameLevel* level) {
		LevelInfoLayer::levelDownloadFinished(level);
		if (!m_fields->m_scrollGameLayer) {
			m_fields->m_scrollGameLayer = ScrollGameLayer::create(level, true);
			addChild(m_fields->m_scrollGameLayer);
		}
	}
};