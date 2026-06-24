#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

using 札内容 = std::unordered_map<std::string, std::string>;

struct 票 {
    std::string 札番号;
    int 版 = 0;
    std::string 種別;
    std::string 状態;
    std::string 日付;
    std::string 部署;
    std::string 担当;
    std::string 見出し;
    int 差戻し回数 = 0;
    札内容 内容;
};

struct 監査結果札 {
    std::string 対象札番号;
    int 対象版 = 0;
    std::string 対象者;
    std::string 判定;
    std::string 問題番号;
    std::string 差戻し先;
    std::string 再処理指示番号;
    std::string 監査時刻;
};

struct 目録項目 {
    std::string 札番号;
    int 版 = 0;
    std::string 日付;
    std::string 部署;
    std::string 状態;
    std::string 見出し;
    std::string 担当;
    std::string 判定;
    std::string 最終更新;
};

class 共通関数 {
public:
    static std::string 現在時刻() {
        using clock = std::chrono::system_clock;
        auto now = clock::now();
        auto now_time = clock::to_time_t(now);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now_time);
#else
        localtime_r(&now_time, &local);
#endif
        std::ostringstream os;
        os << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
        return os.str();
    }

    static std::string 日付文字列() {
        using clock = std::chrono::system_clock;
        auto now = clock::now();
        auto now_time = clock::to_time_t(now);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now_time);
#else
        localtime_r(&now_time, &local);
#endif
        std::ostringstream os;
        os << std::put_time(&local, "%Y%m%d");
        return os.str();
    }

    static std::string 文字列整形(std::string 文字列) {
        std::string 出力;
        for (char c : 文字列) {
            if (c == '\r' || c == '\n') {
                出力.push_back(' ');
            } else if (c == '\t') {
                出力.push_back('_');
            } else {
                出力.push_back(c);
            }
        }
        return 出力;
    }

    static std::string 文字列エスケープ(const std::string& 文字列) {
        std::string 結果;
        結果.reserve(文字列.size() + 8);
        for (char c : 文字列) {
            if (c == '"') {
                結果 += "\\\"";
            } else if (c == '\\') {
                結果 += "\\\\";
            } else if (c == '\n') {
                結果 += "\\n";
            } else {
                結果 += c;
            }
        }
        return 結果;
    }

    static bool 含む(const std::string& 文字列, const std::string& 検索対象) {
        return 文字列.find(検索対象) != std::string::npos;
    }

    static std::string 見出し要約(const std::string& 本文, size_t 上限 = 24) {
        if (本文.size() <= 上限) {
            return 本文;
        }
        return 本文.substr(0, 上限) + "…";
    }

    static std::string 値取得(const std::unordered_map<std::string, std::string>& マップ, const std::string& キー) {
        auto it = マップ.find(キー);
        if (it == マップ.end()) {
            return {};
        }
        return it->second;
    }

    static std::string 値取得既定(const std::unordered_map<std::string, std::string>& マップ, const std::string& キー, const std::string& 既定値) {
        auto it = マップ.find(キー);
        if (it == マップ.end()) {
            return 既定値;
        }
        return it->second;
    }
};

class 量子圧縮器 {
public:
    static std::vector<uint8_t> 文字列Toバイト(const std::string& 入力) {
        return std::vector<uint8_t>(入力.begin(), 入力.end());
    }

    static std::string バイトTo文字列(const std::vector<uint8_t>& 入力) {
        return std::string(入力.begin(), 入力.end());
    }

    static std::vector<uint8_t> 圧縮NIBBLE(const std::string& json文字列) {
        auto bytes = 文字列Toバイト(json文字列);
        std::vector<uint8_t> nibble;
        for (size_t i = 0; i < bytes.size();) {
            uint8_t b = bytes[i];
            uint8_t run = 1;
            while ((i + run) < bytes.size() && run < 15 && bytes[i + run] == b) {
                ++run;
            }
            if (run >= 4) {
                nibble.push_back(0xF);
                nibble.push_back(run - 1);
                nibble.push_back(b >> 4);
                nibble.push_back(b & 0x0F);
                i += run;
            } else {
                for (uint8_t k = 0; k < run; ++k) {
                    nibble.push_back(0x0);
                    nibble.push_back(b >> 4);
                    nibble.push_back(b & 0x0F);
                }
                i += run;
            }
        }

        std::vector<uint8_t> packed;
        packed.reserve((nibble.size() + 1) / 2);
        for (size_t i = 0; i < nibble.size(); i += 2) {
            uint8_t hi = nibble[i];
            uint8_t lo = (i + 1 < nibble.size()) ? nibble[i + 1] : 0;
            packed.push_back(static_cast<uint8_t>((hi << 4) | (lo & 0x0F)));
        }

        packed.insert(packed.begin(), static_cast<uint8_t>(nibble.size() >> 8));
        packed.insert(packed.begin(), static_cast<uint8_t>(nibble.size() & 0xFF));
        return packed;
    }

