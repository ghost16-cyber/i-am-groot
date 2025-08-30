// ============================================================================
// Dynamic Challenges System (C++) — engine-agnostic core + console demo
// --------------------------------------------------------------------------
// Build (GCC/Clang):
//   g++ -std=c++17 -O2 -Wall -Wextra -pedantic -o dynamic_demo dynamic_challenges_demo.cpp
// Run:
//   ./dynamic_demo
// This file is self-contained. Integrate the core classes with your engine.
// ============================================================================

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace groot::dynamic {

// -----------------------------------------
// Challenge model
// -----------------------------------------

enum class ChallengeType { MeteorShower, VineOvergrowth, AcidRain, LowGravity, ResourceBoost };

struct ChallengeDef {
    std::string id;
    std::string displayName;
    std::string description;
    ChallengeType type { ChallengeType::MeteorShower };
    float duration { 30.0f };           // seconds
    float intensity { 1.0f };           // global scaling knob (>= 0.1)
    float weight { 1.0f };              // selection weight
    bool canRepeatConsecutively { false };

    // Optional spawn-related knobs for hazards/obstacles
    int spawnCount { 20 };
    float spawnRadius { 40.0f };
};

// -----------------------------------------
// Environment interface (+ simple demo env)
// -----------------------------------------

struct IEntity { virtual ~IEntity() = default; virtual void applyDamage(float) = 0; };

struct DemoEntity : IEntity { // minimal demo-only entity with health
    explicit DemoEntity(float hp = 100.f) : health(hp) {}
    float health { 100.f };
    void applyDamage(float amt) override { health = std::max(0.f, health - amt); }
};

class IEnvironment {
public:
    virtual ~IEnvironment() = default;
    virtual void apply(const ChallengeDef& def) = 0;
    virtual void revert(const ChallengeDef& def) = 0;
    virtual void tick(float dt) = 0; // why: allows continuous effects (e.g., acid rain)
};

class DemoEnvironment : public IEnvironment {
public:
    // Observables that your game can read to react (gravity, resource multiplier, etc.)
    float gravityScale { 1.0f };
    float resourceMultiplier { 1.0f };

    // Demo state (what we "spawned")
    int activeMeteors { 0 };
    int activeVines { 0 };

    // Entities present in the world (receive damage during AcidRain/Meteor hits)
    std::vector<DemoEntity> entities { DemoEntity(120.f), DemoEntity(80.f), DemoEntity(150.f) };

    // Continuous-effects state
    bool acidRainActive { false };
    float acidRainTimer { 0.f };
    float acidRainInterval { 1.f }; // seconds between ticks
    float acidRainDamage { 5.f };

    void apply(const ChallengeDef& def) override {
        switch (def.type) {
            case ChallengeType::MeteorShower:
                activeMeteors += def.spawnCount; // why: tracks hazard volume for demo
                break;
            case ChallengeType::VineOvergrowth:
                activeVines += def.spawnCount;
                break;
            case ChallengeType::AcidRain:
                acidRainActive = true;
                acidRainInterval = std::max(0.15f, 1.0f / std::max(0.1f, def.intensity));
                acidRainDamage = 5.0f * std::max(0.1f, def.intensity);
                acidRainTimer = 0.f;
                break;
            case ChallengeType::LowGravity:
                gravityScale = 0.4f / std::max(0.4f, def.intensity); // lower is floatier
                break;
            case ChallengeType::ResourceBoost:
                resourceMultiplier = 1.0f + 0.5f * std::max(0.1f, def.intensity);
                break;
        }
    }

    void revert(const ChallengeDef& def) override {
        switch (def.type) {
            case ChallengeType::MeteorShower:
                activeMeteors = 0;
                break;
            case ChallengeType::VineOvergrowth:
                activeVines = 0;
                break;
            case ChallengeType::AcidRain:
                acidRainActive = false;
                break;
            case ChallengeType::LowGravity:
                gravityScale = 1.0f;
                break;
            case ChallengeType::ResourceBoost:
                resourceMultiplier = 1.0f;
                break;
        }
    }

    void tick(float dt) override {
        if (!acidRainActive) return;
        acidRainTimer += dt;
        if (acidRainTimer >= acidRainInterval) {
            acidRainTimer = 0.f;
            for (auto& e : entities) e.applyDamage(acidRainDamage);
        }
    }
};

// -----------------------------------------
// DynamicChallengeManager
// -----------------------------------------

class DynamicChallengeManager {
public:
    using ListenerStart = std::function<void(const ChallengeDef&)>;
    using ListenerEnd   = std::function<void(const ChallengeDef&)>;
    using ListenerTick  = std::function<void(const ChallengeDef&, float progress)>; // 0..1

    std::pair<float, float> initialDelayRange { 2.f, 5.f };
    std::pair<float, float> intervalRange { 8.f, 15.f };

    explicit DynamicChallengeManager(IEnvironment& env)
        : env_(env), rng_(std::random_device{}()) {}

    void setChallenges(std::vector<ChallengeDef> list) { challenges_ = std::move(list); }

    void onStart(ListenerStart cb) { startListeners_.push_back(std::move(cb)); }
    void onEnd(ListenerEnd cb) { endListeners_.push_back(std::move(cb)); }
    void onTick(ListenerTick cb) { tickListeners_.push_back(std::move(cb)); }

    void update(float dt) {
        // initial delay
        if (state_ == State::BootDelay) {
            countdown_ -= dt;
            if (countdown_ <= 0.f) startNewChallenge();
            return;
        }

        // active challenge ticking
        if (state_ == State::Active) {
            env_.tick(dt);
            timeIn_ += dt;
            float progress = std::clamp(timeIn_ / std::max(0.01f, current_.duration), 0.f, 1.f);
            for (auto& cb : tickListeners_) cb(current_, progress);
            if (timeIn_ >= current_.duration) endCurrentChallenge();
            return;
        }

        // between challenges
        if (state_ == State::Interval) {
            countdown_ -= dt;
            if (countdown_ <= 0.f) startNewChallenge();
        }
    }

    void startSystem() {
        state_ = State::BootDelay;
        countdown_ = randInRange(initialDelayRange);
    }

    void triggerNow(ChallengeType type) {
        // ends current & starts selected challenge immediately
        if (state_ == State::Active) endCurrentChallenge();
        auto it = std::find_if(challenges_.begin(), challenges_.end(), [&](const ChallengeDef& d) { return d.type == type; });
        if (it == challenges_.end()) return;
        current_ = *it;
        beginCurrent();
    }

private:
    enum class State { BootDelay, Active, Interval };

    IEnvironment& env_;
    std::vector<ChallengeDef> challenges_;
    ChallengeDef current_{};
    bool havePrev_ { false };
    ChallengeType prevType_ { ChallengeType::MeteorShower };

    State state_ { State::BootDelay };
    float countdown_ { 0.f };
    float timeIn_ { 0.f };

    std::mt19937 rng_;

    float randInRange(const std::pair<float, float>& p) {
        std::uniform_real_distribution<float> dist(std::min(p.first, p.second), std::max(p.first, p.second));
        return dist(rng_);
    }

    const ChallengeDef& pickRandomWeighted() {
        // build pool honoring canRepeatConsecutively
        pool_.clear(); weights_.clear();
        for (const auto& d : challenges_) {
            if (!havePrev_ || d.canRepeatConsecutively || d.type != prevType_) {
                pool_.push_back(&d);
                weights_.push_back(std::max(0.0001f, d.weight));
            }
        }
        if (pool_.empty()) {
            for (const auto& d : challenges_) { pool_.push_back(&d); weights_.push_back(std::max(0.0001f, d.weight)); }
        }
        float total = 0.f; for (float w : weights_) total += w;
        std::uniform_real_distribution<float> dist(0.f, total);
        float r = dist(rng_);
        float acc = 0.f;
        for (size_t i = 0; i < pool_.size(); ++i) {
            acc += weights_[i];
            if (r <= acc) return *pool_[i];
        }
        return *pool_.back();
    }

    void startNewChallenge() {
        if (challenges_.empty()) return;
        current_ = pickRandomWeighted();
        beginCurrent();
    }

    void beginCurrent() {
        current_.duration = std::max(1.f, current_.duration);
        current_.intensity = std::max(0.1f, current_.intensity);
        state_ = State::Active;
        timeIn_ = 0.f;
        env_.apply(current_);
        for (auto& cb : startListeners_) cb(current_);
    }

    void endCurrentChallenge() {
        env_.revert(current_);
        for (auto& cb : endListeners_) cb(current_);
        havePrev_ = true; prevType_ = current_.type;
        state_ = State::Interval;
        countdown_ = randInRange(intervalRange);
    }

    // event listeners
    std::vector<ListenerStart> startListeners_;
    std::vector<ListenerEnd>   endListeners_;
    std::vector<ListenerTick>  tickListeners_;

    // temp buffers
    std::vector<const ChallengeDef*> pool_;
    std::vector<float> weights_;
};

// -----------------------------------------
// Defaults helper
// -----------------------------------------

inline std::vector<ChallengeDef> defaultChallenges() {
    return {
        { "meteor", "Meteor Shower", "Meteors crash from the sky — take cover!", ChallengeType::MeteorShower, 25.f, 1.f, 1.f, false, 25, 45.f },
        { "vines",  "Vine Overgrowth", "Vines rapidly block paths and reshape routes.", ChallengeType::VineOvergrowth, 30.f, 1.2f, 0.8f, false, 18, 35.f },
        { "acid",   "Acid Rain", "Corrosive rain damages entities and structures.", ChallengeType::AcidRain, 20.f, 1.1f, 0.7f, true },
        { "lowgrav","Low Gravity", "Gravity weakens; jumps feel floaty.", ChallengeType::LowGravity, 15.f, 1.0f, 0.5f, true },
        { "boost",  "Resource Bloom", "Resource yields surge across the map!", ChallengeType::ResourceBoost, 25.f, 1.5f, 0.6f, true }
    };
}

// -----------------------------------------
// Tiny console UI helpers (demo)
// -----------------------------------------

static std::string toString(ChallengeType t) {
    switch (t) {
        case ChallengeType::MeteorShower: return "MeteorShower";
        case ChallengeType::VineOvergrowth: return "VineOvergrowth";
        case ChallengeType::AcidRain: return "AcidRain";
        case ChallengeType::LowGravity: return "LowGravity";
        case ChallengeType::ResourceBoost: return "ResourceBoost";
    }
    return "?";
}

static void printHeader() {
    std::cout << "\n=== Dynamic Challenges Demo (C++) ===\n";
    std::cout << "Spawns/FX are simulated. Integrate with your engine to hook visuals.\n\n";
}

} // namespace groot::dynamic

