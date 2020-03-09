"""
Copyright (c) 2019 razaqq

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import asyncio
from typing import Union, Tuple
from aiohttp import ClientSession, ClientTimeout
from aiohttp.client_exceptions import ClientResponseError, ClientError, ClientConnectionError, ServerTimeoutError
from utils.api_errors import InvalidApplicationIdError
from assets.colors import DGrey


class ApiWrapper:
    def __init__(self, api_key, region):
        self.api_key = api_key
        http_ending = region if region != 'na' else 'com'
        self.endpoint = f'https://api.worldofwarships.{http_ending}/wows/{{}}/{{}}/?'
        self.session = ClientSession(timeout=ClientTimeout(connect=20))

    def __del__(self):
        """
        if not self.session.closed:
            if self.session._connector_owner:
                self.session._connector.close()
            self.session._connector = None
        """
        if not self.session.closed:
            loop = asyncio.get_event_loop()
            loop.create_task(self.session.close())

    async def get_result(self, method_block: str, method_name: str, params: dict) -> dict:
        params = {k: v for k, v in params.items() if v or isinstance(v, int)}
        url = self.endpoint.format(method_block, method_name)
        params['application_id'] = self.api_key

        async with self.session.get(url, params=params) as resp:
            res = await resp.json()

        if res['status'] == 'error':
            if res['error']['code'] == 407 and res['error']['message'] == 'INVALID_APPLICATION_ID':
                raise InvalidApplicationIdError
        return res

    async def search_account(self, name) -> dict:
        param = {
            'search': name,
            'type': 'exact',
            'fields': 'account_id'
        }
        return await self.get_result('account', 'list', param)

    async def get_ship_stats(self, account_id: int, ship_id: int = 0) -> dict:
        param = {
            'account_id': account_id,
            'fields': 'pvp.battles,pvp.wins,pvp.damage_dealt,ship_id,pvp.frags',
            'ship_id': ship_id if ship_id else ''
        }
        return await self.get_result('ships', 'stats', param)

    async def get_account_info(self, account_id: int) -> dict:
        param = {
            'account_id': account_id,
            'fields': 'hidden_profile,statistics.pvp.battles,statistics.pvp.losses,statistics.pvp.survived_battles,'
                      'statistics.pvp.wins,statistics.pvp.damage_dealt,statistics.pvp.frags,statistics.pvp.draws,'
                      'statistics.pvp.xp',
        }
        return await self.get_result('account', 'info', param)

    async def get_player_clan(self, account_id: int) -> dict:
        param = {
            'account_id': account_id,
            'extra': 'clan'
        }
        return await self.get_result('clans', 'accountinfo', param)

    async def get_clan_info(self, clan_id: int) -> dict:
        param = {
            'clan_id': clan_id,
            'fields': 'tag,members_count'
        }
        return await self.get_result('clans', 'info', param)

    async def get_ship_infos(self, ship_id: int) -> dict:
        param = {
            'ship_id': ship_id,
            'fields': 'name,tier,type,nation'
        }
        return await self.get_result('encyclopedia', 'ships', param)


class ClanWrapper:
    async def get_rating(self, clan_id: str) -> Tuple[int, int, int]:
        url = f"https://clans.worldofwarships.eu/clans/wows/{clan_id}/api/claninfo/"
        try:
            async with ClientSession(timeout=ClientTimeout(connect=20)) as s:
                async with s.get(url) as resp:
                    res = await resp.json()
                return self.rgb_from_dec(res['clanview']['wows_ladder']['color'])
        except (KeyError, TypeError, IndexError):
            return DGrey

    @staticmethod
    def rgb_from_dec(dec: int) -> Tuple[int, int, int]:
        blue = dec & 255
        green = (dec >> 8) & 255
        red = (dec >> 16) & 255
        return red, green, blue


class WoWsNumbersWrapper:
    @staticmethod
    async def get_results(url: str) -> Union[dict, None]:
        try:  # Get expected values for pr calculation from wows-numbers
            async with ClientSession(timeout=ClientTimeout(connect=10)) as s:
                async with s.get(url) as resp:
                    return await resp.json()
        except (ClientConnectionError, ClientError, ClientResponseError, TimeoutError, ServerTimeoutError):
            return None

    async def get_expected(self) -> Union[dict, None]:
        return await self.get_results('https://api.wows-numbers.com/personal/rating/expected/json/')

    async def get_color_limits(self) -> Union[dict, None]:
        return await self.get_results('https://api.wows-numbers.com/colors/json/')

