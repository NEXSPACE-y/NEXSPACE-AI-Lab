# NEXSPACE AI Lab 軍議管制（デスクトップUI）

このプロジェクトは、`見た目` 側を C++/Qt Desktop アプリとして構成しました。  
エンジンは既存の C++ 実装と同一言語を採用し、  
接続済みAPIに依存しない**Mock / Noop / Real（未接続）境界**を明示しています。

## 構成

- `src/domain.h`
  - 札・AI・監査・本陣状態・履歴のドメイン型
- `src/adapters/`
  - `mock_engine_adapter.*` : UI確認用データ
  - `real_engine_adapter.*` : 接続未実装（Noop扱いで明示）
  - `noop_engine_adapter.*` : 未接続時の明示的な境界
- `src/sound/`
  - `soundCue*` : 軍議演出SE導線（現時点はNoop）
- `src/widgets/`
  - AppShell / サイドバー遷移 / 上部ステータス / 札場 / 監察ログ / 履歴 / 設定 / 本陣状態

## ビルド（配布フォルダ前提）

```powershell
# まず配布成果物をクリーン
Remove-Item -Recurse -Force .\build, .\dist -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path .\build | Out-Null
Set-Location .\build

# CMake 構成後、ビルド（dist/nexspace_ai_lab_ui 配下に exe を作成）
cmake ..
cmake --build .
```

実行:

```powershell
# dist 配下の exe から起動（直接 exe コピーではなく dist フォルダ単位で起動）
..\dist\nexspace_ai_lab_ui\nexspace_ai_lab_ui.exe --adapter mock
```

`--adapter` は `mock` / `real` / `noop` を指定できます。  
既定は `mock` です。

### 環境チェック

```powershell
where.exe cmake
where.exe qmake
where.exe cl
where.exe clang-cl
where.exe ninja
where.exe winget
```

### 環境準備（必要時）

- CMake

```powershell
winget install --id Kitware.CMake --accept-source-agreements --accept-package-agreements
```

- Qt
  - Qt Online Installer で Qt 5.15 または 6.x の **Qt Widgets** をインストール
  - Qt の `bin` 配下（例: `C:\\Qt\\...\\bin`）を `PATH` に追加
- 本プロジェクトは `Qt5` または `Qt6` の `Widgets` が見つかれば自動で利用します
- このUIは **C++/Qt（Qt Widgets）** です。Python 用バインディング（PyQt/PySide）は使っていません。

## 仕様上の明示

- SE音は再生しません（Noop）。
- `mock` と `real/noop` の表示をUI上に明示。
- 監察ログ・エラー表示・未接続表示は未接続のままで「接続済み」のように見せない構成にしています。

未接続・未実装APIの追加実装が必要な項目は、`RealEngineAdapter` / `SoundCue` の差替え導線で拡張可能です。
