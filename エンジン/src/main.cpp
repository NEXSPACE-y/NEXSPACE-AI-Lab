#include "widgets/app_shell.h"
#include <QApplication>
#include <QCommandLineParser>

using namespace nexspace;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("NEXSPACE AI Lab 軍議管制デスクトップUI");
    parser.addHelpOption();
    QCommandLineOption adapterOpt("adapter", "Use adapter: mock, real, noop", "mode", "mock");
    parser.addOption(adapterOpt);
    parser.process(app);

    const QString adapterMode = parser.value(adapterOpt);
    std::unique_ptr<EngineAdapter> adapter;

    if (adapterMode == "real") {
        adapter = std::make_unique<RealEngineAdapter>();
    } else if (adapterMode == "noop") {
        adapter = std::make_unique<NoopEngineAdapter>();
    } else {
        adapter = std::make_unique<MockEngineAdapter>();
    }

    AppShell shell(std::move(adapter));
    shell.setWindowTitle("NEXSPACE AI Lab 軍議管制");
    shell.resize(1700, 980);
    shell.show();

    return app.exec();
}
