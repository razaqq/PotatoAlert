from requests import get
from apiwrapper.errors import InvalidApplicationIdError
from requests.exceptions import RequestException


class ApiWrapper:
    def __init__(self, api_key, region):
        self.api_key = api_key
        http_ending = region if region != 'na' else 'com'
        self.endpoint = f'https://api.worldofwarships.{http_ending}/wows/{{}}/{{}}/?'

    def get_result(self, method_block: str, method_name: str, params: dict) -> dict:
        try:
            params = {k: v for k, v in params.items() if v or isinstance(v, int)}
            url = self.endpoint.format(method_block, method_name)
            params['application_id'] = self.api_key
            res = get(url, params=params)
            res = res.json()
            if res['status'] == 'error':
                if res['error']['code'] == 407 and res['error']['message'] == 'INVALID_APPLICATION_ID':
                    raise InvalidApplicationIdError
            return res
        except RequestException:
            print('Connection Error')

    def search_account(self, name) -> dict:
        param = {
            'search': name,
            'type': 'exact',
            'fields': 'account_id'
        }
        return self.get_result('account', 'list', param)

    def get_ship_stats(self, account_id: int, ship_id: int = 0) -> dict:
        param = {
            'account_id': account_id,
            'fields': 'pvp.battles,pvp.wins,pvp.damage_dealt',
            'ship_id': ship_id
        }
        return self.get_result('ships', 'stats', param)

    def get_account_info(self, account_id: int) -> dict:
        param = {
            'account_id': account_id,
            'fields': 'hidden_profile,statistics.pvp.battles,statistics.pvp.losses,statistics.pvp.survived_battles,'
                      'statistics.pvp.wins,statistics.pvp.damage_dealt,statistics.pvp.frags,statistics.pvp.draws,'
                      'statistics.pvp.xp',
        }
        return self.get_result('account', 'info', param)

    def get_player_clan(self, account_id: int) -> dict:
        param = {
            'account_id': account_id
        }
        return self.get_result('clans', 'accountinfo', param)

    def get_clan_info(self, clan_id: int) -> dict:
        param = {
            'clan_id': clan_id,
            'fields': 'tag,members_count'
        }
        return self.get_result('clans', 'info', param)

    def get_ship_infos(self, ship_id: int) -> dict:
        param = {
            'ship_id': ship_id,
            'fields': 'name,tier,type,nation'
        }
        return self.get_result('encyclopedia', 'ships', param)
