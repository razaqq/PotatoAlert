// Copyright 2020 <github.com/razaqq>

#include "Client/AppDirectories.hpp"
#include "Client/Config.hpp"
#include "Client/DatabaseManager.hpp"
#include "Client/Game.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/ReplayAnalyzer.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"

#include "Core/Defer.hpp"
#include "Core/Format.hpp"
#include "Core/Json.hpp"
#include "Core/Log.hpp"
#include "Core/Sha256.hpp"
#include "Core/String.hpp"
#include "Core/StandardPaths.hpp"
#include "Core/Time.hpp"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <thread>


using PotatoAlert::Client::PotatoClient;
using PotatoAlert::Client::StatsParser::MatchType;
using namespace PotatoAlert::Core;

// static constexpr std::string_view g_submitUrl = "http://127.0.0.1:10001/queue/submit";
// static constexpr std::string_view g_lookupUrl = "http://127.0.0.1:10001{}";
static constexpr std::string_view g_submitUrl = "https://potatoalert.perry-swift.de/queue/submit";
static constexpr std::string_view g_lookupUrl = "https://potatoalert.perry-swift.de{}";
static constexpr int g_transferTimeout = 10000;

namespace {

struct TempArenaInfoResult
{
	std::string Raw;
	std::string PlayerName;
	std::string PlayerVehicle;
	rapidjson::Document Json;
	std::string Hash;
};

// opens and reads a file
static Result<TempArenaInfoResult, std::string> ReadArenaInfo(const fs::path& filePath)
{
	LOG_TRACE(STR("Reading arena info from: {}"), filePath);

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
		return PA_ERROR(fmt::format("Failed to open arena info file: {}", File::LastError()));
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
			return TempArenaInfoResult{ "" };
		}

		// the game has written to the file
		if (file.Size() > 0)
		{
			if (std::string arenaInfo; file.ReadAllString(arenaInfo))
			{
				PA_TRY(j, ParseJson(arenaInfo));
				PA_TRY(playerName, ::FromJson<std::string>(j, "playerName"));
				PA_TRY(playerVehicle, ::FromJson<std::string>(j, "playerVehicle"));

				std::string hash;
				Sha256(arenaInfo, hash);

				return TempArenaInfoResult{ arenaInfo, playerName, playerVehicle, std::move(j), hash };
			}
			return PA_ERROR(fmt::format("Failed to read arena info file: {}", File::LastError()));
		}
		std::this_thread::sleep_for(100ms);
		now = std::chrono::high_resolution_clock::now();
	}
	return PA_ERROR(fmt::format("Game failed to write arena info within 1 second."));
}

enum RequestStatus
{
	InProgress,
	Completed,
	Error,
};

PA_JSON_SERIALIZE_ENUM(RequestStatus,
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
	std::optional<rapidjson::Document> Result;
};

[[maybe_unused]] static JsonResult<void> FromJson(rapidjson::Document& j, ServerResponse& r)
{
	if (!j.HasMember("status"))
		return PA_JSON_ERROR("Server response is missing status");
	FromJson(j["status"], r.Status);
	if (j.HasMember("error"))
	{
		PA_TRYA(r.Error, PotatoAlert::Core::FromJson<std::string>(j, "error"));
	}
	if (j.HasMember("issued_at"))
	{
		PA_TRYA(r.IssuedAt, PotatoAlert::Core::FromJson<std::string>(j, "issued_at"));
	}
	if (j.HasMember("completed_at"))
	{
		PA_TRYA(r.CompletedAt, PotatoAlert::Core::FromJson<std::string>(j, "completed_at"));
	}
	if (j.HasMember("result"))
	{
		rapidjson::Document d;
		d.Swap(j["result"]);
		r.Result = std::move(d);
	}

	return {};
}

static inline std::string GetReplayName(const MatchType::InfoType& info)
{
	const std::vector<std::string> dateSplit = Split(info.DateTime, " ");
	const std::vector<std::string> date = Split(dateSplit[0], ".");
	const std::vector<std::string> time = Split(dateSplit[1], ":");

	return fmt::format("{}{}{}_{}{}{}_{}_{}.wowsreplay", date[2], date[1], date[0], time[0], time[1], time[2], info.ShipIdent, info.Map);
}

}