    static std::string 復元NIBBLE(const std::vector<uint8_t>& 入力) {
        if (入力.size() < 2) {
            return {};
        }

        size_t nibble数 = (static_cast<size_t>(入力[0]) << 8) | 入力[1];
        std::vector<uint8_t> nibble;
        nibble.reserve(nibble数);

        for (size_t i = 2; i < 入力.size(); ++i) {
            uint8_t b = 入力[i];
            nibble.push_back(b >> 4);
            nibble.push_back(b & 0x0F);
        }

        if (nibble.size() > nibble数) {
            nibble.resize(nibble数);
        }

        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < nibble.size();) {
            uint8_t token = nibble[i++];
            if (token == 0x0) {
                if (i + 2 > nibble.size()) {
                    return {};
                }
                uint8_t hi = nibble[i++];
                uint8_t lo = nibble[i++];
                bytes.push_back(static_cast<uint8_t>((hi << 4) | lo));
            } else if (token == 0xF) {
                if (i + 3 > nibble.size()) {
                    return {};
                }
                uint8_t run = nibble[i++] + 1;
                uint8_t hi = nibble[i++];
                uint8_t lo = nibble[i++];
                uint8_t b = static_cast<uint8_t>((hi << 4) | lo);
                for (uint8_t r = 0; r < run; ++r) {
                    bytes.push_back(b);
                }
            } else {
                return {};
            }
        }
        return バイトTo文字列(bytes);
    }

    static bool 保存(const fs::path& path, const std::string& json文字列) {
        auto packed = 圧縮NIBBLE(json文字列);
        fs::create_directories(path.parent_path());
        std::ofstream 出力(path, std::ios::binary | std::ios::trunc);
        if (!出力.is_open()) {
            return false;
        }

        const char magic[4] = {'Q', '4', 'C', '1'};
        const uint32_t dataLen = static_cast<uint32_t>(packed.size());

        出力.write(magic, 4);
        出力.write(reinterpret_cast<const char*>(&dataLen), sizeof(dataLen));
        if (!packed.empty()) {
            出力.write(reinterpret_cast<const char*>(packed.data()), packed.size());
        }
        return 出力.good();
    }

    static bool 読込(const fs::path& path, std::string& json文字列) {
        std::ifstream 入力(path, std::ios::binary);
        if (!入力.is_open()) {
            return false;
        }

        char magic[4];
        入力.read(magic, 4);
        if (!入力 || magic[0] != 'Q' || magic[1] != '4' || magic[2] != 'C' || magic[3] != '1') {
            return false;
        }

        uint32_t dataLen = 0;
        入力.read(reinterpret_cast<char*>(&dataLen), sizeof(dataLen));
        std::vector<uint8_t> packed(dataLen);
        if (dataLen > 0) {
            入力.read(reinterpret_cast<char*>(packed.data()), dataLen);
            if (!入力) {
                return false;
            }
        }

        json文字列 = 復元NIBBLE(packed);
        return !json文字列.empty();
    }
};

class 目録 {
public:
    explicit 目録(const fs::path& パス) : 目録ファイル(パス) {
        読込();
    }

    void 追加(const 目録項目& 項目) {
        全件リスト.push_back(項目);
        書込();
    }

    int 取得最新版版数(const std::string& 札番号) const {
        int 最大値 = 0;
        for (const auto& e : 全件リスト) {
            if (e.札番号 == 札番号 && e.版 > 最大値) {
                最大値 = e.版;
            }
        }
        return 最大値;
    }

    std::vector<目録項目> 検索(const std::string& 札番号) const {
        std::vector<目録項目> 結果;
        for (const auto& e : 全件リスト) {
            if (e.札番号 == 札番号) {
                結果.push_back(e);
            }
        }
        return 結果;
    }

    const std::vector<目録項目>& 全件() const {
        return 全件リスト;
    }

private:
    fs::path 目録ファイル;
    std::vector<目録項目> 全件リスト;

    void 読込() {
        全件リスト.clear();
        std::ifstream 入力(目録ファイル);
        if (!入力.is_open()) {
            return;
        }

        std::string 行;
        while (std::getline(入力, 行)) {
            if (行.empty()) {
                continue;
            }
            auto p = 行分解(行);
            if (p.size() < 9) {
                continue;
            }

            目録項目 e;
            e.札番号 = p[0];
            e.版 = std::stoi(p[1]);
            e.日付 = p[2];
            e.部署 = p[3];
            e.状態 = p[4];
            e.見出し = p[5];
            e.担当 = p[6];
            e.判定 = p[7];
            e.最終更新 = p[8];
            全件リスト.push_back(std::move(e));
        }
    }

    void 書込() const {
        fs::create_directories(目録ファイル.parent_path());
        std::ofstream 出力(目録ファイル, std::ios::trunc);
        if (!出力.is_open()) {
            return;
        }
        for (const auto& e : 全件リスト) {
            出力 << タブ結合({
                              e.札番号,
                              std::to_string(e.版),
                              e.日付,
                              e.部署,
                              e.状態,
                              共通関数::文字列整形(e.見出し),
                              e.担当,
                              e.判定,
                              e.最終更新})
                   << '\n';
        }
    }

