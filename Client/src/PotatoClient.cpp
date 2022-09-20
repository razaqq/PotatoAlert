// Copyright 2020 <github.com/razaqq>

#include "Client/Config.hpp"
#include "Client/Game.hpp"
#include "Client/ReplayAnalyzer.hpp"
#include "Client/MatchHistory.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"

#include "Core/Defer.hpp"
#include "Core/Directory.hpp"
#include "Core/Json.hpp"
#include "Core/Log.hpp"
#include "Core/Sha256.hpp"
#include "Core/StandardPaths.hpp"

#include <QObject>
#include <QString>
#include <QTableWidgetItem>
#include <QTimer>
#include <QUrl>

#include <chrono>
#include <filesystem>
#include <format>
#include <optional>
#include <qnetworkreply.h>
#include <qthread.h>
#include <string>
#include <thread>


using PotatoAlert::Client::PotatoClient;
using namespace PotatoAlert::Core;

// static constexpr std::string_view g_submitUrl = "http://127.0.0.1:10001/queue/submit";
// static constexpr std::string_view g_lookupUrl = "http://127.0.0.1:10001{}";
static constexpr std::string_view g_submitUrl = "https://potatoalert.perry-swift.de/queue/submit";
static constexpr std::string_view g_lookupUrl = "https://potatoalert.perry-swift.de{}";
static constexpr int g_transferTimeout = 10000;

namespace {

struct TempArenaInfoResult
{
	bool Exists = true;
	bool Error = false;
	std::string Raw;
	std::string PlayerName;
	std::string PlayerVehicle;
	json Json;
	std::string Hash;
};

// opens and reads a file
static TempArenaInfoResult ReadArenaInfo(std::string_view filePath)
{
	LOG_TRACE("Reading arena info from: {}", filePath);

	TempArenaInfoResult result;

	using namespace std::chrono_literals;

	using TimePoint = std::chrono::high_resolution_clock::time_point;
	TimePoint startTime = std::chrono::high_resolution_clock::now();
	TimePoint now = std::chrono::high_resolution_clock::now();

	// wait for the game to close the write handle
	File file;
	while (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime) < 3000ms)
	{
		file = File::Open(filePath, File::Flags::Open | File::Flags::Read);

		if (file)
			break;

		std::this_thread::sleep_for(500ms);
		now = std::chrono::high_resolution_clock::now();
	}

	if (!file)
	{
		LOG_ERROR("Failed to open arena info file: {}", File::LastError());
		return { File::Exists(filePath), true, "" };
	}

	// close the file when the scope ends
	auto defer = MakeDefer([&file]() { if (file) file.Close(); });

	startTime = std::chrono::high_resolution_clock::now();
	now = std::chrono::high_resolution_clock::now();
	while (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime) < 1000ms)
	{
		if (!File::Exists(filePath))
		{
			// we have to assume its not an error, because the game sometimes touches it at the end of a match
			// when its about to get deleted
			return { false, false, "" };
		}

		// the game has written to the file
		if (file.Size() > 0)
		{
			if (std::string arenaInfo; file.ReadAllString(arenaInfo))
			{
				json j;
				sax_no_exception sax(j);
				if (!json::sax_parse(arenaInfo, &sax))
				{
					LOG_ERROR("Failed to parse arena info file as JSON.");
					return { true, true, "", "", "" };
				}

				std::string playerName = j.at("playerName").get<std::string>();
				std::string playerVehicle = j.at("playerVehicle").get<std::string>();

				std::string hash;
				Sha256(arenaInfo, hash);

				return { true, false, arenaInfo, playerName, playerVehicle, j, hash };
			}
			LOG_ERROR("Failed to read arena info file: {}", File::LastError());
			return { true, true, "" };
		}
		std::this_thread::sleep_for(100ms);
		now = std::chrono::high_resolution_clock::now();
	}
	LOG_ERROR("Game failed to write arena info within 1 second.");
	return { File::Exists(filePath), true, "", "", "" };
}

enum RequestStatus
{
	InProgress,
	Completed,
	Error,
};

