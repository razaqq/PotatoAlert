from typing import List, Tuple
from os import walk, path
from gitignore_parser import parse_gitignore

qrc_prefix = '<RCC>\n    <qresource prefix="/">'
qrc_suffix = '    </qresource>\n</RCC>'
qrc_include = ['.gif', '.qss', '.png', '.svg', '.jpg', '.ttf']


def include(file: str) -> bool:
    for ending in qrc_include:
        if file.endswith(ending):
            return True
    return False


if __name__ == '__main__':
    all_files: List[Tuple[str, str]] = []

    script_root = path.dirname(path.abspath(__file__))

    for root, dirs, files in walk(script_root):
        rel_path = path.relpath(root, script_root)
        rcignore = path.join(root, '.rcignore')
        if path.exists(rcignore):
            matches = parse_gitignore(rcignore)
            for file in files:
                if matches(path.join(root, file)) or not include(file):
                    continue
                else:
                    all_files.append((file, path.join(rel_path if rel_path != '.' else '', file)))
        else:
            all_files.extend([(file, path.join(rel_path if rel_path != '.' else '', file)) for file in files if include(file)])
        pass

    with open('PotatoAlert.qrc', 'w') as f:
        f.write(qrc_prefix)
        f.write('\n')
        for name, path in all_files:
            f.write(f'        <file alias="{name}">{path}</file>\n')
        f.write(qrc_suffix)
