#!/usr/bin/env python
# coding=utf-8

import subprocess
import codecs

class CorefResolver(object):

    def __init__(self, command="lvcoref.sh", args="", verbose=True):

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

        self.proc = None
        self.inProgress = False

        # command = os.path.abspath(command)

        # null = open(os.devnull, 'w')

        # To track both stderr and stdout:
        # http://stackoverflow.com/questions/4335587/wrap-subprocess-stdout-stderr/4335903#4335903

        # self.proc = subprocess.Popen([command]+args.split(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=null)
        # print [command]+args.split()
        try:
            self.proc = subprocess.Popen([command]+args.split(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except:
            if verbose:
                print "Coref NOT ready"
            return

        # use utf-8 for input
        # self.proc.stdin = codecs.getwriter('utf-8')(self.proc.stdin)

        # print self.proc.stderr.readline()
        # print self.proc.stderr.readline()
        # print self.proc.stderr.readline()

        # self.proc.stderr.readline()
        # self.proc.stderr.readline()
        # status = self.proc.stderr.readline() # should start with done

        if verbose:
            print "Coref ready"

        # if verbose:
        #     if status.startswith("done"):
        #         print "CoNLLizator ready"
        #     else:
        #         print "unknown status:", status


        # import time
        # time.sleep(1)
        # print 'OK'
        

    def __call__(self, text):

        if not self.proc:
            print "CorefResolver not started!"
            return ""

        text = text.strip()
        if not text:
            return ''

        self.inProgress = True

        # if type(text) == str:
        #     text = text.decode('utf-8')
        if type(text) == unicode:
            text = text.encode('utf-8')

        # print text
        # print '--------'
        print >> self.proc.stdin, text
        print >> self.proc.stdin
        print >> self.proc.stdin

        lines = ''
        prev = ''

        while True:
            line = self.proc.stdout.readline()
            if line == '\n' and prev == '\n':
                break
            # print repr(line)
            # if line == '\n':
            #     lines += line
            #     line = self.proc.stdout.readline()
            #     if line == '\n':
            #         break;
            lines += line
            prev = line

        self.inProgress = False

        # print lines
        return lines



if __name__ == "__main__":

    coref = CorefResolver("~/Work/LVCoref/lvcoref.sh")
    import os
    print coref(open(os.path.expanduser('./coref_test.conll')).read());
    print coref(open(os.path.expanduser('./coref_test.conll')).read());