NLOHMANN_JSON_SERIALIZE_ENUM(RequestStatus,
{
	{ RequestStatus::InProgress, "in_progress" },
	{ RequestStatus::Completed, "completed" },
	{ RequestStatus::Error, "error" },
})

struct ServerResponse
{
	RequestStatus Status;
	std::optional<std::string> Error;
	std::optional<std::string> IssuedAt;
	std::optional<std::string> CompletedAt;
	std::optional<json> Result;
};

[[maybe_unused]] static void from_json(const json& j, ServerResponse& r)
{
	j.at("status").get_to(r.Status);
	if (j.contains("error"))
		r.Error = j.at("error").get<std::string>();
	if (j.contains("issued_at"))
		r.IssuedAt = j.at("issued_at").get<std::string>();
	if (j.contains("completed_at"))
		r.CompletedAt = j.at("completed_at").get<std::string>();
	if (j.contains("result"))
		r.Result = j.at("result").get<json>();
}

}

void PotatoClient::Init()
{
	emit StatusReady(Status::Ready, "Ready");

	connect(&m_watcher, &DirectoryWatcher::FileChanged, this, &PotatoClient::OnFileChanged);
	connect(&m_watcher, &DirectoryWatcher::FileChanged, &m_replayAnalyzer, &ReplayAnalyzer::OnFileChanged);

	connect(&m_replayAnalyzer, &ReplayAnalyzer::ReplaySummaryReady, this, &PotatoClient::MatchSummaryChanged);

	TriggerRun();
}

void PotatoClient::SendRequest(std::string_view requestString, MatchContext&& matchContext)
{
	LOG_TRACE("Sending request with content: {}", requestString);

	QNetworkRequest submitRequest;
	submitRequest.setUrl(QUrl(g_submitUrl.data()));
	submitRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	submitRequest.setTransferTimeout(g_transferTimeout);
	QNetworkReply* submitReply = m_networkAccessManager->post(submitRequest, QByteArray(requestString.data(), static_cast<int>(requestString.size())));

	connect(submitReply, &QNetworkReply::finished, [this, submitReply, matchContext = std::move(matchContext)]()
	{
		auto handler = [this, &matchContext](QNetworkReply* submitReply)
		{
			const QString content = QString::fromUtf8(submitReply->readAll());
			if (content.isNull() || content == "null")
			{
				LOG_ERROR("Server responded with null to submit request");
				emit StatusReady(Status::Error, "NULL Response");
				return;
			}

			LOG_TRACE("Got submitReply from server: {}", content.toStdString());

			const QByteArray locationHeader = submitReply->rawHeader("Location");
			if (locationHeader.isEmpty())
			{
				LOG_ERROR("Server responded with an invalid location header");
				emit StatusReady(Status::Error, "Invalid Response");
				return;
			}
			const std::string lookupUrl = std::format(g_lookupUrl, locationHeader.toStdString());

			/* TODO: this should be working, but isn't
			const QVariant locationHeader = submitReply->header(QNetworkRequest::LocationHeader);
			if (!locationHeader.isValid() || locationHeader.type() != QVariant::String)
			{
				LOG_ERROR("Server responded with an invalid location header");
				emit StatusReady(Status::Error, "Invalid Response");
				return;
			}
			const QString location = locationHeader.toString();
			const std::string lookupUrl = std::format(g_lookupUrl, location.toStdString());
			*/

			LOG_TRACE("Got submitReply from server with location '{}' and content '{}'", lookupUrl, content.toStdString());

			json submitJson;
			sax_no_exception sax(submitJson);
			if (!json::sax_parse(content.toUtf8().toStdString(), &sax))
			{
				LOG_ERROR("Failed to parse server response as JSON.");
				emit StatusReady(Status::Error, "JSON Parse Error");
				return;
			}

			const std::string authToken = submitJson.at("AuthToken").get<std::string>();
			LookupResult(lookupUrl, authToken, matchContext);
		};
		HandleReply(submitReply, handler);
	});
}

