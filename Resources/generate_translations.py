#!/usr/bin/env python3

import os
import xml.etree.ElementTree as et


if __name__ == '__main__':
    languages = []
    keys = []
    translations = {}

    cur_dir = os.path.dirname(os.path.abspath(__file__))

    tree = et.parse(os.path.join(cur_dir, 'stringtable.xml'))
    root = tree.getroot()

    # generate languages enum
    for lang in root[0]:
        languages.append(lang.tag)
        translations[lang.tag] = []
    with open(os.path.join(cur_dir, 'StringTableLanguages.i'), 'w') as f:
        f.write(','.join([f'"{l}"' for l in languages]))

    # generate strings
    for child in root:
        keys.append(child.attrib['ID'])
        for lang in child:
            translations[lang.tag].append(lang.text)
    strings = []
    for lang in languages:
        strings.append('{' + ','.join([f'"{t}"' for t in translations[lang]]) + '}')
    with open(os.path.join(cur_dir, 'StringTableStrings.i'), 'w') as f:
        f.write(','.join(strings))

    # generate keys
    with open(os.path.join(cur_dir, 'StringTableKeys.i'), 'w') as f:
        f.write(','.join(keys))