    static std::vector<std::string> 行分解(const std::string& 行) {
        std::vector<std::string> 要素;
        std::string 現在;
        for (char c : 行) {
            if (c == '\t') {
                要素.push_back(現在);
                現在.clear();
            } else {
                現在.push_back(c);
            }
        }
        要素.push_back(現在);
        return 要素;
    }

    static std::string タブ結合(const std::vector<std::string>& 要素) {
        std::string 出力;
        for (size_t i = 0; i < 要素.size(); ++i) {
            if (i > 0) {
                出力.push_back('\t');
            }
            出力 += 要素[i];
        }
        return 出力;
    }
};

class 書類倉庫 {
public:
    explicit 書類倉庫(const fs::path& ルート = fs::path("書類倉庫")) : 保管庫ルート(ルート) {
        fs::create_directories(保管庫ルート);
        目録管理 = std::make_unique<目録>(保管庫ルート / "目録.tsv");
    }

    int 保存票(const 票& 基票) {
        票 票本体 = 基票;
        票本体.版 = 目録管理->取得最新版版数(票本体.札番号) + 1;

        auto json文字列 = 票ToJSON(票本体);
        fs::path 保存先 = 保管庫ルート / (票本体.札番号 + "_" + 票本体.種別 + "_v" + std::to_string(票本体.版) + ".q4");
        量子圧縮器::保存(保存先, json文字列);

        目録項目 行;
        行.札番号 = 票本体.札番号;
        行.版 = 票本体.版;
        行.日付 = 票本体.日付;
        行.部署 = 票本体.部署;
        行.状態 = 票本体.状態;
        行.見出し = 票本体.見出し;
        行.担当 = 票本体.担当;
        行.判定 = 票本体.内容.count("判定") ? 票本体.内容.at("判定") : "未監査";
        行.最終更新 = 共通関数::現在時刻();
        目録管理->追加(行);
        return 票本体.版;
    }

    int 保存監査結果(const 監査結果札& 監査票) {
        票 票本体;
        票本体.札番号 = 監査票.対象札番号;
        票本体.種別 = "監査結果札";
        票本体.版 = 目録管理->取得最新版版数(票本体.札番号) + 1;
        票本体.状態 = 監査対象状態(監査票.判定);
        票本体.日付 = 共通関数::現在時刻();
        票本体.部署 = "監査部門AI";
        票本体.担当 = 監査票.対象者;
        票本体.見出し = "監査結果札";
        票本体.差戻し回数 = 0;
        票本体.内容["対象札番号"] = 監査票.対象札番号;
        票本体.内容["対象版"] = std::to_string(監査票.対象版);
        票本体.内容["対象者"] = 監査票.対象者;
        票本体.内容["判定"] = 監査票.判定;
        票本体.内容["問題番号"] = 監査票.問題番号;
        票本体.内容["差戻し先"] = 監査票.差戻し先;
        票本体.内容["再処理指示番号"] = 監査票.再処理指示番号;
        票本体.内容["監査時刻"] = 監査票.監査時刻;

        fs::path 保存先 = 保管庫ルート / (票本体.札番号 + "_監査結果札_v" + std::to_string(票本体.版) + ".q4");
        量子圧縮器::保存(保存先, 監査結果ToJSON(監査票));

        目録項目 行;
        行.札番号 = 票本体.札番号;
        行.版 = 票本体.版;
        行.日付 = 票本体.日付;
        行.部署 = 票本体.部署;
        行.状態 = 票本体.状態;
        行.見出し = 票本体.見出し;
        行.担当 = 票本体.担当;
        行.判定 = 監査票.判定;
        行.最終更新 = 監査票.監査時刻;
        目録管理->追加(行);
        return 票本体.版;
    }

    const std::vector<目録項目>& 目録一覧() const {
        return 目録管理->全件();
    }

private:
    fs::path 保管庫ルート;
    std::unique_ptr<目録> 目録管理;

    static std::string 監査対象状態(const std::string& 判定) {
        if (判定 == "可") {
            return "最終監査待ち";
        }
        if (判定 == "保") {
            return "処理待ち監査待ち";
        }
        return "差戻し";
    }

    static std::string 票ToJSON(const 票& 票本体) {
        std::ostringstream os;
        os << "{";
        os << "\"札番号\":\"" << 共通関数::文字列エスケープ(票本体.札番号) << "\",";
        os << "\"版\":" << 票本体.版 << ",";
        os << "\"種別\":\"" << 共通関数::文字列エスケープ(票本体.種別) << "\",";
        os << "\"状態\":\"" << 共通関数::文字列エスケープ(票本体.状態) << "\",";
        os << "\"日付\":\"" << 共通関数::文字列エスケープ(票本体.日付) << "\",";
        os << "\"部署\":\"" << 共通関数::文字列エスケープ(票本体.部署) << "\",";
        os << "\"担当\":\"" << 共通関数::文字列エスケープ(票本体.担当) << "\",";
        os << "\"見出し\":\"" << 共通関数::文字列エスケープ(票本体.見出し) << "\",";
        os << "\"差戻し回数\":" << 票本体.差戻し回数 << ",";
        os << "\"内容\":{";
        bool 最初 = true;
        for (auto it = 票本体.内容.begin(); it != 票本体.内容.end(); ++it) {
            if (!最初) {
                os << ",";
            }
            最初 = false;
            os << "\"" << 共通関数::文字列エスケープ(it->first) << "\":\"";
            os << 共通関数::文字列エスケープ(it->second) << "\"";
        }
        os << "}}";
        return os.str();
    }

