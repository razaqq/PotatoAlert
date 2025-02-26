#![no_main]

use std::collections::HashMap;
use cxx::{CxxVector, UniquePtr};
use num_derive::{FromPrimitive, ToPrimitive};
use num_traits::FromPrimitive;
use serde_pickle::{DeOptions, Error, Value};


#[cxx::bridge(namespace = "PotatoAlert::ReplayAnalyzer")]
mod ffi
{
	pub struct ReceiveDamageStatResult
	{
		#[cxx_name = "IsError"]
		pub is_error: bool,
		#[cxx_name = "Error"]
		pub error: String,
		#[cxx_name = "Value"]
		pub value: Vec<ReceiveDamageStat>,
	}

	#[derive(Default)]
	pub struct ReceiveDamageStat
	{
		#[cxx_name = "DamageType"]
		pub damage_type: i64,
		#[cxx_name = "DamageFlag"]
		pub damage_flag: i64,
		#[cxx_name = "Hits"]
		pub hits: i64,
		#[cxx_name = "Damage"]
		pub damage: f32,
	}

	pub struct OnArenaStateReceivedPlayerResult
	{
		#[cxx_name = "IsError"]
		pub is_error: bool,
		#[cxx_name = "Error"]
		pub error: String,
		#[cxx_name = "Value"]
		pub value: UniquePtr<CxxVector<OnArenaStateReceivedPlayer>>,
	}

	#[derive(Debug, Serialize)]
	pub struct KeyValuePair
	{
		#[cxx_name = "Key"]
		pub key: String,
		#[cxx_name = "Value"]
		pub value: String,
	}

	#[derive(Debug, Default, Serialize)]
	pub struct OnArenaStateReceivedPlayer
	{
		#[cxx_name = "EntityId"]
		pub entity_id: i64,
		#[cxx_name = "Id"]
		pub id: i64,
		#[cxx_name = "IsBot"]
		pub is_bot: bool,
		#[cxx_name = "ShipId"]
		pub ship_id: i64,
		#[cxx_name = "SkinId"]
		pub skin_id: i64,
		#[cxx_name = "TeamId"]
		pub team_id: i64,
		#[cxx_name = "ShipParamsId"]
		pub ship_params_id: i64,
		#[cxx_name = "MaxHealth"]
		pub max_health: i64,
		#[cxx_name = "ShipComponents"]
		pub ship_components: Vec<KeyValuePair>,
		#[cxx_name = "Name"]
		pub name: String,
		#[cxx_name = "ClanTag"]
		pub clan_tag: String,
		#[cxx_name = "ClanColor"]
		pub clan_color: i64,
	}

	#[derive(Debug, Default, Serialize)]
	pub struct SetConsumable
	{
		#[cxx_name = "Id"]
		pub id: i64,
		#[cxx_name = "Unknown1"]
		pub u1: i64,
		#[cxx_name = "Count"]
		pub count: i64,
		#[cxx_name = "Unknown2"]
		pub u2: bool,
		#[cxx_name = "Unknown3"]
		pub u3: f64,
	}

	pub struct SetConsumablesResult
	{
		#[cxx_name = "IsError"]
		pub is_error: bool,
		#[cxx_name = "Error"]
		pub error: String,
		#[cxx_name = "Value"]
		pub value: Vec<SetConsumable>,
	}

	extern "Rust"
	{
		#[cxx_name = "ParseReceiveDamageStat"]
		fn parse_receive_damage_stat(data: &CxxVector<u8>) -> ReceiveDamageStatResult;

		#[cxx_name = "ParseArenaStateReceivedPlayers"]
		fn parse_arena_state_received_players(data: &CxxVector<u8>, version: u32) -> OnArenaStateReceivedPlayerResult;

		#[cxx_name = "ParseArenaStateReceivedBots"]
		fn parse_arena_state_received_bots(data: &CxxVector<u8>, version: u32) -> OnArenaStateReceivedPlayerResult;

		#[cxx_name = "ParseSetConsumables"]
		fn parse_set_consumables(data: &CxxVector<u8>, version: u32) -> SetConsumablesResult;
	}
}

use ffi::{KeyValuePair, OnArenaStateReceivedPlayer, OnArenaStateReceivedPlayerResult, ReceiveDamageStat, ReceiveDamageStatResult, SetConsumable, SetConsumablesResult};

pub trait ErrorType<T, E>
{
	fn new_error(err: E) -> Self;
	fn new_error_from_str(err: String) -> Self;
	fn new_result(res: T) -> Self;
}

