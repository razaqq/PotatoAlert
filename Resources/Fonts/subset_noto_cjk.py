# Creates the NotoSansCJK{SC,TC,JP,KR}-{Regular,Bold}.otf subsets from the upstream Noto Sans CJK collections
# Get Sans/OTC/NotoSansCJK-{Regular,Bold}.ttc (v2.004) from github.com/notofonts/noto-cjk, then run:
#   pip install fonttools
#   python subset_noto_cjk.py <dir with the .ttc files>
#
# Every face keeps the characters of its region's legacy charset, plus the characters used by the translations:
#   SC: GB 2312, TC: Big5 level 1, JP: JIS X 0208 and all kana, KR: all Hangul and KS X 1001 symbols
# Hinting, vertical metrics, the locl feature and the vertical layout features are dropped
import os
import sys
import xml.etree.ElementTree as ET

from fontTools import subset
from fontTools.ttLib import TTFont


def LegacyCharset(encoding, leads, trails):
	chars = set()
	for lead in leads:
		for trail in trails:
			try:
				c = bytes([lead, trail]).decode(encoding)
			except UnicodeDecodeError:
				continue
			if len(c) == 1:
				chars.add(ord(c))
	return chars


def IsHan(c):
	return 0x3400 <= c <= 0x9FFF or 0xF900 <= c <= 0xFAFF


EUC = range(0xA1, 0xFF)
BIG5_TRAILS = list(range(0x40, 0x7F)) + list(range(0xA1, 0xFF))

here = os.path.dirname(os.path.abspath(__file__))
strings = ET.parse(os.path.join(here, '..', 'stringtable.xml')).getroot()
ui = {lang: {ord(c) for e in strings.iter(lang) if e.text for c in e.text} for lang in ('简体中文', '繁體中文', '日本語')}

common = set(range(0x3000, 0x3040)) | set(range(0xFF00, 0xFFF0))
gb2312 = LegacyCharset('gb2312', EUC, EUC)
big5_level1 = LegacyCharset('big5', range(0xA1, 0xC7), BIG5_TRAILS) - LegacyCharset('big5', [0xC6], range(0xA1, 0xFF))
jisx0208 = LegacyCharset('euc_jp', EUC, EUC)
kana = set(range(0x3040, 0x3100)) | set(range(0x31F0, 0x3200)) | set(range(0xFF65, 0xFFA0))
hangul = set(range(0xAC00, 0xD7A4)) | set(range(0x1100, 0x1200)) | set(range(0x3130, 0x3190))
ksx1001 = {c for c in LegacyCharset('euc_kr', EUC, EUC) if not IsHan(c)}

faces = {  # name: (index in the collection, code points)
	'SC': (2, gb2312 | ui['简体中文'] | common),
	'TC': (3, big5_level1 | ui['繁體中文'] | common),
	'JP': (0, jisx0208 | kana | ui['日本語'] | common),
	'KR': (1, hangul | ksx1001 | common),
}

options = subset.Options()
options.layout_features = [f for f in options.layout_features if f not in ('locl', 'valt', 'vert', 'vhal', 'vjmo', 'vkrn', 'vpal', 'vrt2')]
options.drop_tables += ['vhea', 'vmtx', 'VORG']
options.name_IDs = ['*']
options.name_languages = ['*']
options.notdef_outline = True

for weight in ('Regular', 'Bold'):
	for name, (index, chars) in faces.items():
		font = TTFont(os.path.join(sys.argv[1], f'NotoSansCJK-{weight}.ttc'), fontNumber=index)
		subsetter = subset.Subsetter(options)
		subsetter.populate(unicodes=chars)
		subsetter.subset(font)
		font.save(os.path.join(here, f'NotoSansCJK{name}-{weight}.otf'))
