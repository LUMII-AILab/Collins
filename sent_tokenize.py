#!/usr/bin/env python
# coding=utf-8

from nltk.tokenize import sent_tokenize

class SentenceTokenizer(object):

    def __call__(self, text):

        text = text.strip()
        # if not text:
        #     return ''

        if text:
            for sentence in sent_tokenize(text):
                yield sentence.strip()


if __name__ == "__main__":

    conllizator = SentenceTokenizer()
    for s in conllizator(u'Pēteris ar Miķeli nevar beku kustināt. Nevar, nevar, nevar, nevar, nevar beku kustināt.'):
        print s

