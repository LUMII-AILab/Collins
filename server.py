#!/usr/bin/env python
# coding=utf-8

import os
from bottle import route, run, static_file, post, request, response
from tokenize import CoNLLizator
from parse import Parser
import conll2json

@route('/rest')
def rest():
    return 'REST API goes here'

@post('/rest/parse')
def parse():
    response.content_type = 'text/html; charset=utf-8'
    conll = request.body.getvalue()
    return parser(conll)


@post('/rest/conllize')
def CoNLLize():
    response.content_type = 'text/html; charset=utf-8'
    sentence = request.body.getvalue()
    return conllizator(sentence)

@post('/rest/conll2json')
def CoNLLize():
    response.content_type = 'application/json; charset=utf-8'
    conll = request.body.getvalue()
    return conll2json.convert(conll)

@route('/')
@route('/<path:path>')
def static(path='index.html'):
    return static_file(path, root=os.path.join(os.path.dirname(os.path.realpath(__file__)), 'server'))


conllizator = CoNLLizator("../LVTagger/morphotagger.sh")
parser = Parser(command="build/collins", args="--load --stdin --stdout -q --basedir=build")

run(host='localhost', port=8080, debug=True)