    static std::string 監査結果ToJSON(const 監査結果札& 監査票) {
        std::ostringstream os;
        os << "{";
        os << "\"対象札番号\":\"" << 共通関数::文字列エスケープ(監査票.対象札番号) << "\",";
        os << "\"対象版\":" << 監査票.対象版 << ",";
        os << "\"対象者\":\"" << 共通関数::文字列エスケープ(監査票.対象者) << "\",";
        os << "\"判定\":\"" << 共通関数::文字列エスケープ(監査票.判定) << "\",";
        os << "\"問題番号\":\"" << 共通関数::文字列エスケープ(監査票.問題番号) << "\",";
        os << "\"差戻し先\":\"" << 共通関数::文字列エスケープ(監査票.差戻し先) << "\",";
        os << "\"再処理指示番号\":\"" << 共通関数::文字列エスケープ(監査票.再処理指示番号) << "\",";
        os << "\"監査時刻\":\"" << 共通関数::文字列エスケープ(監査票.監査時刻) << "\"";
        os << "}";
        return os.str();
    }
};

struct 処理待ち札 {
    std::string 札番号;
    int 版 = 0;
    int 優先度 = 1;
};

class 処理待ちブース {
public:
    void 投入(const 処理待ち札& 札項目) {
        待機列.push_back(札項目);
    }

    bool 空() const {
        return 待機列.empty();
    }

    処理待ち札 取り出し() {
        std::sort(待機列.begin(), 待機列.end(),
            [](const 処理待ち札& A, const 処理待ち札& B) {
                if (A.優先度 != B.優先度) {
                    return A.優先度 > B.優先度;
                }
                return A.版 < B.版;
            });
        処理待ち札 先頭 = 待機列.front();
        待機列.erase(待機列.begin());
        return 先頭;
    }

private:
    std::vector<処理待ち札> 待機列;
};

class 孔明くん {
public:
    票 熟考生成(const std::string& 依頼札番号, const std::string& 主人依頼, int 差戻し回数) {
        票 t;
        t.札番号 = 依頼札番号;
        t.種別 = "熟考札";
        t.状態 = "熟考済";
        t.日付 = 共通関数::現在時刻();
        t.部署 = 判断部門(主人依頼);
        t.担当 = "孔明くん";
        t.見出し = "熟考札";
        t.差戻し回数 = 差戻し回数;
        t.内容["主人依頼"] = 主人依頼;
        t.内容["担当部署"] = t.部署;
        t.内容["熟考要約"] = 共通関数::見出し要約(主人依頼, 40);
        t.内容["熟考方針"] = "依頼主の要件を部署責務で処理可能な形へ分解します。";
        return t;
    }

    票 業務連絡作成(const 票& 熟考札, int 優先度) {
        票 t = 熟考札;
        t.種別 = "業務連絡札";
        t.状態 = "処理待ち監査待ち";
        t.見出し = "業務連絡札";
        t.内容["指示"] = "担当部署へ実行依頼: " + 共同抽出(熟考札.内容.at("熟考要約"));
        t.内容["優先度"] = std::to_string(優先度);
        t.内容["依頼要約"] = 共通関数::値取得(熟考札.内容, "熟考要約");
        return t;
    }

    票 最終報告作成(const std::string& 依頼札番号, const std::string& 処理内容, int 差戻し回数) {
        票 t;
        t.札番号 = 依頼札番号;
        t.種別 = "最終報告札";
        t.状態 = "孔明くん報告待ち";
        t.日付 = 共通関数::現在時刻();
        t.部署 = "全社";
        t.担当 = "孔明くん";
        t.見出し = "最終報告札";
        t.差戻し回数 = 差戻し回数;
        t.内容["最終報告"] = 処理内容;
        t.内容["要旨"] = "主人向け報告準備を完了します。";
        return t;
    }

    票 主人確認札(const std::string& 依頼札番号, int 差戻し回数, const std::string& 理由) {
        票 t;
        t.札番号 = 依頼札番号;
        t.種別 = "主人確認札";
        t.状態 = "孔明くん報告待ち";
        t.日付 = 共通関数::現在時刻();
        t.部署 = "全社";
        t.担当 = "孔明くん";
        t.見出し = "主人確認札";
        t.差戻し回数 = 差戻し回数;
        t.内容["要旨"] = 理由;
        return t;
    }

