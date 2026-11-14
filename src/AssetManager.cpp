// Owner: E3
#include "AssetManager.h"
#include <stdexcept>

AssetManager& AssetManager::GetInstance() {
    static AssetManager instance;
    return instance;
}

void AssetManager::LoadAllAssets(const std::vector<std::string>& modelKeys,
                                 const std::vector<std::string>& iconKeys) {
    UnloadAllAssets();   // safe to call twice
    try {
        for (const std::string& key : modelKeys) {
            if (m_models.count(key)) continue;
            const std::string path = "assets/models/" + key + ".obj";
            Model model = LoadModel(path.c_str());
            if (!IsModelReady(model)) throw std::runtime_error("Missing or invalid model file: " + path);
            m_models[key] = model;
        }
        for (const std::string& key : iconKeys) {
            if (m_icons.count(key)) continue;
            const std::string path = "assets/icons/" + key + ".png";
            Texture2D tex = LoadTexture(path.c_str());
            if (!IsTextureReady(tex)) throw std::runtime_error("Missing or invalid icon file: " + path);
            SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
            m_icons[key] = tex;
        }
        const std::string fontPath = "assets/fonts/JetBrainsMono-Regular.ttf";
        m_uiFont = LoadFontEx(fontPath.c_str(), 24, nullptr, 0);
        if (!IsFontReady(m_uiFont)) throw std::runtime_error("Missing or invalid font file: " + fontPath);
        SetTextureFilter(m_uiFont.texture, TEXTURE_FILTER_BILINEAR);
        m_fontLoaded = true;
    } catch (...) {
        UnloadAllAssets();   // never leave half-loaded state behind
        throw;
    }
}

Model& AssetManager::GetModel(const std::string& modelKey) {
    auto it = m_models.find(modelKey);
    if (it == m_models.end()) throw std::runtime_error("Model not loaded: " + modelKey);
    return it->second;
}

Texture2D& AssetManager::GetIcon(const std::string& iconKey) {
    auto it = m_icons.find(iconKey);
    if (it == m_icons.end()) throw std::runtime_error("Icon not loaded: " + iconKey);
    return it->second;
}

Font& AssetManager::GetUIFont() {
    if (!m_fontLoaded) throw std::runtime_error("UI font not loaded");
    return m_uiFont;
}

void AssetManager::UnloadAllAssets() {
    for (auto& kv : m_models) UnloadModel(kv.second);
    for (auto& kv : m_icons) UnloadTexture(kv.second);
    if (m_fontLoaded) UnloadFont(m_uiFont);
    m_models.clear();
    m_icons.clear();
    m_uiFont = Font{};
    m_fontLoaded = false;
}
