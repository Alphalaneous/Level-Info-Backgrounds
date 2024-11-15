#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelSelectLayer.hpp>
#include <Geode/modify/BoomScrollLayer.hpp>
#include "ScrollGameLayer.h"
#include "Utils.h"

using namespace geode::prelude;

class $modify(MyLevelSelectLayer, LevelSelectLayer) {

	struct Fields {
		ScrollGameLayer* m_scrollGameLayer;
		std::map<int, LevelData> m_levelData;
		int m_currentPage = 0;
	};

    bool init(int page) {
		if (!LevelSelectLayer::init(page)) return false;

		GJGameLevel* level = GameLevelManager::get()->getMainLevel(page + 1, false);

		for (int i = 1; i <= 22; i++) {
			GJGameLevel* level = GameLevelManager::get()->getMainLevel(i, false);
			if (!level) break;
			LevelData data = ScrollGameLayer::getLevelData(level);
			m_fields->m_levelData[i] = data;
		}

		m_fields->m_scrollGameLayer = ScrollGameLayer::create(level, false, {0, 0, 0}, true);
		m_fields->m_scrollGameLayer->updateBG(m_fields->m_levelData[page + 1]);

		if (CCNode* groundLayer = getChildByID("ground-layer")) {
			groundLayer->setVisible(false);
		}

		addChild(m_fields->m_scrollGameLayer);

		return true;
	}

	void updateBG() {
		int pageNum = m_scrollLayer->m_page;
		int count = m_scrollLayer->m_dynamicObjects->count();
		int page = (count + (pageNum % count)) % count;

		int id = 0;
		if (GJGameLevel* level = typeinfo_cast<GJGameLevel*>(m_scrollLayer->m_dynamicObjects->objectAtIndex(page))) {
			id = level->m_levelID;
		}
		
		m_fields->m_scrollGameLayer->updateBG(m_fields->m_levelData[id]);
	}
};

class $modify(MyBoomScrollLayer, BoomScrollLayer) {

    void moveToPage(int p0) {
		int page = m_page;
		BoomScrollLayer::moveToPage(p0);
		if (page == p0) return;
		if (LevelSelectLayer* lsl = typeinfo_cast<LevelSelectLayer*>(getParent())) {
			static_cast<MyLevelSelectLayer*>(lsl)->updateBG();
		}
	}
};

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