impl ErrorType<Vec<ReceiveDamageStat>, Error> for ReceiveDamageStatResult
{
	fn new_error(err: Error) -> Self
	{
		Self
		{
			is_error: true,
			error: err.to_string(),
			value: vec![]
		}
	}

	fn new_error_from_str(err: String) -> Self
	{
		Self
		{
			is_error: true,
			error: err,
			value: vec![]
		}
	}

	fn new_result(res: Vec<ReceiveDamageStat>) -> Self
	{
		Self
		{
			is_error: false,
			error: "".to_string(),
			value: res
		}
	}
}

impl ErrorType<UniquePtr<CxxVector<OnArenaStateReceivedPlayer>>, Error> for OnArenaStateReceivedPlayerResult
{
	fn new_error(err: Error) -> Self
	{
		Self
		{
			is_error: true,
			error: err.to_string(),
			value: UniquePtr::null(),
		}
	}

	fn new_error_from_str(err: String) -> Self
	{
		Self
		{
			is_error: true,
			error: err,
			value: UniquePtr::null()
		}
	}

	fn new_result(res: UniquePtr<CxxVector<OnArenaStateReceivedPlayer>>) -> Self
	{
		Self
		{
			is_error: false,
			error: "".to_string(),
			value: res,
		}
	}
}

impl ErrorType<Vec<SetConsumable>, Error> for SetConsumablesResult
{
	fn new_error(err: Error) -> Self
	{
		Self
		{
			is_error: true,
			error: err.to_string(),
			value: vec![]
		}
	}

	fn new_error_from_str(err: String) -> Self
	{
		Self
		{
			is_error: true,
			error: err,
			value: vec![]
		}
	}

	fn new_result(res: Vec<SetConsumable>) -> Self
	{
		Self
		{
			is_error: false,
			error: "".to_string(),
			value: res
		}
	}
}

fn parse_receive_damage_stat(data: &CxxVector<u8>) -> ReceiveDamageStatResult
{
	type DamageStat = HashMap<(i64, i64), (i64, f32)>;

	match serde_pickle::de::value_from_slice(data.as_slice(), DeOptions::default())
	{
		Ok(val) => {
			match serde_pickle::value::from_value::<DamageStat>(val)
			{
				Ok(ds) => {
					let stats = ds.into_iter()
						.map(|((dmg_type, dmg_flag), (count, dmg))|
							ReceiveDamageStat
							{
								damage_type: dmg_type,
								damage_flag: dmg_flag,
								hits: count,
								damage: dmg
							}
						).collect::<Vec<ReceiveDamageStat>>();
					ReceiveDamageStatResult::new_result(stats)
				}
				Err(err) => {
					ReceiveDamageStatResult::new_error(err)
				}
			}
		}
		Err(err) => {
			ReceiveDamageStatResult::new_error(err)
		}
	}
}

#[allow(non_camel_case_types)]
#[derive(Clone, FromPrimitive)]
#[repr(i64)]
enum PlayerDataIndex_0_11_11  // version > 0.11.11
{
	AccountDbId          = 0x00,  // i64
	AntiAbuseEnabled     = 0x01,  // bool
	EntityId             = 0x02,  // i64  // TODO: this is really the avatarId
	CamoInfo             = 0x03,  // Dict
	ClanColor            = 0x04,  // i64
	ClanId               = 0x05,  // i64
	ClanTag              = 0x06,  // String
	CrewParams           = 0x07,  // Vec<i64 | List<i64>>
	DogTag               = 0x08,  // Vec<i64>
	FragsCount           = 0x09,  // i64
	FriendlyFireEnabled  = 0x0A,  // bool
	Id                   = 0x0B,  // i64
	InvitationsEnabled   = 0x0C,  // bool
	IsAbuser             = 0x0D,  // bool
	IsAlive              = 0x0E,  // bool
	IsBot                = 0x0F,  // bool
	IsClientLoaded       = 0x10,  // bool
	IsConnected          = 0x11,  // bool
	IsHidden             = 0x12,  // bool
	IsLeaver             = 0x13,  // bool
	IsPreBattleOwner     = 0x14,  // bool
	IsTShooter           = 0x15,  // bool
	KeyTargetMarkers     = 0x16,  // i64
	KilledBuildingsCount = 0x17,  // i64
	MaxHealth            = 0x18,  // i64
	Name                 = 0x19,  // Vec<Byte> -> String
	PlayerMode           = 0x1A,  // Dict<Vec<Bytes>, Vec<Bytes>>
	PreBattleIdOnStart   = 0x1B,  // i64
	PreBattleSign        = 0x1C,  // i64
	PreBattleId          = 0x1D,  // i64
	Realm                = 0x1E,  // Vec<Bytes> -> String
	ShipComponents       = 0x1F,  // Dict<Vec<Bytes>, Vec<Bytes>>
	ShipConfigDump       = 0x20,  // Vec<byte>
	ShipId               = 0x21,  // i64
	ShipParamsId         = 0x22,  // i64
	SkinId               = 0x23,  // i64
	TeamId               = 0x24,  // i64
	TtkStatus            = 0x25,  //  bool
}

