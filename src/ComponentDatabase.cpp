// Owner: E1
#include "ComponentDatabase.h"
#include <fstream>
#include <stdexcept>
#include "DerivedComponents.h"
#include "third_party/json.hpp"

using json = nlohmann::json;

namespace {

CraftStats ReadStats(const json& p) {
    const json& s = p.at("stats");
    CraftStats c;
    c.health = s.at("health").get<float>();
    c.strength = s.at("strength").get<float>();
    c.thrust = s.at("thrust").get<float>();
    c.shielding = s.at("shielding").get<float>();
    c.mass = p.at("mass_kg").get<float>();
    c.totalCost = p.at("cost_usd").get<float>();
    c.totalBuildTimeSec = p.at("build_time_sec").get<float>();
    return c;
}

ComponentSlot ParseSlot(const std::string& s) {
    if (s == "MOBILITY") return ComponentSlot::MOBILITY;
    if (s == "OPTICS") return ComponentSlot::OPTICS;
    if (s == "PAYLOAD") return ComponentSlot::PAYLOAD;
    if (s == "POWER") return ComponentSlot::POWER;
    if (s == "SHIELDING") return ComponentSlot::SHIELDING;
    throw std::runtime_error("unknown slot '" + s + "'");
}

MissionType ParseMissionType(const std::string& s) {
    if (s == "FLYBY_RECON") return MissionType::FLYBY_RECON;
    if (s == "ORBITAL_SAMPLE") return MissionType::ORBITAL_SAMPLE;
    if (s == "SURFACE_CORE_DRILL") return MissionType::SURFACE_CORE_DRILL;
    throw std::runtime_error("unknown supported_mission_type '" + s + "'");
}

void ExpectSlot(const std::string& cls, ComponentSlot got, ComponentSlot want) {
    if (got != want) throw std::runtime_error("class " + cls + " cannot use a different slot");
}

std::unique_ptr<ShipComponent> BuildPart(const json& p) {
    std::string cls = p.at("class").get<std::string>();
    std::string id = p.at("id").get<std::string>();
    std::string name = p.at("name").get<std::string>();
    ComponentSlot slot = ParseSlot(p.at("slot").get<std::string>());
    int tier = p.at("tier").get<int>();
    if (tier < 1 || tier > 3) throw std::runtime_error("tier must be 1..3");
    CraftStats stats = ReadStats(p);
    std::string modelKey = p.at("model_key").get<std::string>();
    std::string iconKey = p.at("icon_key").get<std::string>();
    std::string desc = p.at("description").get<std::string>();

    if (cls == "MobilityComponent") {
        ExpectSlot(cls, slot, ComponentSlot::MOBILITY);
        return std::make_unique<MobilityComponent>(id, name, tier, stats, modelKey, iconKey, desc,
                                                   p.at("can_land_on_surface").get<bool>());
    }
    if (cls == "OpticsComponent") {
        ExpectSlot(cls, slot, ComponentSlot::OPTICS);
        return std::make_unique<OpticsComponent>(id, name, tier, stats, modelKey, iconKey, desc,
                                                 p.at("optical_resolution_rating").get<int>());
    }
    if (cls == "PayloadComponent") {
        ExpectSlot(cls, slot, ComponentSlot::PAYLOAD);
        return std::make_unique<PayloadComponent>(
            id, name, tier, stats, modelKey, iconKey, desc,
            ParseMissionType(p.at("supported_mission_type").get<std::string>()));
    }
    if (cls == "PowerComponent") {
        ExpectSlot(cls, slot, ComponentSlot::POWER);
        return std::make_unique<PowerComponent>(id, name, tier, stats, modelKey, iconKey, desc,
                                                p.at("power_output_watts").get<float>());
    }
    if (cls == "ShieldingComponent") {
        ExpectSlot(cls, slot, ComponentSlot::SHIELDING);
        return std::make_unique<ShieldingComponent>(id, name, tier, stats, modelKey, iconKey, desc,
                                                    p.at("max_thermal_tolerance_celsius").get<float>());
    }
    throw std::runtime_error("unknown class '" + cls + "'");
}

}  // namespace

void ComponentDatabase::LoadFromJson(const std::string& partsPath) {
    std::ifstream in(partsPath);
    if (!in) throw std::runtime_error("Cannot open parts catalog: " + partsPath);

    json root;
    try {
        root = json::parse(in);
    } catch (const std::exception& e) {
        throw std::runtime_error("Malformed parts catalog '" + partsPath + "': " + e.what());
    }

    // Build into locals first so a failed load never leaves partial data behind.
    std::vector<std::unique_ptr<ShipComponent>> parts;
    std::unordered_map<std::string, ShipComponent*> byId;
    CraftStats base;
    std::string baseKey;
    std::string where = "<header>";
    try {
        if (root.contains("schema_version") && root.at("schema_version").get<int>() != 2)
            throw std::runtime_error("unsupported schema_version (expected 2)");
        const json& chassis = root.at("base_chassis");
        base = ReadStats(chassis);
        baseKey = chassis.at("model_key").get<std::string>();
        const json& list = root.at("parts");
        if (!list.is_array()) throw std::runtime_error("'parts' must be an array");
        for (const json& p : list) {
            where = p.contains("id") ? p.at("id").get<std::string>() : "<no id>";
            auto part = BuildPart(p);
            if (byId.count(part->GetId())) throw std::runtime_error("duplicate id '" + part->GetId() + "'");
            byId[part->GetId()] = part.get();
            parts.push_back(std::move(part));
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("parts catalog '" + partsPath + "' (part " + where + "): " + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error("parts catalog '" + partsPath + "' (part " + where +
                                 ") missing or invalid key: " + e.what());
    }
    m_parts = std::move(parts);
    m_byId = std::move(byId);
    m_baseChassisStats = base;
    m_baseChassisModelKey = baseKey;
}

ShipComponent* ComponentDatabase::FindById(const std::string& id) const {
    auto it = m_byId.find(id);
    return it == m_byId.end() ? nullptr : it->second;
}

std::vector<ShipComponent*> ComponentDatabase::GetPartsBySlot(ComponentSlot slot) const {
    std::vector<ShipComponent*> out;
    for (const auto& p : m_parts)
        if (p->GetSlotType() == slot) out.push_back(p.get());
    return out;
}

std::vector<ShipComponent*> ComponentDatabase::GetAllParts() const {
    std::vector<ShipComponent*> out;
    out.reserve(m_parts.size());
    for (const auto& p : m_parts) out.push_back(p.get());
    return out;
}

const CraftStats& ComponentDatabase::GetBaseChassisStats() const { return m_baseChassisStats; }
const std::string& ComponentDatabase::GetBaseChassisModelKey() const { return m_baseChassisModelKey; }