    票 孔明くん返却札(const std::string& 依頼札番号, int 差戻し回数, const std::string& 理由) {
        票 t;
        t.札番号 = 依頼札番号;
        t.種別 = "孔明くん返却札";
        t.状態 = "孔明くん報告待ち";
        t.日付 = 共通関数::現在時刻();
        t.部署 = "全社";
        t.担当 = "孔明くん";
        t.見出し = "孔明くん返却札";
        t.差戻し回数 = 差戻し回数;
        t.内容["要旨"] = 理由;
        t.内容["判定"] = "可";
        return t;
    }

private:
    static std::string 共同抽出(const std::string& 元文) {
        return 共通関数::見出し要約(元文, 46);
    }

    static std::string 判断部門(const std::string& 依頼) {
        if (共通関数::含む(依頼, "契約") || 共通関数::含む(依頼, "規程") || 共通関数::含む(依頼, "法務")) return "法務部";
        if (共通関数::含む(依頼, "請求") || 共通関数::含む(依頼, "経費") || 共通関数::含む(依頼, "予算")) return "経理部";
        if (共通関数::含む(依頼, "不具合") || 共通関数::含む(依頼, "開発") || 共通関数::含む(依頼, "実装") || 共通関数::含む(依頼, "バグ")) return "開発部";
        if (共通関数::含む(依頼, "採用") || 共通関数::含む(依頼, "評価") || 共通関数::含む(依頼, "人事")) return "人事部";
        return "総務部";
    }
};

struct 社員AI定義 {
    std::string 部署;
    std::string 氏名;
    std::string 役目;
    std::string 責務;

    std::string 処理(const std::string& 主人依頼, const std::string& 指示) const {
        std::ostringstream os;
        os << "役目:" << 役目 << "\n";
        os << "責務:" << 責務 << "\n";
        os << "指示:" << 指示 << "\n";
        os << "根拠:" << 主人依頼;
        return os.str();
    }
};

class 部長AI {
public:
    部長AI() {
        員工表["総務部"].push_back({"総務部", "山田", "受付調整", "社内連絡の一次整理と調整"});
        員工表["開発部"].push_back({"開発部", "佐々木", "要件整形", "仕様の要件化と実作業方針化"});
        員工表["経理部"].push_back({"経理部", "井上", "審査補助", "予算感覚と記載整合性確認"});
        員工表["法務部"].push_back({"法務部", "鈴木", "規程確認", "外部依存のない規程適合チェック"});
        員工表["人事部"].push_back({"人事部", "高橋", "制度整備", "手続・基準の確認"});
    }

    int 優先度判定(const 票& 業務連絡) const {
        if (業務連絡.内容.count("優先度") > 0) {
            return std::stoi(業務連絡.内容.at("優先度"));
        }
        if (共通関数::含む(共通関数::値取得(業務連絡.内容, "依頼要約"), "緊急")) {
            return 3;
        }
        if (共通関数::含む(共通関数::値取得(業務連絡.内容, "依頼要約"), "重要")) {
            return 2;
        }
        return 1;
    }

    std::pair<bool, 社員AI定義> 割当(const 票& 業務連絡) const {
        auto it = 員工表.find(業務連絡.部署);
        if (it == 員工表.end() || it->second.empty()) {
            return {false, 社員AI定義()};
        }
        return {true, it->second.front()};
    }

    票 割当札作成(const 票& 業務連絡, const 社員AI定義& 社員, int 差戻し回数) const {
        票 t = 業務連絡;
        t.種別 = "割当札";
        t.状態 = "部長確認中";
        t.見出し = "割当札";
        t.担当 = 社員.氏名;
        t.内容["担当社員"] = 社員.氏名;
        t.内容["担当部署"] = 社員.部署;
        t.内容["担当役目"] = 社員.役目;
        t.内容["担当責務"] = 社員.責務;
        t.差戻し回数 = 差戻し回数;
        return t;
    }

    票 部署報告札作成(const 票& 処理後, int 差戻し回数) const {
        票 t = 処理後;
        t.種別 = "部署報告札";
        t.状態 = "部署報告済";
        t.見出し = "部署報告札";
        t.内容["部署報告"] = "社員AIによる処理を受領し、処理完了を確認しました。";
        t.差戻し回数 = 差戻し回数;
        return t;
    }

private:
    std::unordered_map<std::string, std::vector<社員AI定義>> 員工表;
};