// # ModsShell.API_v_1_0.battleGate.PlayersInfo.gSharedBotInfo._numMemberMap
#[allow(non_camel_case_types)]
#[derive(Clone, FromPrimitive)]
#[repr(i64)]
enum BotDataIndex  // since unknown version
{
	AccountDbId          = 0x00,  // i64
	AntiAbuseEnabled     = 0x01,  // bool
	CamoInfo             = 0x02,  // Dict
	ClanColor            = 0x03,  // i64
	ClanId               = 0x04,  // i64
	ClanTag              = 0x05,  // String
	CrewParams           = 0x06,  // Vec<i64 | List<i64>>
	DogTag               = 0x07,  // Vec<i64>
	FragsCount           = 0x08,  // i64
	FriendlyFireEnabled  = 0x09,  // bool
	Id                   = 0x0A,  // i64
	IsAbuser             = 0x0B,  // bool
	IsAlive              = 0x0C,  // bool
	IsBot                = 0x0D,  // bool
	IsHidden             = 0x0E,  // bool
	IsTShooter           = 0x0F,  // bool
	KilledBuildingsCount = 0x10,  // i64
	KeyTargetMarkers     = 0x11,  // i64
	MaxHealth            = 0x12,  // i64
	Name                 = 0x13,  // Vec<Byte> -> String
	Realm                = 0x14,  // Vec<Bytes> -> String
	ShipComponents       = 0x15,  // Dict<Vec<Bytes>, Vec<Bytes>>
	ShipConfigDump       = 0x16,  // Vec<byte>
	ShipId               = 0x17,  // i64
	ShipParamsId         = 0x18,  // i64
	SkinId               = 0x19,  // i64
	TeamId               = 0x1A,  // i64
	TtkStatus            = 0x1B,  // bool
}


#[allow(non_camel_case_types)]
#[derive(Clone, FromPrimitive)]
#[repr(i64)]
enum PlayerDataIndex_0_10_9  // version > 0.10.9
{
	AccountDbId          = 0x00,
	AntiAbuseEnabled     = 0x01,
	EntityId             = 0x02,
	CamoInfo             = 0x03,
	ClanColor            = 0x04,
	ClanId               = 0x05,
	ClanTag              = 0x06,
	CrewParams           = 0x07,
	DogTag               = 0x08,
	FragsCount           = 0x09,
	FriendlyFireEnabled  = 0x0A,
	Id                   = 0x0B,
	InvitationsEnabled   = 0x0C,
	IsAbuser             = 0x0D,
	IsAlive              = 0x0E,
	IsBot                = 0x0F,
	IsClientLoaded       = 0x10,
	IsConnected          = 0x11,
	IsHidden             = 0x12,
	IsLeaver             = 0x13,
	IsPreBattleOwner     = 0x14,
	IsTShooter           = 0x15,
	KilledBuildingsCount = 0x16,
	MaxHealth            = 0x17,
	Name                 = 0x18,
	PlayerMode           = 0x19,
	PreBattleIdOnStart   = 0x1A,
	PreBattleSign        = 0x1B,
	PreBattleId          = 0x1C,
	Realm                = 0x1D,
	ShipComponents       = 0x1E,
	ShipConfigDump       = 0x1F,
	ShipId               = 0x20,
	ShipParamsId         = 0x21,
	SkinId               = 0x22,
	TeamId               = 0x23,
	TtkStatus            = 0x24,
}

