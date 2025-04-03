// Copyright 2020 <github.com/razaqq>

#include "Client/AppDirectories.hpp"
#include "Client/Config.hpp"
#include "Client/DatabaseManager.hpp"
#include "Client/Game.hpp"
#include "Client/PotatoClient.hpp"
#include "Client/ReplayAnalyzer.hpp"
#include "Client/ServiceProvider.hpp"
#include "Client/StatsParser.hpp"
#include "Client/SysInfo.hpp"

#include "Core/Defer.hpp"
#include "Core/Directory.hpp"
#include "Core/Format.hpp"
#include "Core/Json.hpp"
#include "Core/Log.hpp"
#include "Core/Sha256.hpp"
#include "Core/String.hpp"
#include "Core/Time.hpp"

#include "ReplayParser/ReplayParser.hpp"
#include "ReplayParser/Result.hpp"

#include <QApplication>
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
using PotatoAlert::Client::StatsParser::Match;
using PotatoAlert::Client::MatchContext;
using PotatoAlert::Core::String::Split;
using PotatoAlert::ReplayParser::Replay;
using PotatoAlert::ReplayParser::ReplayMeta;
using PotatoAlert::ReplayParser::ReplayResult;
using namespace PotatoAlert::Core;
namespace fs = std::filesystem;

namespace {

struct TempArenaInfoResult
{
	std::string Hash;
	std::string String;
	rapidjson::Document Json;
	ReplayMeta Meta;
};

// opens and reads a file
static Result<std::optional<TempArenaInfoResult>, std::string> ReadArenaInfo(const fs::path& filePath)
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

	PA_DEFER { if (file) file.Close(); };

