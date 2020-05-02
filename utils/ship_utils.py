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

ship_short_names = {
    'Prinz Eitel Friedrich': 'P. E. Friedrich',
    'Friedrich der Große': 'F. der Große',
    'Admiral Graf Spee': 'A. Graf Spee',
    'Oktyabrskaya Revolutsiya': 'Okt. Revolutsiya',
    'HSF Admiral Graf Spee': 'HSF A. Graf Spee',
    'Jurien de la Gravière': 'J. de la Gravière',
    'Raimondo Montecuccoli': 'Montecuccoli'
}

class_sort = {
    'AirCarrier': 4,
    'Battleship': 3,
    'Cruiser': 2,
    'Destroyer': 1
}

nation_sort = {
    'usa': 10,
    'uk': 9,
    'commonwealth': 8,
    'europe': 7,
    'france': 6,
    'germany': 5,
    'italy': 4,
    'japan': 3,
    'pan-asia': 2,
    'ussr': 1
}


def shorten_name(ship_name: str):
    return ship_short_names.get(ship_name, ship_name)


def get_class_sort(ship_class: str):
    return class_sort[ship_class]


def get_nation_sort(ship_nation: str):
    return nation_sort.get(ship_nation, 0)