#[allow(non_camel_case_types)]
#[derive(Clone, FromPrimitive)]
#[repr(i64)]
enum PlayerDataIndex_0_10_6  // version > 0.10.6, for some reason there exist some replays with a bool as 0x01
{
	AccountDbId          = 0x00,
	EntityId             = 0x01,
	CamoInfo             = 0x02,
	ClanColor            = 0x03,
	ClanId               = 0x04,
	ClanTag              = 0x05,
	CrewParams           = 0x06,
	DogTag               = 0x07,
	FragsCount           = 0x08,
	FriendlyFireEnabled  = 0x09,
	Id                   = 0x0A,
	InvitationsEnabled   = 0x0B,
	IsAbuser             = 0x0C,
	IsAlive              = 0x0D,
	IsBot                = 0x0E,
	IsClientLoaded       = 0x0F,
	IsConnected          = 0x10,
	IsHidden             = 0x11,
	IsLeaver             = 0x12,
	IsPreBattleOwner     = 0x13,
	IsTShooter           = 0x14,
	KilledBuildingsCount = 0x15,
	MaxHealth            = 0x16,
	Name                 = 0x17,
	PlayerMode           = 0x18,
	PreBattleIdOnStart   = 0x19,
	PreBattleSign        = 0x1A,
	PreBattleId          = 0x1B,
	Realm                = 0x1C,
	ShipComponents       = 0x1D,
	ShipId               = 0x1E,
	ShipParamsId         = 0x1F,
	SkinId               = 0x20,
	TeamId               = 0x21,
	TtkStatus            = 0x22,
}

#[allow(non_camel_case_types)]
#[derive(Clone, FromPrimitive, ToPrimitive)]
#[repr(i64)]
enum PlayerDataIndex_Pre_0_10_6  // version <= 0.10.6
{
	AccountDbId          = 0x00,
	EntityId             = 0x01,
	CamoInfo             = 0x02,
	ClanColor            = 0x03,
	ClanId               = 0x04,
	ClanTag              = 0x05,
	CrewParams           = 0x06,
	DogTag               = 0x07,
	FragsCount           = 0x08,
	FriendlyFireEnabled  = 0x09,
	Id                   = 0x0A,
	InvitationsEnabled   = 0x0B,
	IsAbuser             = 0x0C,
	IsAlive              = 0x0D,
	IsBot                = 0x0E,
	IsClientLoaded       = 0x0F,
	IsConnected          = 0x10,
	IsHidden             = 0x11,
	IsLeaver             = 0x12,
	IsPreBattleOwner     = 0x13,
	KilledBuildingsCount = 0x14,
	MaxHealth            = 0x15,
	Name                 = 0x16,
	PlayerMode           = 0x17,
	PreBattleIdOnStart   = 0x18,
	PreBattleSign        = 0x19,
	PreBattleId          = 0x1A,
	Realm                = 0x1B,
	ShipComponents       = 0x1C,
	ShipId               = 0x1D,
	ShipParamsId         = 0x1E,
	SkinId               = 0x1F,
	TeamId               = 0x20,
	TtkStatus            = 0x21,
}

macro_rules! parse_arena_state_player
{
	($data_index:ident, $players:expr, $data:expr) =>
	{
		match serde_pickle::de::value_from_slice($data.as_slice(), DeOptions::default())
		{
			Ok(val) => {
				match serde_pickle::value::from_value::<Vec<Player>>(val)
				{
					Ok(players) => {
						for player in players
						{
							let mut p = OnArenaStateReceivedPlayer::default();
							for (idx, value) in player
							{
								match FromPrimitive::from_i64(idx)
								{
									Some($data_index::EntityId) => {
										get_pickle!(i64, value, p.entity_id);
									}
									Some($data_index::ShipId) => {
										get_pickle!(i64, value, p.ship_id);
									}
									Some($data_index::SkinId) => {
										get_pickle!(i64, value, p.skin_id);
									}
									Some($data_index::TeamId) => {
										get_pickle!(i64, value, p.team_id);
									}
									Some($data_index::Id) => {
										get_pickle!(i64, value, p.id);
									}
									Some($data_index::ShipParamsId) => {
										get_pickle!(i64, value, p.ship_params_id);
									}
									Some($data_index::MaxHealth) => {
										get_pickle!(i64, value, p.max_health);
									}
									Some($data_index::Name) => {
										get_pickle!(String, value, p.name);
									}
									Some($data_index::ClanTag) => {
										get_pickle!(String, value, p.clan_tag);
									}
									Some($data_index::ClanColor) => {
										get_pickle!(i64, value, p.clan_color);
									}
									Some($data_index::ShipComponents) => {
										#[allow(unused_assignments)]
										let mut map = HashMap::<serde_bytes::ByteBuf, serde_bytes::ByteBuf>::new();
										get_pickle!(HashMap<serde_bytes::ByteBuf, serde_bytes::ByteBuf>, value, map);
										p.ship_components.reserve(map.keys().len());
										for (k, v) in map
										{
											match (std::str::from_utf8(k.as_slice()), std::str::from_utf8(v.as_slice()))
											{
												(Ok(k_str), Ok(v_str)) => {
													p.ship_components.push(KeyValuePair{key: k_str.to_string(), value: v_str.to_string()});
												}
												_ => {
													return OnArenaStateReceivedPlayerResult::new_error_from_str("ShipComponents map k, v are not ascii".to_string());
												}
											}
										}
									}
									None => { continue; }
									_ => { continue; }
								}
							}
							$players.pin_mut().push(p);
						}
						OnArenaStateReceivedPlayerResult::new_result($players)
					}
					Err(err) => {
						OnArenaStateReceivedPlayerResult::new_error(err)
					}
				}
			}
			Err(err) => {
				OnArenaStateReceivedPlayerResult::new_error(err)
			}
		}
	}
}

