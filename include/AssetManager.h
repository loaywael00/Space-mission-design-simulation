#pragma once
// Owner: E3. Frozen contract (Section 3.11).
#include <string>
#include <unordered_map>
#include <vector>
#include "raylib.h"

class AssetManager {
public:
    static AssetManager& GetInstance();
    // modelKeys: e.g. "base_chassis", "mobility_t1_light_orbital_skids", ...
    // iconKeys : same strings as modelKeys (icons share the same key)
    // Throws std::runtime_error naming any missing file.
    void LoadAllAssets(const std::vector<std::string>& modelKeys,
                       const std::vector<std::string>& iconKeys);
    Model& GetModel(const std::string& modelKey);
    Texture2D& GetIcon(const std::string& iconKey);
    Font& GetUIFont();
    void UnloadAllAssets();
private:
    AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::unordered_map<std::string, Model> m_models;
    std::unordered_map<std::string, Texture2D> m_icons;
    Font m_uiFont {};
    bool m_fontLoaded = false;
};
