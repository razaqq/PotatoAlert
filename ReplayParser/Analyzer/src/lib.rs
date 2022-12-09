#![no_main]

use std::collections::HashMap;
use cxx::CxxVector;
use num_derive::FromPrimitive;
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
		pub value: Vec<OnArenaStateReceivedPlayer>,
	}

	#[derive(Debug, Default, Clone, Serialize)]
	pub struct OnArenaStateReceivedPlayer
	{
		// pub nickname: String,
		// pub clan: String,
		#[cxx_name = "EntityId"]
		pub entity_id: i64,
		#[cxx_name = "Id"]
		pub id: i64,
		#[cxx_name = "ShipId"]
		pub ship_id: i64,
		#[cxx_name = "SkinId"]
		pub skin_id: i64,
		#[cxx_name = "TeamId"]
		pub team_id: i64,
		// pub health: i64,
	}

	extern "Rust"
	{
		#[cxx_name = "ParseReceiveDamageStat"]
		fn parse_receive_damage_stat(data: &CxxVector<u8>) -> ReceiveDamageStatResult;
		#[cxx_name = "ParseArenaStateReceivedPlayers"]
		fn parse_arena_state_received_players(data: &CxxVector<u8>, version: u32) -> OnArenaStateReceivedPlayerResult;
	}
}

use ffi::{OnArenaStateReceivedPlayer, OnArenaStateReceivedPlayerResult, ReceiveDamageStat, ReceiveDamageStatResult};

pub trait ErrorType<T, E>
{
	fn new_error(err: E) -> Self;
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

impl ErrorType<Vec<OnArenaStateReceivedPlayer>, Error> for OnArenaStateReceivedPlayerResult
{
	fn new_error(err: Error) -> Self
	{
		Self
		{
			is_error: true,
			error: err.to_string(),
			value: vec![],
		}
	}

	fn new_result(res: Vec<OnArenaStateReceivedPlayer>) -> Self
	{
		Self
		{
			is_error: false,
			error: "".to_string(),
			value: res,
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
							ReceiveDamageStat{
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
enum DataIndex_0_11_11  // version > 0.11.11
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

#[allow(non_camel_case_types)]
#[derive(Clone, FromPrimitive)]
#[repr(i64)]
enum DataIndex_0_10_9  // version > 0.10.9
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
enum DataIndex_0_10_6  // version > 0.10.6, for some reason there exist some replays with a bool as 0x01
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
#[derive(Clone, FromPrimitive)]
#[repr(i64)]
enum DataIndex_Pre_0_10_6  // version <= 0.10.6
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

macro_rules! parse_arena_state
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
									// Some($data_index::AvatarId) => {
									// 	get_pickle!(i64, value, p.avatarid);
									// }
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
									None => { continue; }
									_ => { continue; }
								}
							}
							$players.push(p);
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
	let mut out_players: Vec<OnArenaStateReceivedPlayer> = vec![];

	type Player = Vec<(i64, Value)>;

	// let val = serde_pickle::de::value_from_slice(data.as_slice(), DeOptions::default()).unwrap();
	// let players = serde_pickle::value::from_value::<Vec<Player>>(val).unwrap();
	// for player in players
	// {
	// 	println!("{:?}", player);
	// }

	if version >= create_version(0, 11, 11, 0)
	{
		parse_arena_state!(DataIndex_0_11_11, out_players, data)
	}
	else if version >= create_version(0, 10, 9, 0)
	{
		parse_arena_state!(DataIndex_0_10_9, out_players, data)
	}
	else if version > create_version(0, 10, 6, 0)
	{
		parse_arena_state!(DataIndex_0_10_6, out_players, data)
	}
	else // if version <= create_version(0, 10, 6, 0)
	{
		parse_arena_state!(DataIndex_Pre_0_10_6, out_players, data)
	}
}
