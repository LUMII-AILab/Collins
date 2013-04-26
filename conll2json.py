#!/usr/bin/env python
# coding=utf-8

import json

def convert(conll):

    data = [dict(id=0, name='*', data=dict(lemma='*', tag='R', parentID=-1), children=[])]

    for line in conll.split('\n'):
        if not line:
            continue
        parts = line.split('\t')
        data.append(dict(id=int(parts[0]), name=parts[1], data=dict(lemma=parts[2], tag=parts[4], parentID=int(parts[6])), children=[]))


    for child in data:
        if child['data']['parentID'] == -1:
            continue
        data[child['data']['parentID']]['children'].append(child)

    return json.dumps(data[0], indent=2)


if __name__ == "__main__":

    conll = """1	Prasīs	prast	v	vmnift130an	_	7
2	Rāviņa	Rāviņa	n	n_msg_	_	7
3	skaidrojumus	skaidrojums	n	n_mpa1	_	4
4	par	par	s	sppdn	_	2
5	Valsts	valsts	n	ncfsg6	_	2
6	kontroles	kontrole	n	ncfsg5	_	2
7	atklātajiem	atklāt	v	vmnpdmpdpsy	_	0
8	pārkāpumiem	pārkāpums	n	ncmpd1	_	0
9	Jelgavā	Jelgava	n	npfsl4	_	0
"""

    print convert(conll)