	startTime = std::chrono::high_resolution_clock::now();
	now = std::chrono::high_resolution_clock::now();
	while (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime) < 1000ms)
	{
		if (!File::Exists(filePath))
		{
			// we have to assume it's not an error, because the game sometimes touches it at the end of a match
			// when its about to get deleted
			return std::nullopt;
		}

		// the game has written to the file
		if (file.Size() > 0)
		{
			if (std::string arenaInfo; file.ReadAllString(arenaInfo))
			{
				ReplayResult<ReplayMeta> meta = Replay::ParseMeta(arenaInfo);
				if (!meta)
				{
					return PA_ERROR(fmt::format("Failed to parse arena info as ReplayMeta: {}", meta.error()));
				}

				std::string hash;
				Sha256(arenaInfo, hash);

				PA_TRY(json, ParseJson(arenaInfo));

				return TempArenaInfoResult
				{
					.Hash = hash,
					.String = arenaInfo,
					.Json = std::move(json),
					.Meta = std::move(*meta),
				};
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

enum ServerErrorCategory
{
	WargamingApiError,
	WowsNumbersApiError,
	MatchBuilderError,
	RequestHandlerError,
	UnknownError
};

PA_JSON_SERIALIZE_ENUM(ServerErrorCategory,
{
	{ WargamingApiError, "WargamingApiError"},
	{ WowsNumbersApiError, "WowsNumbersApiError" },
	{ MatchBuilderError, "MatchBuilderError"},
	{ RequestHandlerError, "RequestHandlerError"},
	{ UnknownError, "UnknownError" }
});

struct ServerResponse
{
	RequestStatus Status;
	std::optional<std::string> Error;
	std::optional<ServerErrorCategory> ErrorCategory;
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
	if (j.HasMember("error_category"))
	{
		ServerErrorCategory category;
		if (FromJson(j["error_category"], category))
		{
			r.ErrorCategory = category;
		}

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

static inline std::optional<std::string> GetReplayName(const Match& match, const MatchContext& ctx)
{
	const std::optional<Time::TimePoint> tp = Time::StrToTime(match.DateTime, "%d.%m.%Y %H:%M:%S");
	if (tp)
	{
		const std::string time = Time::TimeToStr(*tp, "{:%Y%m%d_%H%M%S}");
		return fmt::format("{}_{}_{}.wowsreplay", time, ctx.ShipIdent, match.Map);
	}
	return {};
}

}

void PotatoClient::Init()
{
	Result<SysInfo> sysInfo = GetSysInfo();
	if (sysInfo)
	{
		m_sysInfo = *sysInfo;
	}
	else
	{
		LOG_ERROR("Failed to get SysInfo: {}", sysInfo.error().message());
	}

	connect(&m_watcher, &DirectoryWatcher::FileChanged, this, &PotatoClient::OnFileChanged);

	connect(&m_replayAnalyzer, &ReplayAnalyzer::ReplaySummaryReady, this, &PotatoClient::ReplaySummaryChanged);

	UpdateGameInstalls();
}

std::string PotatoClient::BuildRequest(const std::string& region, const std::string& playerName, rapidjson::Document& json) const
{
	rapidjson::Document request;
	request.SetObject();
	rapidjson::MemoryPoolAllocator<> a = request.GetAllocator();
	request.AddMember("Guid", "placeholder123", a);
	request.AddMember("Player", playerName, a);
	request.AddMember("Region", region, a);
	request.AddMember("StatsMode", ToJson(m_services.Get<Config>().Get<ConfigKey::StatsMode>()), a);
	request.AddMember("TeamDamageMode", ToJson(m_services.Get<Config>().Get<ConfigKey::TeamDamageMode>()), a);
	request.AddMember("TeamWinRateMode", ToJson(m_services.Get<Config>().Get<ConfigKey::TeamWinRateMode>()), a);
	request.AddMember("ArenaInfo", std::move(json), a);

	request.AddMember("ClientInfo", rapidjson::Value(rapidjson::kObjectType), a);
	rapidjson::Value& clientInfo = request["ClientInfo"];
	clientInfo.AddMember("ClientVersion", QApplication::applicationVersion().toStdString(), a);
	if (m_services.Get<Config>().Get<ConfigKey::AllowSendingUsageStats>() && m_sysInfo)
	{
		clientInfo.AddMember("SysInfo", rapidjson::Value(rapidjson::kObjectType), a);
		rapidjson::Value& sysInfo = clientInfo["SysInfo"];
		ToJson(sysInfo, *m_sysInfo, a);
	}

	rapidjson::StringBuffer buffer;
	rapidjson::Writer writer(buffer);
	request.Accept(writer);
	return buffer.GetString();
}

void PotatoClient::SendRequest(std::string_view requestString, MatchContext&& matchContext)
{
	LOG_TRACE("Sending request with content: {}", requestString);

	QNetworkRequest submitRequest;
	submitRequest.setUrl(QUrl(m_options.SubmitUrl.data()));
	submitRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	submitRequest.setTransferTimeout(m_options.TransferTimeout);
	QNetworkReply* submitReply = m_networkAccessManager->post(submitRequest, QByteArray(requestString.data(), static_cast<int>(requestString.size())));

	connect(submitReply, &QNetworkReply::finished, [this, submitReply, matchContext = std::move(matchContext)]()
	{
		auto handler = [this, &matchContext](QNetworkReply* reply)
		{
			const QString content = QString::fromUtf8(reply->readAll());
			if (content.isNull() || content == "null")
			{
				LOG_ERROR("Server responded with null to submit request");
				emit StatusReady(Status::Error, "NULL Response");
				return;
			}

			LOG_TRACE("Got submitReply from server: {}", content.toStdString());

			const QByteArray locationHeader = reply->rawHeader("Location");
			if (locationHeader.isEmpty())
			{
				LOG_ERROR("Server responded with an invalid location header");
				emit StatusReady(Status::Error, "Invalid Response");
				return;
			}
			const std::string lookupUrl = fmt::format("{}{}", m_options.LookupUrl, locationHeader.toStdString());

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
				LOG_ERROR("Failed to parse server submit response as JSON.");
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
	lookupRequest.setTransferTimeout(m_options.TransferTimeout);

	QTimer::singleShot(1000, [this, url, authToken, lookupRequest, &matchContext]()
	{
		QNetworkReply* lookupReply = m_networkAccessManager->get(lookupRequest);
		connect(lookupReply, &QNetworkReply::finished, [this, lookupReply, url, authToken, &matchContext]()
		{
			auto handler = [this, &url = url, authToken, &matchContext](QNetworkReply* reply)
			{
				const QString content = QString::fromUtf8(reply->readAll());
				if (content.isNull() || content == "null")
				{
					emit StatusReady(Status::Error, "NULL Response");
					return;
				}

				const std::string lookupResponseString = content.toUtf8().toStdString();
				LOG_TRACE("Got lookupReply from server with content '{}'", lookupResponseString);

				PA_TRY_OR_ELSE(json, ParseJson(lookupResponseString),
				{
					LOG_ERROR("Failed to parse lookup response as JSON.");
					emit StatusReady(Status::Error, "JSON Parse Error");
					return;
				});

				ServerResponse serverResponse;
				PA_TRYV_OR_ELSE(FromJson(json, serverResponse),
				{
					LOG_ERROR("Failed to parse server lookup response as JSON.");
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

						rapidjson::StringBuffer buffer;
						rapidjson::Writer writer(buffer);
						serverResponse.Result.value().Accept(writer);
						std::string jsonString = buffer.GetString();

						LOG_TRACE("Parsing match.");
						PA_TRY_OR_ELSE(match, StatsParser::ParseMatch(jsonString),
						{
							LOG_ERROR("Failed to parse server match response as JSON: {}", error);
							emit StatusReady(Status::Error, "JSON Parse Error");
							return;
						});

						const Config& config = m_services.Get<Config>();

						if (config.Get<ConfigKey::MatchHistory>())
						{
							DbAddMatch(m_lastArenaInfoHash, match, matchContext, std::move(jsonString));
						}

						if (config.Get<ConfigKey::SaveMatchCsv>())
						{
							PA_TRY_OR_ELSE(csv, StatsParser::ToCSV(match),
							{
								LOG_ERROR("Failed to convert match to CSV: {}", error);
								return;
							});

							const fs::path fileName = m_services.Get<AppDirectories>().MatchesDir / fmt::format("match_{}.csv", Time::GetTimeStamp("%Y-%m-%d_%H-%M-%S"));
							if (const File file = File::Open(fileName, File::Flags::Write | File::Flags::Create))
							{
								if (file.WriteString(csv))
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
						emit MatchReady(match, matchContext);
						emit StatusReady(Status::Ready, "Ready");
						break;
					}
					case Error:
					{
						if (serverResponse.ErrorCategory)
						{
							switch (*serverResponse.ErrorCategory)
							{
								case WargamingApiError:
								{
									emit StatusReady(Status::Error, "WG API Error");
									break;
								}
								case WowsNumbersApiError:
								{
									emit StatusReady(Status::Error, "WOWS-NBRS API Error");
									break;
								}
								case MatchBuilderError:
								case RequestHandlerError:
								case UnknownError:
								{
									emit StatusReady(Status::Error, "Server Error");
									break;
								}
							}
							LOG_ERROR("Server error - {}", serverResponse.Error.value());
							return;
						}
						emit StatusReady(Status::Error, "Server Error");
						if (!serverResponse.Error)
						{
							LOG_ERROR("Server says ResponseStatus is error, but is missing error field");
							return;
						}
						LOG_ERROR("Server error - {}", serverResponse.Error.value());
						break;
					}
				}
			};
			HandleReply(lookupReply, handler);
		});
	});
}

void PotatoClient::DbAddMatch(std::string_view hash, const Match& match, const MatchContext& matchContext, std::string&& jsonString)
{
	const DatabaseManager& dbm = m_services.Get<DatabaseManager>();

	PA_TRY_OR_ELSE(exists, dbm.MatchExists(hash),
	{
		LOG_ERROR("Failed to check if match exists in database: {}", error);
		return;
	});

	if (!exists)
	{
		LOG_TRACE("Adding match to match history '{}'", hash);

		const std::optional<std::string> replayName = GetReplayName(match, matchContext);
		if (!replayName)
		{
			LOG_ERROR("Failed to get replay name");
			return;
		}

		const auto it = std::ranges::find_if(match.Team1.Players, [&matchContext](const StatsParser::Player& player)
		{
			if (player.Name == matchContext.PlayerName)
			{
				return true;
			}
			return false;
		});

		if (it == match.Team1.Players.end())
		{
			LOG_ERROR("Failed to find player in match");
			return;
		}

		StatsParser::Ship ship = it->Ship.value_or(StatsParser::Ship{});

		DbMatch dbMatch
		{
			.Hash = m_lastArenaInfoHash,
			.ReplayName = *replayName,
			.Date = match.DateTime,
			.Ship = ship.Name,
			.ShipNation = ship.Nation,
			.ShipClass = ship.Class,
			.ShipTier = ship.Tier,
			.Map = match.Map,
			.MatchGroup = match.MatchGroup,
			.StatsMode = match.StatsMode,
			.Player = it->Name,
			.Region = match.Region,
			.Json = std::move(jsonString),
			.ArenaInfo = matchContext.ArenaInfo,
			.Analyzed = false,
			.ReplaySummary = ReplaySummary{}
		};

		SqlResult<void> addResult = dbm.AddMatch(dbMatch);
		if (addResult)
		{
			emit MatchHistoryNewMatch(dbMatch);
		}
		else
		{
			LOG_ERROR("Failed to add match to database: {}", addResult.error());
		}
	}
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
	LOG_ERROR("Server responded with error to submit: {}", reply->errorString().toStdString());
}

// triggers a run with the current replays folders
void PotatoClient::TriggerRun()
{
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
	LOG_TRACE("File changed: {}", file);

	if (!File::Exists(file))
	{
		return;
	}

	auto findGame = [this, &file]() -> std::optional<GameInfo>
	{
		std::optional<GameInfo> game = std::nullopt;
		for (const GameDirectory& gameDir : m_gameInfos)
		{
			Result<bool> eq = IsSubdirectory(file, gameDir.Path);
			if (eq && *eq && gameDir.Info)
			{
				game = gameDir.Info;
			}
		}

		return game;
	};

	if (file.extension() == fs::path(".wowsreplay"))
	{
		const std::optional<GameInfo> game = findGame();
		if (!game)
		{
			LOG_ERROR("Replay path is not inside any game directory.");
			emit StatusReady(Status::Error, "Replay Path");
			return;
		}
		OnReplayChanged(file, *game);
	}
	else if (file.filename() == fs::path("tempArenaInfo.json"))
	{
		const std::optional<GameInfo> game = findGame();
		if (!game)
		{
			LOG_ERROR("Arena info path is not inside any game directory.");
			emit StatusReady(Status::Error, "Arena Info Path");
			return;
		}
		OnTempArenaInfoChanged(file, *game);
	}
}

void PotatoClient::OnReplayChanged(const std::filesystem::path& file, const GameInfo& game)
{
	if (file.filename() != fs::path("temp.wowsreplay"))
	{
		LOG_TRACE("Replay file {} changed", file);

		if (game.Region.empty())
		{
			LOG_ERROR("No region set in DirectoryStatus");
			emit StatusReady(Status::Error, "No Region Set");
			return;
		}

		m_replayAnalyzer.AnalyzeReplay(file, game.Region, std::chrono::seconds(30));
	}
}

void PotatoClient::OnTempArenaInfoChanged(const std::filesystem::path& file, const GameInfo& game)
{
	PA_TRY_OR_ELSE(arenaInfo, ReadArenaInfo(file),
	{
		LOG_ERROR("Failed to read arena info from file: {}", error);
		emit StatusReady(Status::Error, "Reading ArenaInfo");
		return;
	});

	if (!File::Exists(file) || !arenaInfo)
	{
		LOG_TRACE("Arena info does not exist when directory changed.");
		return;
	}

	// make sure we don't pull the same match twice
	if (arenaInfo->Hash == m_lastArenaInfoHash)
		return;
	m_lastArenaInfoHash = arenaInfo->Hash;

	if (game.Region.empty())
	{
		LOG_ERROR("No region set in DirectoryStatus");
		emit StatusReady(Status::Error, "No Region Set");
		return;
	}

	const std::string request = BuildRequest(game.Region, arenaInfo->Meta.PlayerName, arenaInfo->Json);

	emit StatusReady(Status::Loading, "Loading");

	SendRequest(request, MatchContext
	{
		.ArenaInfo = arenaInfo->String,
		.PlayerName = arenaInfo->Meta.PlayerName,
		.ShipIdent = arenaInfo->Meta.PlayerVehicle
	});
}

fs::path PotatoClient::GetGameFilePath(const GameInfo& game) const
{
	return m_services.Get<AppDirectories>().GameFilesDir / game.Region / game.GameVersion.ToString(".", true);
}

fs::path PotatoClient::GetGameFilePath(const GameInfo& game, Version version) const
{
	return m_services.Get<AppDirectories>().GameFilesDir / game.Region / version.ToString(".", true);
}

void PotatoClient::UpdateGameInstalls()
{
	m_gameInfos.clear();
	m_watcher.ClearDirectories();

	AppDirectories appDirs = m_services.Get<AppDirectories>();

	for (const fs::path& game : m_services.Get<Config>().Get<ConfigKey::GameDirectories>())
	{
		const Result<GameInfo> gameInfo = Game::ReadGameInfo(game);
		if (gameInfo)
		{
			m_gameInfos.emplace_back(GameDirectory
			{
				.Path = game,
				.Status = "Found",  // TODO: localize
				.Info = *gameInfo,
			});
		}
		else
		{
			LOG_ERROR(STR("Failed to read game info from {}: {}"), game, StringWrap(gameInfo.error().message()));
			m_gameInfos.emplace_back(GameDirectory
			{
				.Path = game,
				.Status = "Not A WoWs Directory",  // TODO: localize
				.Info = std::nullopt,
			});
			continue;
		}

		// start watching all replays paths
		for (const fs::path& folder : gameInfo->ReplaysPaths)
		{
			if (!folder.empty())
			{
				m_watcher.WatchDirectory(folder);
			}
		}

		// make sure we have up-to-date game files
		const std::string gameVersion = gameInfo->GameVersion.ToString(".", true);
		const fs::path gameFilePath = GetGameFilePath(*gameInfo);
		if (!ReplayAnalyzer::HasGameFiles(gameFilePath))
		{
			LOG_INFO("Game files for Client ({}, {}) missing, unpacking...", gameInfo->Region, gameVersion);
			const Result<void> unpackResult = ReplayAnalyzer::UnpackGameFiles(gameFilePath, gameInfo->PkgPath, gameInfo->IdxPath);
			if (!unpackResult)
			{
				LOG_ERROR("Failed to unpack game files for version '{}': {}", gameVersion, unpackResult.error().message());
			}
		}
		else
		{
			LOG_INFO("Game files for Client ({}, {}) found", gameInfo->Region, gameVersion);
		}

		// let's check the entire game folder, replays might be hiding everywhere
		m_replayAnalyzer.AnalyzeDirectory(game);
	}

	emit GameInfosChanged(m_gameInfos);
	emit StatusReady(Status::Ready, "Ready");
	TriggerRun();
}
