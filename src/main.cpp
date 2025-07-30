#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/utils/web.hpp>
#include <sstream>
#include <vector>

using namespace geode::prelude;

struct Social {
    std::string key;
    std::string png;
    bool isCustom;
};

static const std::vector<Social> socials = {
    // if anyone sees this and wants me to add some of these back just dm me on discord (@jarvisdevlin)
    // i dont see any usage of most of these so i just removed them for now or they are going to get me killed

    {"discord",      "gj_discordIcon_001.png", false},
    {"reddit",       "gj_rdIcon_001.png",      false},
    //{"facebook",     "gj_fbIcon_001.png",      false}, (god damn no)
    {"tiktok",       "tiktok.png",             true},
    {"bluesky",      "bluesky.png",            true},
    //{"deviantart",   "deviantart.png",         true}, (people will start linking their nsfw so hell naw brother)
    {"github",       "github.png",             true},
    //{"gitlab",       "gitlab.png",             true}, (ehh)
    {"instagram",    "instagram.png",          true},
    //{"pinterest",    "pinterest.png",          true}, (dead)
    //{"rumble",       "rumble.png",             true}, (99% of geode will kill me for this)
    //{"snapchat",     "snapchat.png",           true}, (dream)
    //{"telegram",     "telegram.png",           true}, (home of the truly awesome and cool racist 14 year old randoms. :skull: :skull: :skull: :skull: /j)
    //{"linkedin",     "linkedin.png",           true}, (i forgot ppl dont know how to get a job)
    //{"mastodon",     "mastodon.png",           true}, (dead)
    //{"patreon",      "patreon.png",            true}, (cough cough)
    //{"soundcloud",   "soundcloud.png",         true}, (planned)
    //{"spotify",      "spotify.png",            true}, (how)
    //{"threads",      "threads.png",            true}, (what)
    //{"tumblr",       "tumblr.png",             true}, (dead)
    //{"vimeo",        "vimeo.png",              true}, (dead)
    //{"vk",           "vk.png",                 true}, (yeah uhhhh no)
    //{"bandlab",      "bandlab.png",            true}, (do i even need to explain this one)
    //{"roblox",       "roblox.png",             true}, (teddy)
    //{"steam",        "steam.png",              true}, (sure)
    //{"itchio",       "itchio.png",             true}, (sure)
    //{"kofi",         "kofi.png",               true}, (at least its better then patreon)
    // rec room would be funny to add here (rec.net or smth idk)
};

class $modify(jdMS, ProfilePage) {
    struct Fields {
        EventListener<web::WebTask> listener;
    };

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        auto task = web::WebRequest()
            .userAgent("MoreSocials 1.0 (Geode)")
            .timeout(std::chrono::seconds(10))
            .get(fmt::format("https://api.jarvisdevil.com/socials/get.php?accountID={}", score->m_accountID));

        m_fields->listener.bind([this, score](web::WebTask::Event* ev) {
            auto maybe = ev->getValue();
            if (!maybe || !maybe->ok()) return;

            std::string body = maybe->string().unwrapOr("");
            //if (body.empty()) return;

            auto robsocials = static_cast<CCMenu*>(getChildByIDRecursive("socials-menu"));
            if (!robsocials) return;
            if (getChildByIDRecursive("more-socials-menu"_spr)) { return; } // wah wah wah

            auto moresocials = CCMenu::create();
            moresocials->setID("more-socials-menu"_spr);
            moresocials->setPosition(robsocials->getPositionX() + 40,
                                    robsocials->getPositionY() - 75); // might do better later
            moresocials->setContentWidth(28.f);
            this->m_mainLayer->addChild(moresocials);

            float y = 0.f;
            std::istringstream ss(body);
            std::string part;
            while (std::getline(ss, part, ';')) {
                auto eq = part.find('=');
                if (eq == std::string::npos) continue;
                auto key = part.substr(0, eq);
                auto link = part.substr(eq + 1);
                if (link.empty()) continue;

                for (auto const& info : socials) {
                    if (info.key == key) {
                        CCSprite* spr = nullptr;
                        if (info.isCustom) {
                            spr = CCSprite::create(fmt::format("{}"_spr, info.png).c_str());
                        } else {
                            spr = CCSprite::createWithSpriteFrameName(info.png.c_str());
                        }
                        spr->setScale(0.9f);
                        if (!spr) break;
                        auto item = CCMenuItemSpriteExtra::create(
                            spr, this, menu_selector(jdMS::onSocial)
                        );
                        item->setID(fmt::format("{}_social", info.key).c_str());
                        item->setUserObject(CCString::create(link));
                        item->setPositionY(-y);
                        y += spr->getContentSize().height + 3.f;
                        moresocials->addChild(item);
                        break;
                    }
                }
            }

            if (score->m_accountID == GJAccountManager::sharedState()->m_accountID) {
                auto more = CCSprite::createWithSpriteFrameName("GJ_infoBtn_001.png");
                more->setID("more-socials-info"_spr);
                more->setScale(0.65f);
                if (more) {
                    auto moreBtn = CCMenuItemSpriteExtra::create(
                        more, this, menu_selector(jdMS::onMore)
                    );
                    moresocials->addChild(moreBtn);
                    moreBtn->setID("more-socials-button"_spr);
                }
            }

            // moresocials->alignItemsVerticallyWithPadding(3.f); // jarvis what the hell i thought the geode docs taught you better than to use outdated cocos functions. geode::Layout* existed for a reason dammit

        });

        m_fields->listener.setFilter(task);
    }

    void onMore(CCObject*) {
        geode::createQuickPopup(
            "MoreSocials",
            "If you want to edit your socials, head over to https://id.jarvisdevil.com.\n\n<cb>Like my mods? Support me by joining my Discord Server: https://dsc.gg/devlin</c>",
            "Nah",
            "OK",
            [](auto, bool btn2) {
                if (btn2) {
                    web::openLinkInBrowser("https://id.jarvisdevil.com");
                }
            }
        );
    }

    void onSocial(CCObject* sender) {
        auto item = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto obj = item->getUserObject();
        if (!obj) return;
        auto str = static_cast<CCString*>(obj)->getCString();
        web::openLinkInBrowser(str);
    }
};