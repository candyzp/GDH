#pragma once

#include <Geode/Geode.hpp>
#include <array>
#include <vector>
#include <unordered_map>
#include <string>
#include <queue>

namespace GDH {
    namespace Labels {
        enum Corner {
            Top_Left,
            Top_Center,
            Top_Right,
            Center_Left,
            Center_Center,
            Center_Right,
            Bottom_Left,
            Bottom_Center,
            Bottom_Right,
        };

        enum LabelType {
            Text,
            Spacing,
            CheatIndicator
        };

        class Label {
        public:
            bool enabled;
            LabelType type;
            std::string text;
            std::array<float, 4> color;
            float size;
            
            bool rainbow;
            bool cps;
            bool noclip;

            Label(std::string text, std::array<float, 4> color, float size = 0.3f, bool rainbow = false, bool cps = false, bool noclip = false) {
                this->enabled = true;
                this->type = LabelType::Text;
                this->text = text;
                this->color = color;
                this->size = size;
                this->rainbow = rainbow;
                this->cps = cps;
                this->noclip = noclip;
            }
            
            Label(float spacing) {
                this->enabled = true;
                this->type = LabelType::Spacing;
                this->text = "";
                this->color = {1.0f, 1.0f, 1.0f, 1.0f};
                this->size = spacing;
                this->rainbow = false;
                this->cps = false;
                this->noclip = false;
            }

            Label(float size, LabelType type, float opacity = 1.0f) {
                this->enabled = true;
                this->type = type;
                this->text = ".";
                this->color = {1.0f, 1.0f, 1.0f, opacity};
                this->size = size;
                this->rainbow = false;
                this->cps = false;
                this->noclip = false;
            }

            std::string render() const;
        };

        class Manager {
        public:
            static Manager& get() {
                static Manager instance;
                return instance;
            }

            Manager& operator=(const Manager&) = delete;
            Manager(const Manager&) = delete;

            float cornerPadding = 4.0f;
            float midPadding = 2.0f;
            std::unordered_map<Corner, std::vector<Label>> labels;

            void save();
            void load();
            
            bool m_cheatedInCurrentAttempt = false;

            float m_bestRunStart = 0.0f;
            float m_bestRunEnd = 0.0f;

            double m_bestTime = 0.0;

            void resetBestRun() {
                m_bestRunStart = 0.0f;
                m_bestRunEnd = 0.0f;
                m_bestTime = 0.0;
            }

            void updateBestRun(float start, float end) {
                if (end < start) return;

                if (end > m_bestRunEnd) {
                    m_bestRunStart = start;
                    m_bestRunEnd = end;
                }
            }

            void updateBestTime(double time) {
                if (time <= 0.0) return;

                if (m_bestTime == 0.0 || time < m_bestTime) {
                    m_bestTime = time;
                }
            }
        private:
            Manager() = default;
        };
    }
}

class NoclipAccuracy {
public:    
    static NoclipAccuracy& get() {
        static NoclipAccuracy instance;
        return instance;
    }

    NoclipAccuracy& operator=(const NoclipAccuracy&) = delete;
    NoclipAccuracy(const NoclipAccuracy&) = delete;

    int frames = 0, deaths = 0, deaths_full = 0;
    bool wouldDie = false, prevDied = false;

    void handle_update(GJBaseGameLayer* self, float delta) {
        auto pl = PlayLayer::get();
        if (!pl || self->m_player1->m_isDead || pl->m_levelEndAnimationStarted) {
            prevDied = false;
            return;
        }

        frames++;

        if (wouldDie) {
            wouldDie = false;
            deaths++;

            if (!prevDied) {
                deaths_full++;
                prevDied = true;
            }
        } else {
            prevDied = false;
        }
    }

    void handle_reset() {
        frames = 0;
        deaths = 0;
        deaths_full = 0;
        wouldDie = false;
        prevDied = false;
    }

    void handle_death() {
        wouldDie = true;
    }

    float getPercentage() const {
        if (frames == 0) return 100.0f;
        return (1.0f - static_cast<float>(deaths) / frames) * 100.0f;
    }

private:
    NoclipAccuracy() = default;
};

class CpsCounter {
public:
    static CpsCounter& get() {
        static CpsCounter instance;
        return instance;
    }

    CpsCounter(const CpsCounter&) = delete;
    CpsCounter& operator=(const CpsCounter&) = delete;

    std::queue<float> clicks;
    std::queue<float> p2_clicks;

    float current_time = 0.f;

    int cps = 0, highscore = 0, overall = 0;
    int p2_cps = 0, p2_highscore = 0, p2_overall = 0;

    bool p1_push = false;
    bool p2_push = false;

    void reset() {
        cps = highscore = overall = 0;
        p2_cps = p2_highscore = p2_overall = 0;
        p1_push = p2_push = false;
        current_time = 0.f;

        std::queue<float>().swap(clicks);
        std::queue<float>().swap(p2_clicks);
    }

    void click(bool isPlayer1) {
        if (isPlayer1) {
            overall++;
            clicks.push(current_time);
        } else {
            p2_overall++;
            p2_clicks.push(current_time);
        }
    }

    void update(float dt) {
        current_time += dt;

        while (!clicks.empty() && (current_time - clicks.front() > 1.0f)) clicks.pop();
        while (!p2_clicks.empty() && (current_time - p2_clicks.front() > 1.0f)) p2_clicks.pop();

        cps = static_cast<int>(clicks.size());
        p2_cps = static_cast<int>(p2_clicks.size());

        highscore = std::max(highscore, cps);
        p2_highscore = std::max(p2_highscore, p2_cps);
    }

private:
    CpsCounter() = default;
};