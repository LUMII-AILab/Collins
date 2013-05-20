#!/usr/bin/env python
# coding=utf-8

from tokenize import CoNLLizator
from parse import Parser
import conll2json

print 'Starting...'

conllizator = CoNLLizator("../LVTagger/morphotagger.sh")
parser = Parser(command="build/collins", args="--load --stdin --stdout -q --basedir=build")

print 'OK'
print

# gevent.monkey.patch_all() rada problēmas ar pipe, tāpēc ielādē tos pēc pipe izveidošanas


from gevent import monkey; monkey.patch_all()
from time import sleep
import os
from bottle import route, run, static_file, post, request, response


@route('/rest')
def rest():
    return 'REST API goes here'

@post('/rest/parse')
def parse():
    response.content_type = 'text/html; charset=utf-8'
    conll = request.body.getvalue()
    while parser.inProgress:
        sleep(0.1)
    return parser(conll)


@post('/rest/conllize')
def CoNLLize():
    response.content_type = 'text/html; charset=utf-8'
    sentence = request.body.getvalue()
    while conllizator.inProgress:
        sleep(0.1)
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


# run(host='localhost', port=8080, debug=True)
# run(host='0.0.0.0', port=8080, debug=True)
run(host='0.0.0.0', port=8080, debug=True, server='gevent')