macro_rules! parse_arena_state_bot
{
	($data_index:ident, $players:expr, $data:expr) =>
	{
		match serde_pickle::de::value_from_slice($data.as_slice(), DeOptions::default())
		{
			Ok(val) => {
				match serde_pickle::value::from_value::<Vec<Player>>(val)
				{
					Ok(players) => {
						for player in players
						{
							let mut p = OnArenaStateReceivedPlayer::default();
							for (idx, value) in player
							{
								match FromPrimitive::from_i64(idx)
								{
									Some($data_index::ShipId) => {
										get_pickle!(i64, value, p.ship_id);
									}
									Some($data_index::SkinId) => {
										get_pickle!(i64, value, p.skin_id);
									}
									Some($data_index::TeamId) => {
										get_pickle!(i64, value, p.team_id);
									}
									Some($data_index::Id) => {
										get_pickle!(i64, value, p.id);
									}
									Some($data_index::ShipParamsId) => {
										get_pickle!(i64, value, p.ship_params_id);
									}
									Some($data_index::MaxHealth) => {
										get_pickle!(i64, value, p.max_health);
									}
									Some($data_index::Name) => {
										get_pickle!(String, value, p.name);
									}
									Some($data_index::ClanTag) => {
										get_pickle!(String, value, p.clan_tag);
									}
									Some($data_index::ClanColor) => {
										get_pickle!(i64, value, p.clan_color);
									}
									Some($data_index::ShipComponents) => {
										#[allow(unused_assignments)]
										let mut map = HashMap::<serde_bytes::ByteBuf, serde_bytes::ByteBuf>::new();
										get_pickle!(HashMap<serde_bytes::ByteBuf, serde_bytes::ByteBuf>, value, map);
										p.ship_components.reserve(map.keys().len());
										for (k, v) in map
										{
											match (std::str::from_utf8(k.as_slice()), std::str::from_utf8(v.as_slice()))
											{
												(Ok(k_str), Ok(v_str)) => {
													p.ship_components.push(KeyValuePair{key: k_str.to_string(), value: v_str.to_string()});
												}
												_ => {
													return OnArenaStateReceivedPlayerResult::new_error_from_str("ShipComponents map k, v are not ascii".to_string());
												}
											}
										}
									}
									None => { continue; }
									_ => { continue; }
								}
							}
							$players.pin_mut().push(p);
						}
						OnArenaStateReceivedPlayerResult::new_result($players)
					}
					Err(err) => {
						OnArenaStateReceivedPlayerResult::new_error(err)
					}
				}
			}
			Err(err) => {
				OnArenaStateReceivedPlayerResult::new_error(err)
			}
		}
	}
}

macro_rules! get_pickle
{
	($t:ty, $value:expr, $target:expr) => {
		match serde_pickle::value::from_value::<$t>($value)
		{
			Ok(v) => { $target = v; }
			Err(err) => { return OnArenaStateReceivedPlayerResult::new_error(err); }
		}
	}
}

fn create_version(major: u8, minor: u8, patch: u8, build: u8) -> u32
{
	((major as u32) << 0x18) + ((minor as u32) << 0x10) + ((patch as u32) << 0x08) + (build as u32)
}