// ============================================================================
// Console demo main()
// ============================================================================

int main() {
    using namespace groot::dynamic;

    printHeader();
    DemoEnvironment env;
    DynamicChallengeManager mgr(env);
    mgr.setChallenges(defaultChallenges());

    mgr.onStart([&](const ChallengeDef& d) {
        std::cout << "[START] " << d.displayName << " — " << d.description << "\n";
        std::cout << "        type=" << toString(d.type)
                  << ", intensity=" << d.intensity
                  << ", duration=" << d.duration << "s\n";
        std::cout << "        (meteors=" << env.activeMeteors
                  << ", vines=" << env.activeVines
                  << ", gravityScale=" << env.gravityScale
                  << ", resourceX=" << env.resourceMultiplier << ")\n";
    });

    mgr.onTick([&](const ChallengeDef& d, float p) {
        if (static_cast<int>(p * 10) % 2 == 0) { // coarse throttle for console
            std::cout << "  progress: " << std::fixed << std::setprecision(0) << (p * 100.f) << "%\r" << std::flush;
        }
    });

    mgr.onEnd([&](const ChallengeDef& d) {
        std::cout << "\n[END]   " << d.displayName << "\n";
        std::cout << "        (meteors=" << env.activeMeteors
                  << ", vines=" << env.activeVines
                  << ", gravityScale=" << env.gravityScale
                  << ", resourceX=" << env.resourceMultiplier << ")\n";
        // snapshot demo entities
        std::cout << "        Entities HP:";
        for (const auto& e : env.entities) std::cout << ' ' << std::setw(3) << static_cast<int>(e.health);
        std::cout << "\n";
    });

    mgr.startSystem();

    // Demo simulation loop (~60 seconds)
    const float dt = 0.1f; // 10 Hz for console neatness
    float simTime = 0.f;
    const float maxTime = 60.f;

    while (simTime < maxTime) {
        mgr.update(dt);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        simTime += dt;

        // showcase: trigger a specific challenge at 20s
        if (std::abs(simTime - 20.f) < 0.001f) {
            std::cout << "\n[DEBUG] Forcing LowGravity now.\n";
            mgr.triggerNow(ChallengeType::LowGravity);
        }
    }

    std::cout << "\nDemo finished. Integrate by wiring IEnvironment to your game systems (physics, FX, spawners).\n";
    return 0;
}
