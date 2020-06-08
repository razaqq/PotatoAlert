import uuid
import platform
import os
import logging
from version import __version__

from aioga import GA

TRACKING_ID = 'UA-151171454-1'


def get_useragent():
    if os.name == 'nt':
        ua = f"Mozilla/5.0 (Windows NT {platform.release()}; {platform.machine().replace('AMD', 'Win')}; " \
             f"{platform.machine().replace('AMD', 'x')}; rv:72.0) PotatoAlert/{__version__}"
    elif os.name == 'posix':
        ua = f"Mozilla/5.0 (Linux x86_64) PotatoAlert/{__version__}"
    else:
        ua = f"Mozilla/5.0 ({platform.release()}) PotatoAlert/{__version__}"
    return ua


async def run_ga():
    cid = uuid.UUID(int=uuid.getnode())

    try:
        async with GA(TRACKING_ID) as ga:
            ga.event(str(cid), ec='start', ea='launched app', ua=get_useragent())
    except Exception as e:
        logging.exception(e)