fn parse_arena_state_received_players(data: &CxxVector<u8>, version: u32) -> OnArenaStateReceivedPlayerResult
{
	// let mut out_players: Vec<OnArenaStateReceivedPlayer> = vec![];
	let mut out_players = CxxVector::<OnArenaStateReceivedPlayer>::new();

	type Player = Vec<(i64, Value)>;

	// let val = serde_pickle::de::value_from_slice(data.as_slice(), DeOptions::default()).unwrap();
	// let players = serde_pickle::value::from_value::<Vec<Player>>(val).unwrap();
	// for player in players
	// {
	// 	println!("{:?}", player);
	// }

	if version >= create_version(0, 11, 11, 0)
	{
		parse_arena_state_player!(PlayerDataIndex_0_11_11, out_players, data)
	}
	else if version >= create_version(0, 10, 9, 0)
	{
		parse_arena_state_player!(PlayerDataIndex_0_10_9, out_players, data)
	}
	else if version > create_version(0, 10, 6, 0)
	{
		parse_arena_state_player!(PlayerDataIndex_0_10_6, out_players, data)
	}
	else // if version <= create_version(0, 10, 6, 0)
	{
		parse_arena_state_player!(PlayerDataIndex_Pre_0_10_6, out_players, data)
	}
}

fn parse_arena_state_received_bots(data: &CxxVector<u8>, _version: u32) -> OnArenaStateReceivedPlayerResult
{
	let mut out_players = CxxVector::<OnArenaStateReceivedPlayer>::new();

	type Player = Vec<(i64, Value)>;

	parse_arena_state_bot!(BotDataIndex, out_players, data)
}

macro_rules! get_or_error
{
	($t:ty, $value:expr, $target:expr) => {
		match serde_pickle::value::from_value::<$t>($value)
		{
			Ok(v) => { $target = v; }
			Err(err) => { return SetConsumablesResult::new_error(err); }
		}
	}
}

fn parse_set_consumables_tuple(value: Value) -> SetConsumablesResult
{
	match value
	{
		Value::List(consumables_values) => {
			let mut cons = vec![];
			for consumable_value in consumables_values
			{
				match consumable_value
				{
					Value::Tuple(mut tuple_values) => {
						if tuple_values.len() != 2
						{
							return SetConsumablesResult::new_error_from_str("consumable tuple has length != 2 (id, value_tuple)".to_string());
						}
						let mut con = SetConsumable::default();
						get_or_error!(i64, tuple_values.remove(0), con.id);

						match tuple_values.remove(0)
						{
							Value::Tuple(mut con_values) => {
								if con_values.len() < 4
								{
									return SetConsumablesResult::new_error_from_str("consumable tuple has length != 4 (id, value_tuple)".to_string());
								}
								get_or_error!(i64, con_values.remove(0), con.u1);
								get_or_error!(i64, con_values.remove(0), con.count);
								get_or_error!(bool, con_values.remove(0), con.u2);
								get_or_error!(f64, con_values.remove(0), con.u3);
							},
							_ => {
								return SetConsumablesResult::new_error_from_str("consumable value is not a tuple".to_string());
							}
						}

						cons.push(con);
					},
					_ => {
						return SetConsumablesResult::new_error_from_str("consumable value is not a tuple".to_string());
					}
				}
			}
			SetConsumablesResult::new_result(cons)
		},
		_ => {
			return SetConsumablesResult::new_error_from_str("value is not a list".to_string());
		},
	}
}

fn parse_set_consumables(data: &CxxVector<u8>, version: u32) -> SetConsumablesResult
{
	match serde_pickle::de::value_from_slice(data.as_slice(), DeOptions::default())
	{
		Ok(value) => {
			if version >= create_version(13, 2, 0, 0)
			{
				match value
				{
					Value::Dict(map) => {
						let m = map.into_iter().map(|(k, v)| -> Result<(String, Value), Error> {
							let key = serde_pickle::value::from_value::<String>(k.into_value())?;
							Ok((key, v))
						}).collect::<Result<HashMap<String, Value>, Error>>();
						if m.is_err()
						{
							return SetConsumablesResult::new_error(m.unwrap_err());
						}
						match m.unwrap().remove("consumablesDict")
						{
							Some(dict) => {
								parse_set_consumables_tuple(dict)
							},
							None => SetConsumablesResult::new_error_from_str("no such key 'consumablesDict'".to_string())
						}
					},
					_ => {
						return SetConsumablesResult::new_error_from_str("value is not a dict".to_string());
					}
				}
			}
			else
			{
				parse_set_consumables_tuple(value)
			}
		}
		Err(err) => {
			SetConsumablesResult::new_error(err)
		}
	}
}