class 監査部門AI {
public:
    監査結果札 監査実行(const 票& 票本体, const std::string& 対象者) {
        監査結果札 結果;
        結果.対象札番号 = 票本体.札番号;
        結果.対象版 = 票本体.版;
        結果.対象者 = 対象者;
        結果.監査時刻 = 共通関数::現在時刻();

        auto [問題番号, 判定] = 判定ロジック(票本体, 対象者);
        結果.問題番号 = 問題番号;
        結果.判定 = 判定;
        結果.再処理指示番号 = 再処理指示番号決定(票本体.種別, 問題番号);
        結果.差戻し先 = 差戻し先決定(票本体.種別, 問題番号);
        if (判定 == "可") {
            結果.問題番号 = "";
        }
        return 結果;
    }

private:
    static std::pair<std::string, std::string> 判定ロジック(const 票& 票本体, const std::string&) {
        const auto ng = [](const std::string& 文) {
            return 共通関数::含む(文, "外部送信") || 共通関数::含む(文, "メール") || 共通関数::含む(文, "カレンダー");
        };

        if (票本体.種別 == "依頼札") {
            if (!票本体.内容.count("主人依頼") || 票本体.内容.at("主人依頼").empty()) {
                return {"A07", "否"};
            }
            if (ng(票本体.内容.at("主人依頼"))) {
                return {"A12", "否"};
            }
            return {"", "可"};
        }

        if (票本体.種別 == "熟考札") {
            if (!票本体.内容.count("担当部署") || 票本体.内容.at("担当部署").empty()) {
                return {"A03", "否"};
            }
            if (!票本体.内容.count("熟考要約") || 票本体.内容.at("熟考要約").empty()) {
                return {"A07", "保"};
            }
            return {"", "可"};
        }

        if (票本体.種別 == "業務連絡札") {
            if (!票本体.内容.count("指示") || 票本体.内容.at("指示").empty()) {
                return {"A02", "否"};
            }
            if (票本体.部署.empty()) {
                return {"A03", "否"};
            }
            if (ng(票本体.内容.at("指示"))) {
                return {"A12", "否"};
            }
            return {"", "可"};
        }

        if (票本体.種別 == "割当札") {
            if (!票本体.内容.count("担当社員") || 票本体.内容.at("担当社員").empty()) {
                return {"A04", "否"};
            }
            return {"", "可"};
        }

        if (票本体.種別 == "処理後札") {
            if (!票本体.内容.count("処理成果") || 票本体.内容.at("処理成果").empty()) {
                return {"A08", "否"};
            }
            if (!票本体.内容.count("根拠") || 票本体.内容.at("根拠").empty()) {
                return {"A09", "否"};
            }
            if (共通関数::含む(票本体.内容.at("処理成果"), "追加仕様") || 共通関数::含む(票本体.内容.at("処理成果"), "勝手に")) {
                return {"A11", "保"};
            }
            return {"", "可"};
        }

        if (票本体.種別 == "部署報告札") {
            if (!票本体.内容.count("部署報告") || 票本体.内容.at("部署報告").empty()) {
                return {"A02", "保"};
            }
            return {"", "可"};
        }

        if (票本体.種別 == "最終報告札") {
            if (!票本体.内容.count("最終報告") || 票本体.内容.at("最終報告").empty()) {
                return {"A08", "否"};
            }
            return {"", "可"};
        }

        if (票本体.種別 == "監査結果札") {
            return {"", "可"};
        }

        if (票本体.種別 == "孔明くん返却札" || 票本体.種別 == "主人確認札") {
            return {"A06", "保"};
        }

        return {"", "可"};
    }

    static std::string 再処理指示番号決定(const std::string& 種別, const std::string& 問題番号) {
        if (問題番号 == "A11") {
            return "R07";
        }
        if (問題番号 == "A10") {
            return "R08";
        }
        if (種別 == "熟考札" || 種別 == "業務連絡札") {
            return "R01";
        }
        if (種別 == "割当札") {
            return "R03";
        }
        if (種別 == "処理後札") {
            if (問題番号 == "A08" || 問題番号 == "A09") {
                return "R07";
            }
            return "R05";
        }
        if (種別 == "部署報告札") {
            return "R02";
        }
        if (種別 == "最終報告札") {
            return "R06";
        }
        return "R01";
    }

    static std::string 差戻し先決定(const std::string& 種別, const std::string& 問題番号) {
        if (問題番号 == "A12") {
            return "孔明くん";
        }
        if (種別 == "最終報告札") {
            return "主人";
        }
        if (種別 == "処理後札" || 種別 == "割当札") {
            return "孔明くん";
        }
        if (種別 == "監査結果札") {
            return "孔明くん";
        }
        return "孔明くん";
    }
};

struct 案件 {
    std::string 札番号;
    std::string 主人依頼;
    int 差戻し回数 = 0;
    std::string 直近問題;
};

class 中核機構 {
public:
    std::string 依頼受理(const std::string& 主人依頼) {
        案件 案;
        案.札番号 = 札番号生成();
        案.主人依頼 = 主人依頼;

        票 依頼札;
        依頼札.札番号 = 案.札番号;
        依頼札.種別 = "依頼札";
        依頼札.状態 = "受付";
        依頼札.日付 = 共通関数::現在時刻();
        依頼札.部署 = "未定";
        依頼札.担当 = "主人";
        依頼札.見出し = 共通関数::見出し要約(主人依頼);
        依頼札.差戻し回数 = 0;
        依頼札.内容["主人依頼"] = 主人依頼;
        依頼札.内容["依頼要約"] = 共通関数::見出し要約(主人依頼, 40);

        依頼札.版 = 書類.保存票(依頼札);

        auto 監査初期 = 監査.監査実行(依頼札, "主人");
        書類.保存監査結果(監査初期);
        if (監査初期.判定 != "可" && !差戻し進行(案, 監査初期)) {
            return 案.札番号;
        }
        if (監査初期.判定 != "可") {
            return 案.札番号;
        }

        案件処理(案);
        return 案.札番号;
    }