void PotatoClient::LookupResult(const std::string& url, const std::string& authToken, const MatchContext& matchContext)
{
	LOG_TRACE("url: {} | auth: {}", url, authToken);

	QNetworkRequest lookupRequest;
	lookupRequest.setUrl(QUrl(url.c_str()));
	lookupRequest.setRawHeader("auth-token", authToken.c_str());
	lookupRequest.setTransferTimeout(g_transferTimeout);

	QTimer::singleShot(1000, [this, url, authToken, lookupRequest, &matchContext]()
	{
		QNetworkReply* lookupReply = m_networkAccessManager->get(lookupRequest);
		connect(lookupReply, &QNetworkReply::finished, [this, lookupReply, url, authToken, &matchContext]()
		{
			auto handler = [this, &url = url, authToken, &matchContext](QNetworkReply* lookupReply)
			{
				const QString content = QString::fromUtf8(lookupReply->readAll());
				if (content.isNull() || content == "null")
				{
					emit StatusReady(Status::Error, "NULL Response");
					return;
				}

				std::string lookupResponseString = content.toUtf8().toStdString();
				LOG_TRACE("Got lookupReply from server with content '{}'", lookupResponseString);

				json lookupJson;
				sax_no_exception sax(lookupJson);
				if (!json::sax_parse(lookupResponseString, &sax))
				{
					LOG_ERROR("Failed to parse server response as JSON.");
					emit StatusReady(Status::Error, "JSON Parse Error");
					return;
				}

				const ServerResponse serverResponse = lookupJson.get<ServerResponse>();
				switch (serverResponse.Status)
				{
					case InProgress:
					{
						LOG_TRACE("Request still in progress, sending another lookup");
						LookupResult(url, authToken, matchContext);
						break;
					}
					case Completed:
					{
						if (!serverResponse.Result)
						{
							LOG_ERROR("Server says ResponseStatus is completed, but is missing result field");
							return;
						}
						LOG_TRACE("Parsing match.");
						const StatsParser::StatsParseResult res = StatsParser::ParseMatch(serverResponse.Result.value(), matchContext, true);
						if (!res.Success)
						{
							emit StatusReady(Status::Error, "JSON Parse Error");
							return;
						}

						// TODO: this could use some cleanup
						if (m_services.Get<Config>().Get<ConfigKey::MatchHistory>())
						{
							if (m_services.Get<MatchHistory>().SaveMatch(
								res.Match.Info, matchContext.ArenaInfo, m_lastArenaInfoHash, serverResponse.Result.value().dump(), res.Csv.value()))
							{
								emit MatchHistoryChanged();
							}
						}

						LOG_TRACE("Updating tables.");
						emit MatchReady(res.Match);

						emit StatusReady(Status::Ready, "Ready");
						break;
					}
					case Error:
					{
						emit StatusReady(Status::Error, "Server Error");
						if (!serverResponse.Error)
						{
							LOG_ERROR("Server says ResponseStatus is error, but is missing error field");
							return;
						}
						LOG_ERROR("Server error: {}", serverResponse.Error.value());
						break;
					}
				}
			};
			HandleReply(lookupReply, handler);
		});
	});
}

void PotatoClient::HandleReply(QNetworkReply* reply, auto& successHandler)
{
	switch (reply->error())
	{
		case QNetworkReply::NoError:
		{
			successHandler(reply);
			return;
		}
		case QNetworkReply::ConnectionRefusedError:
			emit StatusReady(Status::Error, "Connection Refused");
			break;
		case QNetworkReply::RemoteHostClosedError:
			emit StatusReady(Status::Error, "Host Closed Conn.");
			break;
		case QNetworkReply::HostNotFoundError:
			emit StatusReady(Status::Error, "Host Not Found");
			break;
		// this means the request was cancelled along the way, aka timeout
		case QNetworkReply::OperationCanceledError:
		case QNetworkReply::TimeoutError:
			emit StatusReady(Status::Error, "Connection Timeout");
			break;
		case QNetworkReply::SslHandshakeFailedError:
			emit StatusReady(Status::Error, "SSL Error");
			break;
		case QNetworkReply::ServiceUnavailableError:
			emit StatusReady(Status::Error, "Service Unavailable");
			break;
		case QNetworkReply::InternalServerError:
			emit StatusReady(Status::Error, "Server Error");
			break;
		case QNetworkReply::TooManyRedirectsError:
			emit StatusReady(Status::Error, "Too Many Redirects");
			break;
		case QNetworkReply::InsecureRedirectError:
			emit StatusReady(Status::Error, "Insecure Redirect");
			break;
		default:
			emit StatusReady(Status::Error, "Request Error");
			break;
	}
	LOG_ERROR("Server reponded with error to submit: {}", reply->errorString().toStdString());
}

