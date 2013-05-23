#!/usr/bin/env python
# coding=utf-8

from tokenize import CoNLLizator
from parse import Parser
from sent_tokenize import SentenceTokenizer
import conll2json

print 'Starting...'

splitter = SentenceTokenizer()
conllizator = CoNLLizator("../LVTagger/morphotagger.sh")
parser = Parser(command="build/collins", args="--load --stdin --stdout -q --basedir=build")

print 'OK'
print

# gevent.monkey.patch_all() rada problēmas ar pipe, tāpēc ielādē tos pēc pipe izveidošanas
# from: http://code.mixpanel.com/2010/10/29/gevent-the-good-the-bad-the-ugly/
# "Order matters."
# "Daemonize before you import gevent or at least before you call monkey.patch_all(). ..."
# "... gevent modifies a socket in python internals. When you daemonize, all open file descriptors are closed ..."



from gevent import monkey; monkey.patch_all()
from time import sleep
import os
from bottle import route, run, static_file, post, request, response
import json
import re


@route('/rest')
def rest():
    return 'REST API goes here'


@post('/rest/split')
def split():
    response.content_type = 'application/json; charset=utf-8'
    text = request.body.read()

    count = 0
    parts = re.split('\r?\n\r?\n', text)
    for part in parts:
        # te ir divi varianti: vai nu uzskata katru rindiņu par atsevišķu teikumu, vai tomēr par veinu (meklē pieturzīmes)
        for sentences in part.split('\n'):
            if not sentences:
                continue
            for sentence in splitter(sentences):
                count += 1
                if count == 1:
                    yield sentence
                else:
                    yield '\n'+sentence

        # bet tad ir jāmaina arī klienta formāts
        # for sentence in splitter(part):
        #     yield sentence.split('\n').join(' ')+'\n'


    # re.split('(?\nko')
    # parts = text.split('\n\n')
    # for sentence in splitter(text):
    #     if not sentence:
    #         continue

    #     yield sentence.split('\n').join(' ')+'\n'
    # return splitter(text)
    # return json.dumps(sentences, indent=2)

# TODO: getvalue() -> read()
@post('/rest/parse')
def parse():
    response.content_type = 'text/html; charset=utf-8'
    conll = request.body.getvalue()
    while parser.inProgress:
        sleep(0.1)
    return parser(conll)

@post('/rest/parse2')
def parse2():
    response.content_type = 'text/html; charset=utf-8'
    try:
        conll = request.body.getvalue()
    except:
        conll = request.body.read()
    while parser.inProgress:
        sleep(0.1)

    # sadala pa teikumiem
    inputSentences = []
    sentence = []
    for line in conll.split('\n'):
        line = line.strip()
        if not line:
            if not sentence:
                continue
            inputSentences.append('\n'.join(inputSentences))
            sentence = []
        sentence.append(line)

    sentences = []
    for sentence in inputSentences:
        sentences.append(parser(sentence))
    return '\n\n'.join(sentences)

@post('/rest/conllize')
def CoNLLize():
    response.content_type = 'text/html; charset=utf-8'
    sentence = request.body.getvalue()
    while conllizator.inProgress:
        sleep(0.1)
    return conllizator(sentence)

@post('/rest/conllize2')
def CoNLLize2():
    response.content_type = 'text/html; charset=utf-8'
    while conllizator.inProgress:
        sleep(0.1)
    sentences = []
    for sentence in request.body:
        sentences.append(conllizator(sentence))
    return '\n\n'.join(sentences)


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