    void 目録表示() const {
        for (const auto& e : 書類.目録一覧()) {
            std::cout << e.札番号 << '\t' << e.版 << '\t' << e.日付 << '\t' << e.部署 << '\t'
                      << e.状態 << '\t' << e.見出し << '\t' << e.担当 << '\t' << e.判定 << '\t'
                      << e.最終更新 << '\n';
        }
    }

    void 追跡(const std::string& 札番号) const {
        for (const auto& e : 書類.目録一覧()) {
            if (e.札番号 == 札番号) {
                std::cout << e.札番号 << '\t' << e.版 << '\t' << e.状態 << '\t' << e.見出し << '\t'
                          << e.最終更新 << '\n';
            }
        }
    }

private:
    書類倉庫 書類;
    処理待ちブース 待ち札列;
    孔明くん 伝令;
    部長AI 部長;
    監査部門AI 監査;

    std::string 札番号生成() const {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return "札-" + 共通関数::日付文字列() + "-" + std::to_string(ms);
    }

    bool 差戻し進行(案件& 案件情報, const 監査結果札& 結果) {
        if (結果.判定 == "可") {
            return true;
        }

        案件情報.差戻し回数++;

        if (案件情報.差戻し回数 > 3) {
            票 返却 = 伝令.孔明くん返却札(案件情報.札番号, 案件情報.差戻し回数, "差戻し回数上限を超えました。孔明くんへ判断を戻します。");
            返却.版 = 書類.保存票(返却);
            auto rx = 監査.監査実行(返却, "孔明くん");
            書類.保存監査結果(rx);
            if (rx.判定 != "可") {
                票 主人確認 = 伝令.主人確認札(案件情報.札番号, 案件情報.差戻し回数, "差戻し上限後、孔明くんでも再確定不可。主人確認へ回します。");
                主人確認.版 = 書類.保存票(主人確認);
                auto ry = 監査.監査実行(主人確認, "孔明くん");
                書類.保存監査結果(ry);
            }
            return false;
        }

        if (案件情報.差戻し回数 == 3 && 案件情報.直近問題 == 結果.問題番号 && !結果.問題番号.empty()) {
            票 主人確認 = 伝令.主人確認札(案件情報.札番号, 案件情報.差戻し回数, "同一問題の繰返しで、主人確認へ回します。");
            主人確認.版 = 書類.保存票(主人確認);
            auto rz = 監査.監査実行(主人確認, "孔明くん");
            書類.保存監査結果(rz);
            return false;
        }

        案件情報.直近問題 = 結果.問題番号;
        return true;
    }