// triggers a run with the current replays folders
void PotatoClient::TriggerRun()
{
	m_watcher.ForceDirectoryChanged();
	m_watcher.ForceFileChanged("tempArenaInfo.json");
}

void PotatoClient::ForceRun()
{
	m_lastArenaInfoHash = "";
	TriggerRun();
}

// triggered whenever a file gets modified in a replays path
void PotatoClient::OnFileChanged(const std::string& file)
{
	LOG_TRACE("Directory changed.");

	if (fs::path(file).filename() != "tempArenaInfo.json" || !File::Exists(file))
	{
		return;
	}

	// read arena info from file
	TempArenaInfoResult arenaInfo = ReadArenaInfo(file);
	if (arenaInfo.Error)
	{
		LOG_ERROR("Failed to read arena info from file.");
		emit StatusReady(Status::Error, "Reading ArenaInfo");
		return;
	}

	if (!arenaInfo.Exists)
	{
		LOG_TRACE("Arena info did not exist when directory changed.");
		return;
	}

	// make sure we don't pull the same match twice
	if (arenaInfo.Hash == m_lastArenaInfoHash)
		return;
	m_lastArenaInfoHash = arenaInfo.Hash;

	// build the request
	const json request =
	{
		{ "Guid", "placeholder123" },
		{ "Player", arenaInfo.PlayerName },
		{ "Region", m_dirStatus.region },
		{ "StatsMode", m_services.Get<Config>().Get<ConfigKey::StatsMode>() },
		{ "TeamDamageMode", m_services.Get<Config>().Get<ConfigKey::TeamDamageMode>() },
		{ "TeamWinRateMode", m_services.Get<Config>().Get<ConfigKey::TeamWinRateMode>() },
		{ "ArenaInfo", arenaInfo.Json }
	};

	emit StatusReady(Status::Loading, "Loading");

	SendRequest(request.dump(),
		MatchContext{arenaInfo.Raw, arenaInfo.PlayerName, arenaInfo.PlayerVehicle});
}

// checks the game path for a valid replays folder and triggers a new match run
DirectoryStatus PotatoClient::CheckPath()
{
	DirectoryStatus dirStatus;

	m_watcher.ClearDirectories();

	const std::string gamePath = m_services.Get<Config>().Get<ConfigKey::GameDirectory>();

	if (Game::CheckPath(gamePath, dirStatus))
	{
		m_dirStatus = dirStatus;

		// start watching all replays paths
		for (const std::string& folder : dirStatus.replaysPath)
		{
			if (!folder.empty())
			{
				m_watcher.WatchDirectory(folder);
			}
		}

		// make sure we have up-to-date game scripts
		const std::string scriptVersion = dirStatus.gameVersion.ToString(".", true);
		if (!m_replayAnalyzer.HasGameScripts(dirStatus.gameVersion))
		{
			LOG_INFO("Missing game scripts for version {} detected, trying to unpack...", scriptVersion);
			const std::string dst = (AppDataPath("PotatoAlert") / "ReplayVersions" / scriptVersion).string();
			if (!ReplayAnalyzer::UnpackGameScripts(dst, dirStatus.pkgPath.string(), dirStatus.idxPath.string()))
			{
				LOG_ERROR("Failed to unpack game scripts for version: {}", scriptVersion);
			}
		}
		LOG_INFO("Game scripts for version {} found", scriptVersion);

		// lets check the entire game folder, replays might be hiding everywhere
		m_replayAnalyzer.AnalyzeDirectory(gamePath);

		TriggerRun();
	}
	return dirStatus;
}
