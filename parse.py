#!/usr/bin/env python
# coding=utf-8

import subprocess
import codecs
from tokenize import CoNLLizator

# http://stackoverflow.com/questions/375427/non-blocking-read-on-a-subprocess-pipe-in-python/4896288#4896288
from threading import Thread
def stderr_watch(stderr, queue):
    # while True:
    #     line = stderr.readline().rstrip()
    #     print '>',line
    for line in iter(stderr.readline, b''):
        print '#', line.rstrip()
        # queue.put(line)
        # print '>', line.rstrip()

class Parser(object):

    def __init__(self, command="collins", args="--load --stdin --stdout -q --basedir=~/Work/collins++4/build", verbose=True):

        self.proc = None

        import os

        if command.startswith('~'):
            command = os.path.expanduser(command)

        if not os.path.isfile(command) and not os.path.isabs(command):
            newcmd = os.path.join(os.path.dirname(os.path.realpath(__file__)), command)
            if not os.path.isfile(command):
                print "Error: bad command:", command
                return
            command = newcmd

        # command = os.path.abspath(command)

        null = open(os.devnull, 'w')

        self.args = [command]+args.split()

        # To track both stderr and stdout:
        # http://stackoverflow.com/questions/4335587/wrap-subprocess-stdout-stderr/4335903#4335903

        # self.proc = subprocess.Popen([command]+args.split(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=null)
        self.proc = subprocess.Popen(self.args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # use utf-8 for input
        # self.proc.stdin = codecs.getwriter('utf-8')(self.proc.stdin)

        # self.proc.stderr.readline()
        status = self.proc.stderr.readline().rstrip()
        if verbose:
            print "Parser", status

        self.thread = Thread(target=stderr_watch, args=(self.proc.stderr, 0))
        self.thread.daemon = True
        self.thread.start()

        # import time
        # time.sleep(4)
        # print 'OK'
        

    def __call__(self, text):

        if not self.proc:
            print "Parser not started!"
            return ""

        text = text.strip()

        if not text:
            return ''

        # if type(text) == str:
        #     text = text.decode('utf-8')
        if type(text) == unicode:
            text = text.encode('utf-8')
        
        # print text
        print >> self.proc.stdin, text
        print >> self.proc.stdin

        lines = ''

        while True:
            line = self.proc.stdout.readline()
            if line == '\n':
                break;
            lines += line

        return lines



if __name__ == "__main__":

    conllizator = CoNLLizator("~/Work/LVTagger/morphotagger.sh")
    # conllizator('Jānītis iet.')
    # conllizator('Jānītis lido.')
    # conllizator(u'Jānītis ceļo.')
    parser = Parser(command="build/collins", args="--load --stdin --stdout -q --basedir=~/Work/collins++4/build")
    l = parser(conllizator("Saule spīd, jo laiks ir jauks!"))
    print l
    l = parser(conllizator("Lai notiek tā, kā jānotiek."))
    print l
    # l = parser('hei')
    # print repr(l)
    # print '-----------'
    # l = parser('hei')
    # print repr(l)
    # import time
    # time.sleep(2)