    void 案件処理(案件& 案件情報) {
        while (true) {
            auto 熟考 = 伝令.熟考生成(案件情報.札番号, 案件情報.主人依頼, 案件情報.差戻し回数);
            熟考.版 = 書類.保存票(熟考);
            auto r1 = 監査.監査実行(熟考, "孔明くん");
            書類.保存監査結果(r1);
            if (r1.判定 != "可" && !差戻し進行(案件情報, r1)) {
                return;
            }
            if (r1.判定 != "可") {
                continue;
            }

            int 優先度 = 部長.優先度判定(熟考);
            auto 業務 = 伝令.業務連絡作成(熟考, 優先度);
            業務.状態 = "処理待ち監査待ち";
            業務.版 = 書類.保存票(業務);
            auto r2 = 監査.監査実行(業務, "孔明くん");
            書類.保存監査結果(r2);
            if (r2.判定 != "可" && !差戻し進行(案件情報, r2)) {
                return;
            }
            if (r2.判定 != "可") {
                continue;
            }

            int 指示優先度 = 部長.優先度判定(業務);
            業務.状態 = "処理待ち";
            業務.版 = 書類.保存票(業務);
            処理待ち札 列{案件情報.札番号, 業務.版, 指示優先度};
            待ち札列.投入(列);

            if (待ち札列.空()) {
                票 主人向けエラー;
                主人向けエラー.札番号 = 案件情報.札番号;
                主人向けエラー.種別 = "主人確認札";
                主人向けエラー.状態 = "差戻し";
                主人向けエラー.日付 = 共通関数::現在時刻();
                主人向けエラー.見出し = "主人確認札";
                主人向けエラー.内容["要旨"] = "処理待ちブースが空でした。";
                主人向けエラー.版 = 書類.保存票(主人向けエラー);
                auto rw = 監査.監査実行(主人向けエラー, "孔明くん");
                書類.保存監査結果(rw);
                return;
            }

            (void)待ち札列.取り出し();

            auto [割当可, 社員] = 部長.割当(業務);
            if (!割当可) {
                監査結果札 r3;
                r3.対象札番号 = 案件情報.札番号;
                r3.対象版 = 業務.版;
                r3.対象者 = "部長AI";
                r3.判定 = "否";
                r3.問題番号 = "A03";
                r3.差戻し先 = "孔明くん";
                r3.再処理指示番号 = "R02";
                r3.監査時刻 = 共通関数::現在時刻();
                書類.保存監査結果(r3);
                if (!差戻し進行(案件情報, r3)) {
                    return;
                }
                continue;
            }

            auto 割当 = 部長.割当札作成(業務, 社員, 案件情報.差戻し回数);
            割当.状態 = "社員処理中";
            割当.版 = 書類.保存票(割当);
            auto r4 = 監査.監査実行(割当, "部長AI");
            書類.保存監査結果(r4);
            if (r4.判定 != "可" && !差戻し進行(案件情報, r4)) {
                return;
            }
            if (r4.判定 != "可") {
                continue;
            }

            票 処理後;
            処理後.札番号 = 案件情報.札番号;
            処理後.種別 = "処理後札";
            処理後.状態 = "処理後監査待ち";
            処理後.日付 = 共通関数::現在時刻();
            処理後.部署 = 割当.部署;
            処理後.担当 = 社員.氏名;
            処理後.見出し = "処理後札";
            処理後.差戻し回数 = 案件情報.差戻し回数;
            処理後.内容["処理成果"] = 社員.処理(案件情報.主人依頼, 割当.内容.at("指示"));
            処理後.内容["根拠"] = "熟考要約: " + 共通関数::値取得(熟考.内容, "熟考要約");
            処理後.版 = 書類.保存票(処理後);

            auto r5 = 監査.監査実行(処理後, 社員.氏名);
            書類.保存監査結果(r5);
            if (r5.判定 != "可" && !差戻し進行(案件情報, r5)) {
                return;
            }
            if (r5.判定 != "可") {
                continue;
            }

            auto 部署報告 = 部長.部署報告札作成(処理後, 案件情報.差戻し回数);
            部署報告.版 = 書類.保存票(部署報告);
            auto r6 = 監査.監査実行(部署報告, "部長AI");
            書類.保存監査結果(r6);
            if (r6.判定 != "可" && !差戻し進行(案件情報, r6)) {
                return;
            }
            if (r6.判定 != "可") {
                continue;
            }

            auto 最終報告 = 伝令.最終報告作成(案件情報.札番号, 処理後.内容.at("処理成果"), 案件情報.差戻し回数);
            最終報告.状態 = "最終監査待ち";
            最終報告.版 = 書類.保存票(最終報告);
            auto r7 = 監査.監査実行(最終報告, "孔明くん");
            書類.保存監査結果(r7);
            if (r7.判定 != "可" && !差戻し進行(案件情報, r7)) {
                return;
            }
            if (r7.判定 != "可") {
                continue;
            }

            auto 主人報告 = 伝令.最終報告作成(案件情報.札番号, 最終報告.内容.at("最終報告"), 案件情報.差戻し回数);
            主人報告.状態 = "主人報告済";
            主人報告.見出し = "主人報告札";
            主人報告.版 = 書類.保存票(主人報告);

            auto r8 = 監査.監査実行(主人報告, "孔明くん");
            書類.保存監査結果(r8);
            if (r8.判定 != "可" && !差戻し進行(案件情報, r8)) {
                return;
            }
            if (r8.判定 != "可") {
                continue;
            }

            auto 書類入庫 = 伝令.最終報告作成(案件情報.札番号,
                                              "監査を通過し、書類倉庫へ保存完了",
                                              案件情報.差戻し回数);
            書類入庫.状態 = "書類倉庫入り";
            書類入庫.日付 = 共通関数::現在時刻();
            書類入庫.見出し = "書類倉庫保存札";
            書類入庫.内容["要旨"] = "監査済みの最終結果を保存し完了します。";
            書類.保存票(書類入庫);
            return;
        }
    }
};

int main(int argc, char* argv[]) {
    std::locale::global(std::locale(""));
    中核機構 engine;

    if (argc <= 1) {
        std::cout << "使用法:\n";
        std::cout << "  中核仕組み 受付 <主人依頼>\n";
        std::cout << "  中核仕組み 目録\n";
        std::cout << "  中核仕組み 追跡 <札番号>\n";
        return 0;
    }

    std::string 命令 = argv[1];
    if (命令 == "受付") {
        if (argc < 3) {
            std::cout << "主人依頼を1つ指定してください。\n";
            return 1;
        }
        std::string 依頼;
        for (int i = 2; i < argc; ++i) {
            if (i > 2) {
                依頼.push_back(' ');
            }
            依頼 += argv[i];
        }
        std::string 番号 = engine.依頼受理(依頼);
        std::cout << "受付完了: " << 番号 << "\n";
        return 0;
    }

    if (命令 == "目録") {
        engine.目録表示();
        return 0;
    }

    if (命令 == "追跡") {
        if (argc < 3) {
            std::cout << "追跡する札番号が必要です。\n";
            return 1;
        }
        engine.追跡(argv[2]);
        return 0;
    }

    std::cout << "不明な命令です。\n";
    return 1;
}
