#!/usr/bin/env python3

import xml.etree.ElementTree as ET


if __name__ == '__main__':
    languages = []
    keys = []
    translations = {}

    tree = ET.parse('stringtable.xml')
    root = tree.getroot()

    # generate languages enum
    for lang in root[0]:
        languages.append(lang.tag)
        translations[lang.tag] = []
    with open('StringTableLanguages.i', 'w') as f:
        f.write(','.join([f'"{l}"' for l in languages]))

    # generate strings
    for child in root:
        keys.append(child.attrib['ID'])
        for lang in child:
            translations[lang.tag].append(lang.text)
    strings = []
    for lang in languages:
        strings.append('{' + ','.join([f'"{t}"' for t in translations[lang]]) + '}')
    with open('StringTableStrings.i', 'w') as f:
        f.write(','.join(strings))

    # generate keys
    with open('StringTableKeys.i', 'w') as f:
        f.write(','.join(keys))