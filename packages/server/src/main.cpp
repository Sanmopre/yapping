#include "tcp_server.h"
#include "cmake_constants.h"

// cli11
#include "CLI/CLI.hpp"

#include "db_manager.h"
#include "data_manager.h"

int main(int argc, char **argv)
{
    CLI::App serverApplication(SERVER_DESCRIPTION);
    serverApplication.set_version_flag("--version", PROJECT_VERSION);

    std::string loggingFolder = "./logs";
    u16 port;

    serverApplication.add_option("-l,--log-folder", loggingFolder, "Path tp the folder where the logs from the application will be generated.")
       ->check(CLI::ExistingDirectory);

    serverApplication.add_option("-p,--port", port, "Server port for incoming connections")
       ->required()
       ->check(CLI::Range(1, 65535));

    CLI11_PARSE(serverApplication, argc, argv);

    const auto timeStampStringForFile = makeSafeForFilename(getTimeStamp(currentSecondsSinceEpoch()));
    const std::string logFile = std::string(SERVER_TARGET_NAME) + "_" + timeStampStringForFile + ".log";

    const auto logger = getLogger(
        SERVER_TARGET_NAME,
        (std::filesystem::path(loggingFolder) / logFile).string()
    );

    logger->info("Starting {} version {}", SERVER_TARGET_NAME, PROJECT_VERSION);

    DataBaseManager dbManager{logger.get()};

    server::DataManager dataManager(&dbManager, logger.get());
    dataManager.connect("0.0.0.0", port);

    spdlog::info("Server started on port {}", port);
    std::cin.get();
}