void PotatoClient::Init()
{
	connect(&m_watcher, &DirectoryWatcher::FileChanged, this, &PotatoClient::OnFileChanged);
	connect(&m_watcher, &DirectoryWatcher::FileChanged, &m_replayAnalyzer, &ReplayAnalyzer::OnFileChanged);

	connect(&m_replayAnalyzer, &ReplayAnalyzer::ReplaySummaryReady, this, &PotatoClient::ReplaySummaryChanged);

	CheckPath();
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
			const std::string lookupUrl = fmt::format(g_lookupUrl, locationHeader.toStdString());

			/* TODO: this should be working, but isn't
			const QVariant locationHeader = submitReply->header(QNetworkRequest::LocationHeader);
			if (!locationHeader.isValid() || locationHeader.type() != QVariant::String)
			{
				LOG_ERROR("Server responded with an invalid location header");
				emit StatusReady(Status::Error, "Invalid Response");
				return;
			}
			const QString location = locationHeader.toString();
			const std::string lookupUrl = fmt::format(g_lookupUrl, location.toStdString());
			*/

			LOG_TRACE("Got submitReply from server with location '{}' and content '{}'", lookupUrl, content.toStdString());

			PA_TRY_OR_ELSE(json, ParseJson(content.toUtf8().toStdString()),
			{
				LOG_ERROR("Failed to parse server response as JSON.");
				emit StatusReady(Status::Error, "JSON Parse Error");
				return;
			});

			if (!json.HasMember("AuthToken"))
			{
				LOG_ERROR("Server response does not include an auth token");
				emit StatusReady(Status::Error, "Auth Error");
				return;
			}

			LookupResult(lookupUrl, Core::FromJson<std::string>(json["AuthToken"]), matchContext);
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

				const std::string lookupResponseString = content.toUtf8().toStdString();
				LOG_TRACE("Got lookupReply from server with content '{}'", lookupResponseString);

				PA_TRY_OR_ELSE(json, ParseJson(lookupResponseString),
				{
					LOG_ERROR("Failed to parse server response as JSON.");
					emit StatusReady(Status::Error, "JSON Parse Error");
					return;
				});

				ServerResponse serverResponse;
				PA_TRYV_OR_ELSE(FromJson(json, serverResponse),
				{
					LOG_ERROR("Failed to parse json as ServerResponse");
					emit StatusReady(Status::Error, "JSON Parse Error");
					return;
				});
				
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
						const bool showKarma = m_services.Get<Config>().Get<ConfigKey::ShowKarma>();
						const bool fontShadow = m_services.Get<Config>().Get<ConfigKey::FontShadow>();
						PA_TRY_OR_ELSE(res, ParseMatch(serverResponse.Result.value(), matchContext, showKarma, fontShadow),
						{
							LOG_ERROR("Failed to parse server response as JSON: {}", error);
							emit StatusReady(Status::Error, "JSON Parse Error");
							return;
						});

						const Config& config = m_services.Get<Config>();

						if (config.Get<ConfigKey::MatchHistory>())
						{
							const DatabaseManager& dbm = m_services.Get<DatabaseManager>();

							PA_TRY_OR_ELSE(exists, dbm.MatchExists(m_lastArenaInfoHash),
							{
								LOG_ERROR("Failed to check if match exists in database: {}", error);
								return;
							});

							if (!exists)
							{
								LOG_TRACE("Adding match to match history '{}'", m_lastArenaInfoHash);
								
								rapidjson::StringBuffer buffer;
								rapidjson::Writer writer(buffer);
								serverResponse.Result.value().Accept(writer);

								Match match
								{
									.Hash = m_lastArenaInfoHash,
									.ReplayName = GetReplayName(res.Match.Info),
									.Date = res.Match.Info.DateTime,
									.Ship = res.Match.Info.ShipName,
									.ShipNation = res.Match.Info.ShipNation,
									.ShipClass = res.Match.Info.ShipClass,
									.ShipTier = res.Match.Info.ShipTier,
									.Map = res.Match.Info.Map,
									.MatchGroup = res.Match.Info.MatchGroup,
									.StatsMode = res.Match.Info.StatsMode,
									.Player = res.Match.Info.Player,
									.Region = res.Match.Info.Region,
									.Json = buffer.GetString(),
									.ArenaInfo = matchContext.ArenaInfo,
									.Analyzed = false,
									.ReplaySummary = ReplaySummary{}
								};

								SqlResult<void> addResult = dbm.AddMatch(match);
								if (addResult)
								{
									emit MatchHistoryNewMatch(match);
								}
								else
								{
									LOG_ERROR("Failed to add match to database: {}", addResult.error());
								}
							}
						}

						if (config.Get<ConfigKey::SaveMatchCsv>())
						{
							const fs::path fileName = m_services.Get<AppDirectories>().MatchesDir / fmt::format("match_{}.csv", Time::GetTimeStamp("%Y-%m-%d_%H-%M-%S"));
							if (const File file = File::Open(fileName, File::Flags::Write | File::Flags::Create))
							{
								if (file.WriteString(res.Csv))
								{
									LOG_TRACE("Wrote match as CSV.");
								}
								else
								{
									LOG_ERROR("Failed to save match as csv: {}", File::LastError());
								}
							}
							else
							{
								LOG_ERROR("Failed to open csv file for writing: {}", File::LastError());
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
		case QNetworkReply::OperationNotImplementedError:
		{
			const QString content = QString::fromUtf8(reply->readAll());

			PA_TRY_OR_ELSE(errorJson, ParseJson(content.toUtf8().toStdString()),
			{
				LOG_ERROR("Failed to parse server error response as JSON.");
				emit StatusReady(Status::Error, "JSON Parse Error");
				return;
			});

			emit StatusReady(Status::Error, Core::FromJson<std::string>(errorJson["error"]));
			break;
		}
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
void PotatoClient::OnFileChanged(const std::filesystem::path& file)
{
	LOG_TRACE("Directory changed.");

	if (file.filename() != fs::path("tempArenaInfo.json") || !File::Exists(file))
	{
		return;
	}

	PA_TRY_OR_ELSE(arenaInfo, ReadArenaInfo(file),
	{
		LOG_ERROR("Failed to read arena info from file: {}", error);
		emit StatusReady(Status::Error, "Reading ArenaInfo");
		return;
	});

	if (!File::Exists(file))
	{
		LOG_TRACE("Arena info does not exist when directory changed.");
		return;
	}

	// make sure we don't pull the same match twice
	if (arenaInfo.Hash == m_lastArenaInfoHash)
		return;
	m_lastArenaInfoHash = arenaInfo.Hash;

	if (m_dirStatus.region.empty())
	{
		LOG_ERROR("No region set in DirectoryStatus");
		emit StatusReady(Status::Error, "No Region Set");
		return;
	}

	// build the request
	rapidjson::Document request;
	request.SetObject();
	rapidjson::MemoryPoolAllocator<> a = request.GetAllocator();
	request.AddMember("Guid", "placeholder123", a);
	request.AddMember("Player", arenaInfo.PlayerName, a);
	request.AddMember("Region", m_dirStatus.region, a);
	request.AddMember("StatsMode", ToJson(m_services.Get<Config>().Get<ConfigKey::StatsMode>()), a);
	request.AddMember("TeamDamageMode", ToJson(m_services.Get<Config>().Get<ConfigKey::TeamDamageMode>()), a);
	request.AddMember("TeamWinRateMode", ToJson(m_services.Get<Config>().Get<ConfigKey::TeamWinRateMode>()), a);
	request.AddMember("ArenaInfo", arenaInfo.Json, a);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer writer(buffer);
	request.Accept(writer);

	emit StatusReady(Status::Loading, "Loading");

	SendRequest(buffer.GetString(),
		MatchContext{arenaInfo.Raw, arenaInfo.PlayerName, arenaInfo.PlayerVehicle});
}

// checks the game path for a valid replays folder and triggers a new match run
DirectoryStatus PotatoClient::CheckPath()
{
	DirectoryStatus dirStatus;

	m_watcher.ClearDirectories();

	const Result<fs::path> gamePath = m_services.Get<Config>().Get<ConfigKey::GameDirectory>();
	if (!gamePath)
	{
		emit DirectoryStatusChanged(dirStatus);
		return dirStatus;
	}

	if (Game::CheckPath(gamePath.value(), dirStatus))
	{
		m_dirStatus = dirStatus;

		// start watching all replays paths
		for (const fs::path& folder : dirStatus.replaysPath)
		{
			if (!folder.empty())
			{
				m_watcher.WatchDirectory(folder);
			}
		}

		// make sure we have up-to-date game files
		const std::string gameVersion = dirStatus.gameVersion.ToString(".", true);
		if (!m_replayAnalyzer.HasGameFiles(dirStatus.gameVersion))
		{
			LOG_INFO("Missing game files for version {} detected, trying to unpack...", gameVersion);
			const fs::path dst = AppDataPath("PotatoAlert") / "ReplayVersions" / gameVersion;
			if (!ReplayAnalyzer::UnpackGameFiles(dst, dirStatus.pkgPath, dirStatus.idxPath))
			{
				LOG_ERROR("Failed to unpack game files for version: {}", gameVersion);
			}
		}
		LOG_INFO("Game files for version {} found", gameVersion);

		// lets check the entire game folder, replays might be hiding everywhere
		m_replayAnalyzer.AnalyzeDirectory(gamePath.value());

		emit StatusReady(Status::Ready, "Ready");

		TriggerRun();
	}
	else
	{
		emit StatusReady(Status::Error, "Game Directory");
	}

	emit DirectoryStatusChanged(dirStatus);
	return dirStatus;
}
