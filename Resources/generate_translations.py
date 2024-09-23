#!/usr/bin/env python3

import os
from argparse import ArgumentParser
import xml.etree.ElementTree as et


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('-o', '--output_path', type=str, required=True)

    args = parser.parse_args()

    languages = []
    keys = []
    translations = {}

    tree = et.parse(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'stringtable.xml'))
    root = tree.getroot()

    # generate languages enum
    for lang in root[0]:
        languages.append(lang.tag)
        translations[lang.tag] = []
    with open(os.path.join(args.output_path, 'StringTableLanguages.i'), 'w', encoding='utf-8') as f:
        f.write(','.join([f'"{lang}"' for lang in languages]) + '\n')

    # generate strings
    for child in root:
        keys.append(child.attrib['ID'])
        for lang in child:
            translations[lang.tag].append(lang.text)
    strings = []
    for lang in languages:
        strings.append('{' + ','.join([f'"{t}"' for t in translations[lang]]) + '}')
    with open(os.path.join(args.output_path, 'StringTableStrings.i'), 'w', encoding='utf-8') as f:
        f.write(','.join(strings) + '\n')

    # generate keys
    with open(os.path.join(args.output_path, 'StringTableKeys.i'), 'w', encoding='utf-8') as f:
        f.write(','.join(keys) + '\n')
